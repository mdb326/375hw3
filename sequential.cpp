#include "sequential.h"
#include <algorithm>

SequentialCuckoo::SequentialCuckoo(int _size) {
    table1.resize(_size);
    table2.resize(_size);
}
SequentialCuckoo::SequentialCuckoo() {
    table1.resize(size);
    table2.resize(size);
}

bool SequentialCuckoo::add(int value) {
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
bool SequentialCuckoo::contains(int value) {
    if(std::count(table1.begin(), table1.end(), value) || std::count(table2.begin(), table2.end(), value)){
        return true;
    }
    return false;
}
void SequentialCuckoo::display() {
    for (int val : table1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}