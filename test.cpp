#include "sequential.h"
// #include "coarseConcurrent.h"
#include "concurrentBook.h"
#include "concurrentWithSets.h"
#include "concurrentLogical.h"
#include "concurrent.h"
#include "transactionalSynchronized.h"
#include "transactional.h"
#include <chrono>
#include <iostream>
#include <random>
#include <cstdlib>
#include <thread>


#define TESTAMT 33554432
// #define TESTAMT 67108864
#define THREADS 16
#define CONTAINSPER 8
#define ADDSPER 9


std::chrono::duration<double> times[THREADS];
int deltas[THREADS];
int generateRandomVal(int size);
int generateRandomInteger(int min, int max);
void do_work(ConcurrentCuckoo<int>& cuckoo, int threadNum, int iter, int size);
void do_workBook(ConcurrentBook<int>& cuckoo, int threadNum, int iter, int size);
void do_workSets(SetsConcurrent<int>& cuckoo, int threadNum, int iter, int size);
void do_workTransactional(TransactionalCuckoo<int>& cuckoo, int threadNum, int iter, int size);
void do_workSynch(SynchronizedCuckoo<int>& cuckoo, int threadNum, int iter, int size);

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
    // TransactionalCuckoo<int> transactional(size);
    // SynchronizedCuckoo<int> synchronize(size);
    // ConcurrentBook<int> cuckooBook(size);
    // SetsConcurrent<int> cuckooSets(size);
    // LogicalCuckoo<int> cuckooLogical(size);
    int startingSize = size/2;
    cuckoo.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    // transactional.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    // cuckooBook.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    // cuckooSets.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    // cuckooLogical.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    cuckooSeq.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    // synchronize.populate(startingSize, [size]() { return generateRandomVal(size*4); });
    std::thread threads[THREADS];
    int resultSize1 = startingSize;
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
        resultSize1 += deltas[i];
    }

    if(resultSize1 == cuckoo.size()){
        std::cout << "SUCCESS" << std::endl;
    }
    else{
        std::cout << "EXPECTED: " << resultSize1 << " ACTUAL: " << cuckoo.size()  << std::endl;
    }
    printf("Total Striped %d Threaded time: %lf seconds\n", THREADS, maxTime);

    // for(int i = 0; i < THREADS; i++){
    //     threads[i] = std::thread(do_workBook, std::ref(cuckooBook), i, TESTAMT/THREADS, size);
    // }

    // for (auto &th : threads){
    //     th.join();
    // }

    // maxTime = 0.0;
    // for(int i = 0; i < THREADS; i++){
    //     if(times[i].count() > maxTime){
    //         maxTime = times[i].count();
    //     }
    // }

    // printf("Total Book %d Threaded time: %lf seconds\n", THREADS, maxTime);

    // for(int i = 0; i < THREADS; i++){
    //     threads[i] = std::thread(do_workSets, std::ref(cuckooSets), i, TESTAMT/THREADS, size);
    // }

    // for (auto &th : threads){
    //     th.join();
    // }

    // maxTime = 0.0;
    // for(int i = 0; i < THREADS; i++){
    //     if(times[i].count() > maxTime){
    //         maxTime = times[i].count();
    //     }
    // }

    // printf("Total Sets %d Threaded time: %lf seconds\n", THREADS, maxTime);
    int resultSize = startingSize;
    auto begin1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TESTAMT; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= CONTAINSPER) {
            cuckooSeq.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            if(cuckooSeq.add(generateRandomVal(size))){
                resultSize++;
            }
        } else {
            if(cuckooSeq.remove(generateRandomVal(size))){
                resultSize--;
            }
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1);
    if(resultSize != cuckooSeq.size()){
        std::cout << "EXPECTED: " << resultSize << " ACTUAL: " << cuckooSeq.size()  << std::endl;
    }
    printf("Total Sequential time: %lf seconds\n", exec_time_i);

    // for(int i = 0; i < THREADS; i++){
    //     deltas[i] = 0;
    //     threads[i] = std::thread(do_workTransactional, std::ref(transactional), i, TESTAMT/THREADS, size);
    // }

    // for (auto &th : threads){
    //     th.join();
    // }
    // int resultSize3 = startingSize;
    // maxTime = 0.0;
    // for(int i = 0; i < THREADS; i++){
    //     if(times[i].count() > maxTime){
    //         maxTime = times[i].count();
    //         resultSize3 += deltas[i];
    //     }
    // }

    // // if(resultSize3 == transactional.size()){
    // //     std::cout << "SUCCESS" << std::endl;
    // // }
    // // else{
    // //     std::cout << "EXPECTED: " << resultSize3 << " ACTUAL: " << transactional.size()  << std::endl;
    // // }

    // printf("Total Transactional %d Threaded time: %lf seconds\n", THREADS, maxTime);


    // for(int i = 0; i < THREADS; i++){
    //     deltas[i] = 0;
    //     threads[i] = std::thread(do_workSynch, std::ref(synchronize), i, TESTAMT/THREADS, size);
    // }

    // for (auto &th : threads){
    //     th.join();
    // }
    // int resultSize4 = startingSize;
    // maxTime = 0.0;
    // for(int i = 0; i < THREADS; i++){
    //     if(times[i].count() > maxTime){
    //         maxTime = times[i].count();
    //         resultSize4 += deltas[i];
    //     }
    // }

    // if(resultSize3 == transactional.size()){
    //     std::cout << "SUCCESS" << std::endl;
    // }
    // else{
    //     std::cout << "EXPECTED: " << resultSize3 << " ACTUAL: " << transactional.size()  << std::endl;
    // }

    // printf("Total Synchronized %d Threaded time: %lf seconds\n", THREADS, maxTime);
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
        if (num <= CONTAINSPER) {
            cuckoo.contains(generateRandomVal(size), false);
        } else if (num <= ADDSPER) {
            if(cuckoo.add(generateRandomVal(size))){
                deltas[threadNum]++;
            }
        } else {
            if(cuckoo.remove(generateRandomVal(size))){
                deltas[threadNum]--;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}
void do_workBook(ConcurrentBook<int>& cuckoo, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= 8) {
            cuckoo.contains(generateRandomVal(size), false);
        } else if (num <= 9) {
            if(cuckoo.add(generateRandomVal(size))){
                deltas[threadNum]++;
            }
        } else {
            if(cuckoo.remove(generateRandomVal(size))){
                deltas[threadNum]--;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}
void do_workSets(SetsConcurrent<int>& cuckoo, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= 8) {
            cuckoo.contains(generateRandomVal(size), false);
        } else if (num <= 9) {
            if(cuckoo.add(generateRandomVal(size))){
                deltas[threadNum]++;
            }
        } else {
            if(cuckoo.remove(generateRandomVal(size))){
                deltas[threadNum]--;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}
void do_workTransactional(TransactionalCuckoo<int>& cuckoo, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= CONTAINSPER) {
            cuckoo.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            if(cuckoo.add(generateRandomVal(size))){
                deltas[threadNum]++;
            }
        } else {
            if(cuckoo.remove(generateRandomVal(size))){
                deltas[threadNum]--;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}
void do_workSynch(SynchronizedCuckoo<int>& cuckoo, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= CONTAINSPER) {
            cuckoo.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            if(cuckoo.add(generateRandomVal(size))){
                deltas[threadNum]++;
            }
        } else {
            if(cuckoo.remove(generateRandomVal(size))){
                deltas[threadNum]--;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}