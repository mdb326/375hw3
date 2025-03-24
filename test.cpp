#include "sequential.h"
#define SIZE 8192
#define TESTAMT 2000000

int generateRandomVal();
int generateRandomInteger(int min, int max);

int main() {
    SequentialCuckoo<int> cuckoo(SIZE);
    cuckoo.populate(SIZE/2, generateRandomVal);
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
    std::cout << cuckoo.size() << std::endl;

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