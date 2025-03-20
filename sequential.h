#include <iostream>
#include <vector>

template <typename T>
class SequentialCuckoo {
public:
    SequentialCuckoo();
    SequentialCuckoo(int _size);
    bool add(T value);
    bool contains(T value);
    // void remove(int value);
    void display();

private:
    std::vector<T> table1; 
    std::vector<T> table2;
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
    for(int i = 0; i < limit; i++){
        /*
        if((x = swap(0,hash0(x), x) == null){
            return true;
        }
        else if ((x = swap(1,hash1(x), x) == null)){
            return true;
        }
        */
    }
    //resize()
    //add(x)
    // table1.push_back(value);
    return false;
}
template <typename T>
bool SequentialCuckoo<T>::contains(T value) {
    for (const auto& item : table1) {
        if (item == value) return true;
    }
    for (const auto& item : table2) {
        if (item == value) return true;
    }
    return false;
}
template <typename T>
void SequentialCuckoo<T>::display() {
    for (int val : table1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

