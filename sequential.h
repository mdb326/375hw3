#include <iostream>
#include <vector>
#include <optional>
#include <functional>
#include <random>

template <typename T>
class SequentialCuckoo {
public:
    SequentialCuckoo();
    SequentialCuckoo(int _size);
    bool add(T value);
    int size();
    bool contains(T value);
    std::optional<T> swap(int table, int loc, std::optional<T> value);
    bool remove(T value);
    void display();
    void populate(int amt, std::function<T()> generator);

private:
    std::function<int(std::optional<T>)> t1Hash;
    std::function<int(std::optional<T>)> t2Hash;

    int hash1(std::optional<T> value) const;
    int hash2(std::optional<T> value) const;
    int hash3(std::optional<T> value) const;
    int hash4(std::optional<T> value) const;
    void resize(int newSize);
    void newHashes();
    int generateRandomInt(int min, int max);

    std::vector<std::optional<T>> table1; 
    std::vector<std::optional<T>> table2;
    int maxSize = 10;
    int limit = 40;
    int dataAmt = 0;
};

template <typename T>
SequentialCuckoo<T>::SequentialCuckoo(int _size) {
    maxSize = _size;
    table1.resize(_size);
    table2.resize(_size);
    t1Hash = [this](std::optional<T> value) { return hash1(value); };
    t2Hash = [this](std::optional<T> value) { return hash2(value); };
}
template <typename T>
SequentialCuckoo<T>::SequentialCuckoo() {
    table1.resize(maxSize);
    table2.resize(maxSize);
    t1Hash = [this](std::optional<T> value) { return hash1(value); };
    t2Hash = [this](std::optional<T> value) { return hash2(value); };
}
template <typename T>
bool SequentialCuckoo<T>::add(T value) {
    dataAmt++;
    if(dataAmt > maxSize){
        resize(maxSize * 2);
    }
    if(contains(value)){
        return false;
    }
    std::optional<T> x = value;
    for(int i = 0; i < limit; i++){
        if((x = swap(1,t1Hash(x), x)) == NULL){
            return true;
        }
        else if ((x = swap(2,t2Hash(x), x)) == NULL){
            return true;
        }
    }
    //failed to add
    newHashes();
    resize(maxSize);
    add(x.value()); //guaranteed to have value since we just took it out
    return false;
}

template <typename T>
bool SequentialCuckoo<T>::remove(T value) {
    int loc = t1Hash(value);
    if(table1[loc].has_value() && table1[loc].value() == value){
        table1[loc] = std::nullopt;
        return true;
    }
    loc = t2Hash(value);
    if(table2[loc].has_value() && table2[loc].value() == value){
        table2[loc] = std::nullopt;
        return true;
    }
    return false;
}

template <typename T>
std::optional<T> SequentialCuckoo<T>::swap(int table, int loc, std::optional<T> value){
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
bool SequentialCuckoo<T>::contains(T value) {
    if(table1[t1Hash(value)] == value){
        return true;
    }
    if(table2[t2Hash(value)] == value){
        return true;
    }
    return false;
}
template <typename T>
void SequentialCuckoo<T>::display() {
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
int SequentialCuckoo<T>::hash1(std::optional<T> value) const {
    return static_cast<int>(value.value()) % maxSize; 
}

template <typename T>
int SequentialCuckoo<T>::hash2(std::optional<T> value) const {
    return (static_cast<int>(value.value()) * 3  + 3) % maxSize; 
}

template <typename T>
int SequentialCuckoo<T>::hash3(std::optional<T> value) const {
    return (static_cast<int>(value.value()) / 3 + 2) % maxSize; 
}
template <typename T>
int SequentialCuckoo<T>::hash4(std::optional<T> value) const {
    return (static_cast<int>(value.value()) * 8 / 5 + 1) % maxSize; 
}

template <typename T>
int SequentialCuckoo<T>::size() {
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
void SequentialCuckoo<T>::resize(int newSize) {
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
int SequentialCuckoo<T>::generateRandomInt(int min, int max) {
    static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}
template <typename T>
void SequentialCuckoo<T>::newHashes() {
    int hash1 = SequentialCuckoo<T>::generateRandomInt(1,4);
    int hash2 = hash1;
    while(hash2 = hash1){
        hash2 = SequentialCuckoo<T>::generateRandomInt(1,4);
    }
    t1Hash = [this](std::optional<T> value) { return this->hash1(value); };
    t2Hash = [this](std::optional<T> value) { return this->hash2(value); };
}
template <typename T>
void SequentialCuckoo<T>::populate(int amt, std::function<T()> generator) {
    for (int i = 0; i < amt; i++) {
        while(!add(generator()));
    }
}
