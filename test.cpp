#include "sequential.h"

int main() {
    SequentialCuckoo<int> ds;
    ds.add(10);
    // ds.add(20);
    // ds.add(10);
    ds.display();
    // ds.remove(10);
    // ds.display();
    return 0;
}
