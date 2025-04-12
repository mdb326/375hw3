#!/bin/bash

# Clean and build the program
make clean
make hw
# for i in {1..10}; do
#     ./test 65536
# done
./test 65536
# List of size parameters
# sizes=(1024 4096 32768 262144 1048576 4194304)
# sizes=(32768)

# # Loop over sizes
# for size in "${sizes[@]}"; do
#     echo "Running with size = $size" >> results.txt
#     ./test "$size" >> results.txt
#     echo "---------------------------------------------"
# done
