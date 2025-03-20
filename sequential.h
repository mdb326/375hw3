#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include <iostream>
#include <vector>

class DataStructure {
public:
    DataStructure();
    void insert(int value);
    void remove(int value);
    void display() const;

private:
    std::vector<int> data;
};

#endif // SEQUENTIAL_H
