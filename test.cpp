#include "sequential.h"

int main() {
    SequentialCuckoo<int> ds;
    ds.add(10);
    ds.add(21);
    ds.add(30);
    std::cout << ds.size() << std::endl;
    ds.display();
    ds.remove(10);
    ds.remove(30);
    ds.display();
    return 0;
}
