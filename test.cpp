#include "sequential.h"
#include "concurrent.h"
#include <chrono>
#include <iostream>
#include <random>
#include <cstdlib>
#include <thread>


#define TESTAMT 20000000
#define THREADS 16

std::chrono::duration<double> times[THREADS];
int generateRandomVal(int size);
int generateRandomInteger(int min, int max);
void do_work(ConcurrentCuckoo<int>& cuckoo, int threadNum, int iter, int size);

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

    SequentialCuckoo<int> cuckooSeq(size);
    ConcurrentCuckoo<int> cuckoo(size);
    cuckoo.populate(size / 2, [size]() { return generateRandomVal(size); });
    cuckooSeq.populate(size/2, [size]() { return generateRandomVal(size); });

    
    std::thread threads[THREADS];
    for(int i = 0; i < THREADS; i++){
        threads[i] = std::thread(do_work, std::ref(cuckoo), i, TESTAMT/THREADS, size);
    }

    for (auto &th : threads){
        th.join();
    }

    double maxTime = 0.0;
    for(int i = 0; i < THREADS; i++){
        if(times[i].count() > maxTime){
            maxTime = times[i].count();
        }
    }

    printf("Total %d Threaded time: %lf seconds\n", THREADS, maxTime);

    

    // std::cout << "TOTAL EXECUTION TIME = "
    //     << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";


    auto begin1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TESTAMT; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= 8) {
            cuckooSeq.contains(generateRandomVal(size));
        }
        // } else if (num <= 9) {
        //     cuckooSeq.add(generateRandomVal(size));
        // } else {
        //     cuckooSeq.remove(generateRandomVal(size));
        // }
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1);

    printf("Total Sequential time: %lf seconds\n", exec_time_i);

    return 0;
}

int generateRandomVal(int size) {
    thread_local static std::random_device rd;
    thread_local static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, size);
    return distrib(gen);
}

int generateRandomInteger(int min, int max) {
    thread_local static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    thread_local static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}

void do_work(ConcurrentCuckoo<int>& cuckoo, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= 8) {
            cuckoo.contains(generateRandomVal(size), false);
        }
        // } else if (num <= 9) {
        //     cuckoo.add(generateRandomVal(size));
        // } else {
        //     cuckoo.remove(generateRandomVal(size));
        // }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}