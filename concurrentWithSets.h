#include <iostream>
#include <vector>
#include <optional>
#include <functional>
#include <random>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <algorithm>
#include <unordered_set>

template <typename T>
class SetsConcurrent {
public:
    SetsConcurrent();
    SetsConcurrent(int _size);
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
    void newHashes();
    int generateRandomInt(int min, int max);
    bool relocate(int i, int hi);
    void acquire(T x);
    void release(T x);

    // static int PROBE_SIZE = 4; //CHECK THIS
    static constexpr int PROBE_SIZE = 10;
    std::atomic<int> capacity;// = 2; //this too
    std::atomic<int> threshold;

    std::vector<std::shared_ptr<std::unordered_set<T>>> table1;
    std::vector<std::shared_ptr<std::unordered_set<T>>> table2;

    int maxSize = 10;
    int limit = 40;
    int dataAmt = 0;
    std::vector<std::shared_ptr<std::mutex>> locks1;
    std::vector<std::shared_ptr<std::mutex>> locks2;
};

template <typename T>
SetsConcurrent<T>::SetsConcurrent(int _size) {
    maxSize = _size;
    capacity = 4;
    threshold = 2;
    locks1.resize(_size);
    locks2.resize(_size);
    table1.resize(_size);
    table2.resize(_size);
    t1Hash = [this](T value) { return hash1(value); };
    t2Hash = [this](T value) { return hash2(value); };

    for(int i = 0; i < _size; i++){
        locks1[i] = std::make_shared<std::mutex>();
        locks2[i] = std::make_shared<std::mutex>();
        table1[i] = std::make_shared<std::unordered_set<T>>();
        table1[i]->reserve(capacity);
        table2[i] = std::make_shared<std::unordered_set<T>>();
        table2[i]->reserve(capacity);
    }
}
template <typename T>
SetsConcurrent<T>::SetsConcurrent() {
    capacity = 4;
    threshold = 2;
    table1.resize(maxSize);
    table2.resize(maxSize);
    locks1.resize(maxSize);
    locks2.resize(maxSize);
    t1Hash = [this](T value) { return hash1(value); };
    t2Hash = [this](T value) { return hash2(value); };

    for(int i = 0; i < maxSize; i++){
        locks1[i] = std::make_shared<std::mutex>();
        locks2[i] = std::make_shared<std::mutex>();
        table1[i] = std::make_shared<std::unordered_set<T>>();
        table2[i] = std::make_shared<std::unordered_set<T>>();
        table1[i]->reserve(capacity);
        table2[i]->reserve(capacity);
    }
}
template <typename T>
bool SetsConcurrent<T>::add(T value) {
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

    if (set0.size() < threshold) {
        set0.insert(value);
        release(value);
        return true;
    } else if (set1.size() < threshold) {
        set1.insert(value);
        release(value);
        return true;
    } else if (set0.size() < PROBE_SIZE) {
        set0.insert(value);
        i = 0;
        h = h0;
    } else if (set1.size() < PROBE_SIZE) {
        set1.insert(value);
        i = 1;
        h = h1;
    } else {
        if(set0.size() < set1.size()){
            set0.insert(value);
        }
        else{
            set1.insert(value);
        }

        mustResize = true;
    }

    if (mustResize) {
        resize();
        release(value);
        return add(value);
    } else if (!relocate(i, h)) {
        resize();
    }
    release(value);
    return true;
}

template <typename T>
bool SetsConcurrent<T>::remove(T value) {
    acquire(value);
    int h0 = t1Hash(value) % maxSize;
    int h1 = t2Hash(value) % maxSize;
    auto& set0 = *table1[h0];
    if (set0.erase(value)) {
        release(value);
        return true;
    }
    auto& set1 = *table2[h1];

    if (set1.erase(value)) {
        release(value);
        return true;
    }

    release(value);
    return false; // Not found
}


template <typename T>
bool SetsConcurrent<T>::relocate(int i, int hi) {
    int hj = 0;
    int j = 1 - i;
    int LIMIT = 10;

    for (int round = 0; round < LIMIT; round++) {
        auto& iSet = *table1[hi]; // Select table based on `i`
        if (iSet.empty()) return false; // Prevent accessing an empty bucket
        T value = *iSet.begin();
        acquire(value);
        switch (i) {
            case 0: hj = t2Hash(value) % maxSize; break;
            case 1: hj = t1Hash(value) % maxSize; break;
        }

        auto& jSet = *table2[hj];

        if (iSet.erase(value)) {
            if (jSet.size() < threshold) {
                jSet.insert(value);
                release(value);
                return true;
            } else if (jSet.size() < PROBE_SIZE) {
                jSet.insert(value);
                i = 1 - i;
                hi = hj;
                j = 1 - j;
            } else {
                iSet.insert(value); // Put value back if no space
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
std::optional<T> SetsConcurrent<T>::swap(int table, int loc, std::optional<T> value){
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
bool SetsConcurrent<T>::contains(T value, bool fromAdd) {
    if(!fromAdd){
        acquire(value);
    }
    auto& set1 = table1[t1Hash(value)];
    


    if (set1->find(value) != set1->end()) {
        if(!fromAdd){
            release(value);
        }
        return true;
    }
    
    auto& set2 = table2[t2Hash(value)];
    if (set2->find(value) != set2->end()) {
        if(!fromAdd){
            release(value);
        }
        return true;
    }
    
    if(!fromAdd){
        release(value);
    }
    return false;
}

template <typename T>
void SetsConcurrent<T>::display() {
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
int SetsConcurrent<T>::hash1(T value) const {
    return std::hash<T>{}(value) % maxSize; 
}

template <typename T>
int SetsConcurrent<T>::hash2(T value) const {
    unsigned int hash = std::hash<T>{}(value);
    hash ^= (hash >> 13) ^ (hash << 17);
    return static_cast<int>(hash % maxSize);
}

template <typename T>
int SetsConcurrent<T>::hash3(T value) const {
    unsigned int hash = std::hash<T>{}(value);
    hash = (~hash) + (hash << 15);
    return static_cast<int>(hash % maxSize);
}
template <typename T>
int SetsConcurrent<T>::hash4(T value) const {
    unsigned int hash = std::hash<T>{}(value);
    hash ^= (hash >> 11);
    hash += (hash << 3);
    return static_cast<int>(hash % maxSize);
}

template <typename T>
int SetsConcurrent<T>::size() {
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
void SetsConcurrent<T>::resize(int newSize) {
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
int SetsConcurrent<T>::generateRandomInt(int min, int max) {
    thread_local static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    thread_local static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}
template <typename T>
void SetsConcurrent<T>::newHashes() {
    int hash1 = SetsConcurrent<T>::generateRandomInt(1,4);
    int hash2 = hash1;
    while(hash2 == hash1){
        hash2 = SetsConcurrent<T>::generateRandomInt(1,4);
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
void SetsConcurrent<T>::populate(int amt, std::function<T()> generator) {
    for (int i = 0; i < amt; i++) {
        while(!add(generator()));
    }
}

template <typename T>
void SetsConcurrent<T>::resize() {
    int oldCapacity = capacity;

    //dont have to lock2 since locks1 always obtained first
    for (auto& l : locks1) {
        l->lock();
    }

    //check if already resized
    if (capacity != oldCapacity) {
        std::cout << "here?" << std::endl;
        return;
    }

    //Save old tables
    auto oldTable1 = table1;
    auto oldTable2 = table2;

    // Double the capacity
    capacity += capacity;

    // Create new tables with larger capacity
    table1 = std::vector<std::shared_ptr<std::unordered_set<T>>>(capacity);
    table2 = std::vector<std::shared_ptr<std::unordered_set<T>>>(capacity);

    for (int i = 0; i < capacity; i++) {
        table1[i] = std::make_shared<std::unordered_set<T>>();
        table2[i] = std::make_shared<std::unordered_set<T>>();
    }

    std::cout << "Here tjo" << std::endl;

    for (const auto& bucket : oldTable1) {
        for (const auto& value : *bucket) {
            add(value);
        }
    }

    for (const auto& bucket : oldTable2) {
        for (const auto& value : *bucket) {
            add(value);
        }
    }

    std::cout << "Here tjo2" << std::endl;

    for (auto& l : locks1) {
        l->unlock();
    }
}
template <typename T>
void SetsConcurrent<T>::acquire(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();


    locks1[h0]->lock();
    locks2[h1]->lock();

}
template <typename T>
void SetsConcurrent<T>::release(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();

    locks1[h0]->unlock();
    locks2[h1]->unlock();
}