#!/usr/bin/env bash
g++ -o generateImages generateImages.cpp
./generateImages "$1"

kernel1="-1 -1 -1 -1 8 -1 -1 -1 -1"
kernel2="0 -1 0 -1 5 -1 0 -1 0"
kernel3="2 3 -2 -1 9 4 3 2 11"

KERNEL_ARRAY=("$kernel1" "$kernel2" "$kernel3")

for i in 0 1 2; do
    current_kernel="${KERNEL_ARRAY[$i]}"
    echo "Kernel #$((i+1)): $current_kernel"

    ./script.sh $current_kernel

    echo "Kraj merenja za ovaj kernel."

    mv performance_comparison_with_variance.png "performance_comparison_with_variance_$((i+1)).png"
    mv performance_comparison.png "performance_comparison_$((i+1)).png"
    mv all_results.txt "all_results_$((i+1)).txt"
done

rm generateImages