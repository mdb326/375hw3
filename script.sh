#!/bin/bash

# Clean and build the program
make clean
make hw
./test 128

# List of size parameters
# sizes=(128 1024 4096 32768 262144 1048576 4194304 16777216)
# sizes=(128)

# runs=10

# for size in "${sizes[@]}"; do
#     # echo "Testing with size = $size"
#     total_time=0

#     for i in $(seq 1 $runs); do
#         # Run the program with the current size and capture the output
#         output=$(./test "$size")

#         # Extract the execution time (assuming format: "TOTAL EXECUTION TIME = 1234567")
#         time=$(echo "$output" | grep -oP '(?<=TOTAL EXECUTION TIME = )\d+')

#         # Print the extracted time for debugging
#         #echo "Run $i: Execution time = $time µs"

#         # Add to total time
#         total_time=$((total_time + time))
#     done

#     # Calculate and print the average execution time
#     average_time=$((total_time / runs))
#     echo "$average_time"
#     # echo "-----------------------------------------"
# done
