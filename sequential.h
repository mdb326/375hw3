#include <iostream>
#include <vector>

class SequentialCuckoo {
public:
    SequentialCuckoo();
    SequentialCuckoo(int size);
    void add(int value);
    // void remove(int value);
    void display();

private:
    std::vector<int> table1; 
    std::vector<int> table2;
};

