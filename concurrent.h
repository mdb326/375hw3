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

template <typename T>
class ConcurrentCuckoo {
public:
    ConcurrentCuckoo();
    ConcurrentCuckoo(int _size);
    bool add(T value);
    int size();
    bool contains(T value);
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

    std::vector<std::shared_ptr<std::vector<T>>> table1;
    std::vector<std::shared_ptr<std::vector<T>>> table2;

    int maxSize = 10;
    int limit = 40;
    int dataAmt = 0;
    std::vector<std::shared_ptr<std::mutex>> locks1;
    std::vector<std::shared_ptr<std::mutex>> locks2;
};

template <typename T>
ConcurrentCuckoo<T>::ConcurrentCuckoo(int _size) {
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
        table1[i] = std::make_shared<std::vector<T>>();
        table2[i] = std::make_shared<std::vector<T>>();
    }
}
template <typename T>
ConcurrentCuckoo<T>::ConcurrentCuckoo() {
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
        table1[i] = std::make_shared<std::vector<T>>();
        table2[i] = std::make_shared<std::vector<T>>();
    }
}
template <typename T>
bool ConcurrentCuckoo<T>::add(T value) {
    acquire(value);
    int h0 = t1Hash(value) % capacity, h1 = t2Hash(value) % capacity;
    int i = -1, h = -1;
    bool mustResize = false;

    if (contains(value)){
        release(value);
        return false; // Check if value is already in the set
    } 

    auto& set0 = *table1[h0];
    auto& set1 = *table2[h1];

    if (set0.size() < threshold) {
        set0.push_back(value);
        release(value);
        return true;
    } else if (set1.size() < threshold) {
        set1.push_back(value);
        release(value);
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

        mustResize = true;
    }

    // if (mustResize) {
    //     resize();
    //     // release(value);
    //     return add(value);
    // } else if (!relocate(i, h)) {
    //     resize();
    // }
    release(value);
    return true;
}

template <typename T>
bool ConcurrentCuckoo<T>::remove(T value) {
    acquire(value);
    int h0 = t1Hash(value) % capacity;
    int h1 = t2Hash(value) % capacity;
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
bool ConcurrentCuckoo<T>::relocate(int i, int hi) {
    int hj = 0;
    int j = 1 - i;
    int LIMIT = 10;

    for (int round = 0; round < LIMIT; round++) {
        auto& iSet = *table1[hi]; // Select table based on `i`
        if (iSet.empty()) return false; // Prevent accessing an empty bucket
        T value = iSet.front();
        acquire(value);
        switch (i) {
            case 0: hj = t2Hash(value) % capacity; break;
            case 1: hj = t1Hash(value) % capacity; break;
        }

        // acquire(value);
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
std::optional<T> ConcurrentCuckoo<T>::swap(int table, int loc, std::optional<T> value){
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
bool ConcurrentCuckoo<T>::contains(T value) {
    // acquire(value);
    auto& bucket1_ptr = table1[t1Hash(value)];
    auto& bucket2_ptr = table2[t2Hash(value)];


    for (const auto& val : *bucket1_ptr) {
        if (val == value) {
            // release(value);
            return true;
        }
    }

    for (const auto& val : *bucket2_ptr) {
        if (val == value) {
            // release(value);
            return true;
        }
    }
    // release(value);
    return false;
}

template <typename T>
void ConcurrentCuckoo<T>::display() {
    std::cout << "Table 1: ";
    for (const std::optional<T>& val : table1) {
        if (val.has_value()) {
            std::cout << val.value() << " ";
        } else {
            std::cout << "[empty] ";
        }
    }
    std::cout << "\nTable 2: ";
    for (const std::optional<T>& val : table2) {
        if (val.has_value()) {
            std::cout << val.value() << " ";
        } else {
            std::cout << "[empty] ";
        }
    }
    std::cout << std::endl;
}

template <typename T>
int ConcurrentCuckoo<T>::hash1(T value) const {
    return static_cast<int>(value) % maxSize; 
}

template <typename T>
int ConcurrentCuckoo<T>::hash2(T value) const {
    return (static_cast<int>(value) * 3  + 3) % maxSize; 
}

template <typename T>
int ConcurrentCuckoo<T>::hash3(T value) const {
    return (static_cast<int>(value) / 3 + 2) % maxSize; 
}
template <typename T>
int ConcurrentCuckoo<T>::hash4(T value) const {
    return (static_cast<int>(value) * 8 / 5 + 1) % maxSize; 
}

template <typename T>
int ConcurrentCuckoo<T>::size() {
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
void ConcurrentCuckoo<T>::resize(int newSize) {
    maxSize = newSize;
    std::vector<T> values;
    for (const std::optional<T>& val : table1) {
        if (val.has_value()) {
            values.push_back(val.value());
        }
    }
    for (const std::optional<T>& val : table2) {
        if (val.has_value()) {
            values.push_back(val.value());
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
int ConcurrentCuckoo<T>::generateRandomInt(int min, int max) {
    thread_local static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    thread_local static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}
template <typename T>
void ConcurrentCuckoo<T>::newHashes() {
    int hash1 = ConcurrentCuckoo<T>::generateRandomInt(1,4);
    int hash2 = hash1;
    while(hash2 = hash1){
        hash2 = ConcurrentCuckoo<T>::generateRandomInt(1,4);
    }
    t1Hash = [this](std::optional<T> value) { return this->hash1(value); };
    t2Hash = [this](std::optional<T> value) { return this->hash2(value); };
}
template <typename T>
void ConcurrentCuckoo<T>::populate(int amt, std::function<T()> generator) {
    for (int i = 0; i < amt; i++) {
        while(!add(generator()));
    }
}

template <typename T>
void ConcurrentCuckoo<T>::resize() {
    int oldCapacity = capacity;

    // for (auto& l : locks1) {
    //     l->lock();
    // }

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
    table1 = std::vector<std::shared_ptr<std::vector<T>>>(capacity);
    table2 = std::vector<std::shared_ptr<std::vector<T>>>(capacity);

    for (int i = 0; i < capacity; i++) {
        table1[i] = std::make_shared<std::vector<T>>();
        table2[i] = std::make_shared<std::vector<T>>();
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

    // for (auto& l : locks1) {
    //     l->unlock();
    // }
}
template <typename T>
void ConcurrentCuckoo<T>::acquire(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();

    if (h0 < h1) {
        locks1[h0]->lock();
        locks2[h1]->lock();
    } else {
        locks2[h1]->lock();
        locks1[h0]->lock();
    }
}
template <typename T>
void ConcurrentCuckoo<T>::release(T x) {
    int h0 = t1Hash(x) % locks1.size();
    int h1 = t2Hash(x) % locks2.size();

    locks1[h0]->unlock();
    locks2[h1]->unlock();
}