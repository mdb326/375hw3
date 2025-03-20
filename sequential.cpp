#include "sequential.h"

SequentialCuckoo::SequentialCuckoo(int size) {
    table1.resize(size);
    table2.resize(size);
}
SequentialCuckoo::SequentialCuckoo() {
    table1.resize(10);
    table2.resize(10);
}

void SequentialCuckoo::add(int value) {
    table1.push_back(value);
}
void SequentialCuckoo::display() {
    for (int val : table1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}