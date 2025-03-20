#include <iostream>
#include <vector>
#include <optional>

template <typename T>
class SequentialCuckoo {
public:
    SequentialCuckoo();
    SequentialCuckoo(int _size);
    bool add(T value);
    bool contains(T value);
    std::optional<T> swap(int table, int loc, std::optional<T> value);
    // void remove(int value);
    void display();

private:
    int hash1(std::optional<T> value) const;
    int hash2(std::optional<T> value) const;
    std::vector<std::optional<T>> table1; 
    std::vector<std::optional<T>> table2;
    int size = 10;
    int limit = 20;
};

template <typename T>
SequentialCuckoo<T>::SequentialCuckoo(int _size) {
    size = _size;
    table1.resize(_size);
    table2.resize(_size);
}
template <typename T>
SequentialCuckoo<T>::SequentialCuckoo() {
    table1.resize(size);
    table2.resize(size);
}
template <typename T>
bool SequentialCuckoo<T>::add(T value) {
    if(contains(value)){
        return false;
    }
    std::optional<T> x = value;
    for(int i = 0; i < limit; i++){
        if(x = swap(0,hash1(x), x) == NULL){
            return true;
        }
        else if (x = swap(1,hash2(x), x) == NULL){
            return true;
        }
    }
    //resize()
    //add(x)
    // table1.push_back(value);
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
    else {
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
    if(table1[hash1(value)] == value){
        return true;
    }
    if(table2[hash2(value)] == value){
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
    return static_cast<int>(value.value()) % size; 
}

template <typename T>
int SequentialCuckoo<T>::hash2(std::optional<T> value) const {
    return static_cast<int>(value.value()) % size + 3; 
}