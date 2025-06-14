#!/bin/bash

#kernel="-1 -1 -1 -1 8 -1 -1 -1 -1"
# Prosledjivanje kernela kao niz argumenata komandne linije
kernel="$1 $2 $3 $4 $5 $6 $7 $8 $9"

sizes=(1000 10000 100000 1000000 10000000)
mkdir results
mkdir output
mkdir gp_results
mkdir gp_output

# Pokretanje bez optimizacija
g++ -o Convolution Convolution.cpp
for size in "${sizes[@]}"; do
    ./Convolution test_${size}.bmp ./output/output_${size}_noopt_1.bmp $kernel > ./results/result_noopt_1_${size}.txt
done

for opt in O0 O1 O2 O3; do
    for threads in 1 2; do
        g++ -std=c++11 -fopenmp -march=native -mavx2 -${opt} -o Convolution Convolution.cpp

        for size in "${sizes[@]}"; do
            OMP_NUM_THREADS=${threads} ./Convolution test_${size}.bmp ./output/output_${size}_${opt}_${threads}.bmp $kernel > ./results/result_${opt}_${threads}_${size}.txt
        done
    done
done

nvcc -o GPUConvolution GPUConvolution.cu
for size in "${sizes[@]}"; do
    ./GPUConvolution test_${size}.bmp ./gp_output/output_${size}.bmp $kernel > ./gp_results/result_${size}.txt
done


# validacija rezultata
g++ -o validate_results validate_results.cpp
./validate_results

# Kreiranje grafova rezultata
g++ -o plot plot.cpp
./plot

# Brisanje nepotrebnih fajlova
rm Convolution
rm GPUConvolution
rm validate_results
rm plot
rm -r results
rm -r gp_results
rm data_noopt_1.dat
rm data_O0_1.dat
rm data_O1_1.dat
rm data_O2_1.dat
rm data_O3_1.dat
rm data_O0_2.dat
rm data_O1_2.dat
rm data_O2_2.dat
rm data_O3_2.dat
rm data_GP.dat
rm data_GP_var.dat
rm data_var_noopt_1.dat
rm data_var_O0_1.dat
rm data_var_O1_1.dat
rm data_var_O2_1.dat
rm data_var_O3_1.dat
rm data_var_O0_2.dat
rm data_var_O1_2.dat
rm data_var_O2_2.dat
rm data_var_O3_2.dat
rm plot_script.gp
