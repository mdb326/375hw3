#include "sequential.h"
#define SIZE 1024

int generateRandomVal();

int main() {
    SequentialCuckoo<int> cuckoo(SIZE);
    cuckoo.populate(SIZE/2, generateRandomVal);
    std::cout << cuckoo.size() << std::endl;
    return 0;
}
int generateRandomVal() {
    static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(0, SIZE); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}