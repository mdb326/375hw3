#include "sequential.h"
#include "concurrent.h"
#include <chrono>
#include <iostream>
#include <random>
#include <cstdlib>

#define TESTAMT 200

int generateRandomVal(int size);
int generateRandomInteger(int min, int max);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <size>\n";
        return 1;
    }

    int size = std::atoi(argv[1]);
    if (size <= 0) {
        std::cerr << "Error: size must be a positive integer.\n";
        return 1;
    }

    // SequentialCuckoo<int> cuckoo(size);
    ConcurrentCuckoo<int> cuckoo(size);
    cuckoo.populate(size/2, [size]() { return generateRandomVal(size); });
    std::cout << "Made it here" << std::endl;

    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TESTAMT; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= 8) {
            cuckoo.contains(generateRandomVal(size));
        } else if (num <= 9) {
            cuckoo.add(generateRandomVal(size));
        } else {
            cuckoo.remove(generateRandomVal(size));
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    // std::cout << "TOTAL EXECUTION TIME = "
    //           << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";

    return 0;
}

int generateRandomVal(int size) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, size);
    return distrib(gen);
}

int generateRandomInteger(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}
