#include "sequential.h"

DataStructure::DataStructure() {}

void DataStructure::insert(int value) {
    data.push_back(value);
}

void DataStructure::remove(int value) {
    std::cout << value;
}

void DataStructure::display() const {
    for (int val : data) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}
