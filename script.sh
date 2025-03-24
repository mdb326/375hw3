#!/bin/bash

# Clean and build the program
make clean
make hw

total_time=0
runs=10

# Run the test 10 times
for i in $(seq 1 $runs); do
    # Run the program and capture the output
    output=$(./test)

    # Extract the execution time (assuming format: "TOTAL EXECUTION TIME = 1234567")
    time=$(echo "$output" | grep -oP '(?<=TOTAL EXECUTION TIME = )\d+')

    # Print the extracted time for debugging
    echo "Run $i: Execution time = $time µs"

    # Add to total time
    total_time=$((total_time + time))
done

# Calculate and print the average execution time
average_time=$((total_time / runs))
echo "AVERAGE EXECUTION TIME = $average_time µs"
