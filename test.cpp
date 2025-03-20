#include "sequential.h"

int main() {
    SequentialCuckoo ds;
    ds.add(10);
    ds.add(20);
    ds.display();
    // ds.remove(10);
    // ds.display();
    return 0;
}
