#include "sequential.h"
#include <chrono>

#define SIZE 16777216
#define TESTAMT 20000000

int generateRandomVal();
int generateRandomInteger(int min, int max);

int main() {
    SequentialCuckoo<int> cuckoo(SIZE);
    cuckoo.populate(SIZE/2, generateRandomVal);
    auto begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < TESTAMT; i++){
        int num = generateRandomInteger(1, 10);
        if(num <= 8){
            cuckoo.contains(cuckoo.contains(generateRandomVal()));
        }
        else if (num == 9){
            cuckoo.add(generateRandomVal());
        }
        else {
            cuckoo.remove(generateRandomVal());
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << cuckoo.size() << std::endl;
    std::cout << "TOTAL EXECUTION TIME = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"\n";

    return 0;
}
int generateRandomVal() {
    static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(1, SIZE); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}
int generateRandomInteger(int min, int max) {
    static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}