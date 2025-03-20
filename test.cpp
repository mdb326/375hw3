#include "sequential.h"

int main() {
    DataStructure ds;
    ds.insert(10);
    ds.insert(20);
    ds.display();
    ds.remove(10);
    ds.display();
    return 0;
}
