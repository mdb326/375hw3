#include <iostream>
#include <vector>
#include <optional>
#include <functional>
#include <random>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <atomic>
#include <algorithm>

template <typename T>
class LogicalCuckoo {
public:
    LogicalCuckoo();
    LogicalCuckoo(int _size);
    bool add(T value);
    int size();
    bool contains(T value, bool fromAdd);
    std::optional<T> swap(int table, int loc, std::optional<T> value);
    bool remove(T value);
    void display();
    void populate(int amt, std::function<T()> generator);

private:
    std::function<int(T)> t1Hash;
    std::function<int(T)> t2Hash;

    int hash1(T) const;
    int hash2(T) const;
    int hash3(T) const;
    int hash4(T) const;
    void resize(int newSize);
    void resize();
    bool add_safe(T value);
    void newHashes();
    int generateRandomInt(int min, int max);
    bool relocate(int i, int hi);
    bool relocate_safe(int i, int hi);
    void acquire(T x);
    void acquireShared(T x);
    void release(T x);
    void releaseShared(T x);

    static constexpr int PROBE_SIZE = 10;
    static constexpr int STRIPING = 512;
    std::atomic<int> capacity;// = 2; //this too
    std::atomic<int> threshold;

    std::vector<std::shared_ptr<std::vector<T>>> table1;
    std::vector<std::shared_ptr<std::vector<T>>> table2;
    std::vector<std::shared_ptr<std::vector<std::atomic<bool>*>>> deletes1; //copying rules
    std::vector<std::shared_ptr<std::vector<std::atomic<bool>*>>> deletes2;

    int maxSize = 10;
    int limit = 40;
    int dataAmt = 0;
    std::vector<std::shared_ptr<std::shared_mutex>> locks1;
    std::vector<std::shared_ptr<std::shared_mutex>> locks2;
    
};

template <typename T>
LogicalCuckoo<T>::LogicalCuckoo(int _size) {
    maxSize = _size;
    capacity = 4;
    threshold = 2;
    locks1.resize(_size/STRIPING);
    locks2.resize(_size/STRIPING);
    table1.resize(_size);
    table2.resize(_size);
    deletes1.resize(_size);
    deletes2.resize(_size);
    t1Hash = [this](T value) { return hash1(value); };
    t2Hash = [this](T value) { return hash2(value); };

    for(int i = 0; i < _size; i++){
        
        table1[i] = std::make_shared<std::vector<T>>();
        table1[i]->resize(capacity);
        table2[i] = std::make_shared<std::vector<T>>();
        table2[i]->resize(capacity);
        deletes1[i] = std::make_shared<std::vector<std::atomic<bool>*>>();
        deletes2[i] = std::make_shared<std::vector<std::atomic<bool>*>>();
        deletes1[i]->resize(capacity);
        deletes2[i]->resize(capacity);
        for(int j = 0; j < capacity; j++){
            (*deletes1[i])[j] = new std::atomic<bool>(false);
            (*deletes2[i])[j] = new std::atomic<bool>(false);
        }
    }
    for(int i = 0; i < _size/STRIPING; i++){
        locks1[i] = std::make_shared<std::shared_mutex>();
        locks2[i] = std::make_shared<std::shared_mutex>();
    }
}
template <typename T>
LogicalCuckoo<T>::LogicalCuckoo() {
    capacity = 4;
    threshold = 2;
    table1.resize(maxSize);
    table2.resize(maxSize);
    locks1.resize(maxSize/STRIPING);
    locks2.resize(maxSize/STRIPING);
    deletes1.resize(maxSize);
    deletes2.resize(maxSize);
    t1Hash = [this](T value) { return hash1(value); };
    t2Hash = [this](T value) { return hash2(value); };

    for(int i = 0; i < maxSize; i++){
        table1[i] = std::make_shared<std::vector<T>>();
        table2[i] = std::make_shared<std::vector<T>>();
        table1[i]->resize(capacity);
        table2[i]->resize(capacity);
        deletes1[i] = std::make_shared<std::vector<std::atomic<bool>*>>();
        deletes2[i] = std::make_shared<std::vector<std::atomic<bool>*>>();
        deletes1[i]->resize(capacity);
        deletes2[i]->resize(capacity);
        for(int j = 0; j < capacity; j++){
            (*deletes1[i])[j] = new std::atomic<bool>(false);
            (*deletes2[i])[j] = new std::atomic<bool>(false);
        }
    }
    for(int i = 0; i < maxSize/STRIPING; i++){
        locks1[i] = std::make_shared<std::shared_mutex>();
        locks2[i] = std::make_shared<std::shared_mutex>();
    }
}
template <typename T>
bool LogicalCuckoo<T>::add(T value) {
    acquire(value);
    int h0 = t1Hash(value) % maxSize, h1 = t2Hash(value) % maxSize;
    int i = -1, h = -1;
    bool mustResize = false;

    if (contains(value, true)){
        release(value);
        return false; // Check if value is already in the set
    } 

    auto& set0 = *table1[h0];
    auto& set1 = *table2[h1];

    size_t size0 = 0;
    size_t size1 = 0;
    for (size_t i = 0; i < set0.size(); ++i) {
        if (!(*deletes1[h0])[i]->load()) {  // Only count if not marked for deletion
            ++size0;
        }
    }
    for (size_t i = 0; i < set1.size(); ++i) {
        if (!(*deletes2[h1])[i]->load()) {  // Only count if not marked for deletion
            ++size1;
        }
    }

    if (size0 < threshold) {
        for (size_t i = 0; i < set0.size(); ++i) {
            if (!(*deletes1[h0])[i]->load()) { 
                set0[i] = value; 
                (*deletes1[h0])[i]->store(true);
                break; 
            }
        }
        release(value);
        return true;
    } else if (size1 < threshold) {
        for (size_t i = 0; i < set1.size(); ++i) {
            if (!(*deletes1[h0])[i]->load()) { 
                set1[i] = value; 
                (*deletes1[h0])[i]->store(true);
                break; 
            }
        }
        release(value);
        return true;
    } else if (size0 < PROBE_SIZE) {
        for (size_t i = 0; i < set0.size(); ++i) {
            if (!(*deletes1[h0])[i]->load()) { 
                set0[i] = value; 
                (*deletes1[h0])[i]->store(true);
                break; 
            }
        }
        i = 0;
        h = h0;
    } else if (size1 < PROBE_SIZE) {
        for (size_t i = 0; i < set1.size(); ++i) {
            if (!(*deletes1[h0])[i]->load()) { 
                set1[i] = value; 
                (*deletes1[h0])[i]->store(true);
                break; 
            }
        }
        i = 1;
        h = h1;
    } else {
        if(size0 < size1){
            set0.push_back(value);
        }
        else{
            set1.push_back(value);
        }

        mustResize = true;
    }

    if (mustResize) {
        release(value);
        resize();
        return add(value);
    } else if (!relocate(i, h)) { // look at this
        release(value);
        resize();
    }
    release(value);
    return true;
}

template <typename T>
bool LogicalCuckoo<T>::remove(T value) {
    acquire(value);
    int h0 = t1Hash(value) % maxSize;
    int h1 = t2Hash(value) % maxSize;
    auto& set0 = *table1[h0];
    auto it = std::remove(set0.begin(), set0.end(), value);
    if (it != set0.end()) {
        set0.erase(it, set0.end());
        release(value);
        return true;
    }
    auto& set1 = *table2[h1];
    auto it2 = std::remove(set1.begin(), set1.end(), value);
    if (it2 != set1.end()) {
        set1.erase(it2, set1.end());
        release(value);
        return true;
    }

    release(value);
    return false; // Not found
}


template <typename T>
bool LogicalCuckoo<T>::relocate(int i, int hi) {
    int hj = 0;
    int j = 1 - i;
    int LIMIT = 10;

    for (int round = 0; round < LIMIT; round++) {
        auto& iSet = *table1[hi];
        if (iSet.empty()) return false; // Prevent accessing an empty bucket
        T value = iSet.front();
        acquire(value);
        switch (i) {
            case 0: hj = t2Hash(value) % maxSize; break;
            case 1: hj = t1Hash(value) % maxSize; break;
        }

        auto& jSet = *table2[hj];

        auto it = std::find(iSet.begin(), iSet.end(), value);
        if (it != iSet.end()) {
            iSet.erase(it);
            if (jSet.size() < threshold) {
                jSet.push_back(value);
                release(value);
                return true;
            } else if (jSet.size() < PROBE_SIZE) {
                jSet.push_back(value);
                i = 1 - i;
                hi = hj;
                j = 1 - j;
            } else {
                iSet.push_back(value); // Put value back if no space
                release(value);
                return false;
            }
        } else if (iSet.size() >= threshold) {
            continue; // Retry relocation
        } else {
            release(value);
            return true; // Relocation succeeded
        }
        release(value);
    }
    
    return false; // If all rounds fail, relocation is unsuccessful
}

template <typename T>
bool LogicalCuckoo<T>::relocate_safe(int i, int hi) {
    int hj = 0;
    int j = 1 - i;
    int LIMIT = 10;

    for (int round = 0; round < LIMIT; round++) {
        auto& iSet = *table1[hi];
        if (iSet.empty()) return false; // Prevent accessing an empty bucket
        T value = iSet.front();
        switch (i) {
            case 0: hj = t2Hash(value) % maxSize; break;
            case 1: hj = t1Hash(value) % maxSize; break;
        }

        auto& jSet = *table2[hj];

        auto it = std::find(iSet.begin(), iSet.end(), value);
        if (it != iSet.end()) {
            iSet.erase(it);
            if (jSet.size() < threshold) {
                jSet.push_back(value);
                return true;
            } else if (jSet.size() < PROBE_SIZE) {
                jSet.push_back(value);
                i = 1 - i;
                hi = hj;
                j = 1 - j;
            } else {
                iSet.push_back(value); // Put value back if no space
                return false;
            }
        } else if (iSet.size() >= threshold) {
            continue; // Retry relocation
        } else {
            return true; // Relocation succeeded
        }
    }
    
    return false; // If all rounds fail, relocation is unsuccessful
}

template <typename T>
std::optional<T> LogicalCuckoo<T>::swap(int table, int loc, std::optional<T> value){
    if(table == 1){ //go into table1 
        if(!table1[loc].has_value()){
            table1[loc] = value;
            return NULL;
        }
        else { 
            std::optional<T> temp = table1[loc];
            table1[loc] = value;
            return temp;
        }
    }
    else { //table2
        if(!table2[loc].has_value()){
            table2[loc] = value;
            return NULL;
        }
        else {
            std::optional<T> temp = table2[loc];
            table2[loc] = value;
            return temp;
        }
    }
}

template <typename T>
bool LogicalCuckoo<T>::contains(T value, bool fromAdd) {
    if(!fromAdd){
        acquireShared(value);
    }
    size_t h0 = t1Hash(value);
    size_t h1 = t2Hash(value);
    auto& bucket1_ptr = table1[h0];
    auto& bucket2_ptr = table2[h1];


    for (size_t i = 0; i < bucket1_ptr->size(); ++i) {
        if ((*bucket1_ptr)[i] == value && !(*deletes1[h0])[i]->load()) {
            if (!fromAdd) {
                releaseShared(value);
            }
            return true;
        }
    }
    for (size_t i = 0; i < bucket2_ptr->size(); ++i) {
        if ((*bucket2_ptr)[i] == value && !(*deletes2[h1])[i]->load()) {
            if (!fromAdd) {
                releaseShared(value);
            }
            return true;
        }
    }
    if(!fromAdd){
        releaseShared(value);
    }
    return false;
}

template <typename T>
void LogicalCuckoo<T>::display() {
    std::cout << "Table 1: ";
    for (const std::optional<T>& val : table1) {
        if (val.has_value()) {
            std::cout << val << " ";
        } else {
            std::cout << "[empty] ";
        }
    }
    std::cout << "\nTable 2: ";
    for (const std::optional<T>& val : table2) {
        if (val.has_value()) {
            std::cout << val << " ";
        } else {
            std::cout << "[empty] ";
        }
    }
    std::cout << std::endl;
}

template <typename T>
int LogicalCuckoo<T>::hash1(T value) const {
    return std::hash<T>{}(value) % maxSize; 
}

template <typename T>
int LogicalCuckoo<T>::hash2(T value) const {
    unsigned int hash = std::hash<T>{}(value);
    hash ^= (hash >> 13) ^ (hash << 17);
    return static_cast<int>(hash % maxSize);
}

template <typename T>
int LogicalCuckoo<T>::hash3(T value) const {
    unsigned int hash = std::hash<T>{}(value);
    hash = (~hash) + (hash << 15);
    return static_cast<int>(hash % maxSize);
}
template <typename T>
int LogicalCuckoo<T>::hash4(T value) const {
    unsigned int hash = std::hash<T>{}(value);
    hash ^= (hash >> 11);
    hash += (hash << 3);
    return static_cast<int>(hash % maxSize);
}

template <typename T>
int LogicalCuckoo<T>::size() {
    int cnt = 0;
    for (const std::optional<T>& val : table1) {
        if (val.has_value()) {
            cnt++;
        }
    }
    for (const std::optional<T>& val : table2) {
        if (val.has_value()) {
            cnt++;
        }
    }
    return cnt;
}

template <typename T>
void LogicalCuckoo<T>::resize(int newSize) {
    maxSize = newSize;
    std::vector<T> values;
    for (const std::optional<T>& val : table1) {
        if (val.has_value()) {
            values.push_back(val);
        }
    }
    for (const std::optional<T>& val : table2) {
        if (val.has_value()) {
            values.push_back(val);
        }
    }
    table1.clear();
    table2.clear();
    table1.resize(newSize);
    table2.resize(newSize);
    for(T val : values){
        add(val);
    }
}
// Generates a random int between min and max (inclusive)
template <typename T>
int LogicalCuckoo<T>::generateRandomInt(int min, int max) {
    thread_local static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    thread_local static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}
template <typename T>
void LogicalCuckoo<T>::newHashes() {
    int hash1 = LogicalCuckoo<T>::generateRandomInt(1,4);
    int hash2 = hash1;
    while(hash2 == hash1){
        hash2 = LogicalCuckoo<T>::generateRandomInt(1,4);
    }
    switch (hash1) {
        case 1:
            t1Hash = [this](std::optional<T> value) { return this->hash1(value); };
            break;
        case 2:
            t1Hash = [this](std::optional<T> value) { return this->hash2(value); };
            break;
        case 3:
            t1Hash = [this](std::optional<T> value) { return this->hash3(value); };
            break;
        case 4:
            t1Hash = [this](std::optional<T> value) { return this->hash4(value); };
            break;
    }
    
    // Assign t2Hash using a switch on hash2
    switch (hash2) {
        case 1:
            t2Hash = [this](std::optional<T> value) { return this->hash1(value); };
            break;
        case 2:
            t2Hash = [this](std::optional<T> value) { return this->hash2(value); };
            break;
        case 3:
            t2Hash = [this](std::optional<T> value) { return this->hash3(value); };
            break;
        case 4:
            t2Hash = [this](std::optional<T> value) { return this->hash4(value); };
            break;
    }
}
template <typename T>
void LogicalCuckoo<T>::populate(int amt, std::function<T()> generator) {
    for (int i = 0; i < amt; i++) {
        T val = generator();
        std::cout << val << std::endl;
        while(!add(val));
    }
}

template <typename T>
void LogicalCuckoo<T>::resize() {
    for (auto& l : locks1) {
        l->lock();
    }

    bool resizing = true;
    while(resizing){
        resizing = false;
        //Save old tables
        auto oldTable1 = table1;
        auto oldTable2 = table2;
        // Double the capacity
        maxSize += maxSize;

        // Create new tables with larger capacity
        table1 = std::vector<std::shared_ptr<std::vector<T>>>(maxSize);
        table2 = std::vector<std::shared_ptr<std::vector<T>>>(maxSize);

        for (int i = 0; i < maxSize; i++) {
            table1[i] = std::make_shared<std::vector<T>>();
            table2[i] = std::make_shared<std::vector<T>>();
        }

        std::cout << "Here tjo: " << maxSize << std::endl;

        for (const auto& bucket : oldTable1) {
            if (!bucket) continue; //New buckets if resizing twice
            for (const auto& value : *bucket) {
                if(!add_safe(value)){
                    std::cout << value << std::endl;
                    resizing = true;
                    break;
                }
            }
        }

        for (const auto& bucket : oldTable2) {
            if (!bucket) continue;
            for (const auto& value : *bucket) {
                if(!add_safe(value)){
                    std::cout << value << std::endl;
                    resizing = true;
                    break;
                }
            }
        }
    }

    

    std::cout << "Here tjo2" << std::endl;

    for (auto& l : locks1) {
        l->unlock();
    }
}

template <typename T>
bool LogicalCuckoo<T>::add_safe(T value) {
    int h0 = t1Hash(value) % maxSize, h1 = t2Hash(value) % maxSize;
    int i = -1, h = -1;
    bool mustResize = false;

    auto& set0 = *table1[h0];
    auto& set1 = *table2[h1];

    if (set0.size() < threshold) {
        set0.push_back(value);
        return true;
    } else if (set1.size() < threshold) {
        set1.push_back(value);
        return true;
    } else if (set0.size() < PROBE_SIZE) {
        set0.push_back(value);
        i = 0;
        h = h0;
    } else if (set1.size() < PROBE_SIZE) {
        set1.push_back(value);
        i = 1;
        h = h1;
    } else {
        if(set0.size() < set1.size()){
            set0.push_back(value);
        }
        else{
            set1.push_back(value);
        }

        return false;
    }

    if (!relocate_safe(i, h)) {
        return false;
    }
    return true;
}

template <typename T>
void LogicalCuckoo<T>::acquire(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();


    locks1[h0]->lock();
    locks2[h1]->lock();
}
template <typename T>
void LogicalCuckoo<T>::acquireShared(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();

    locks1[h0]->lock_shared();
    locks2[h1]->lock_shared();
}
template <typename T>
void LogicalCuckoo<T>::release(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();

    locks1[h0]->unlock();
    locks2[h1]->unlock();
}
template <typename T>
void LogicalCuckoo<T>::releaseShared(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();

    locks1[h0]->unlock_shared();
    locks2[h1]->unlock_shared();
}