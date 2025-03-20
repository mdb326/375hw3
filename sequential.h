#include <iostream>
#include <vector>

class SequentialCuckoo {
public:
    SequentialCuckoo();
    SequentialCuckoo(int size);
    bool add(int value);
    bool contains(int value);
    // void remove(int value);
    void display();

private:
    std::vector<int> table1; 
    std::vector<int> table2;
    int size = 10;
    int limit = 20;
};

