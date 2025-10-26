# 🚀 GPGPU Acceleration of 2D Image Convolution

This project explores and demonstrates performance optimization techniques for the computationally intensive operation of 2D discrete convolution on images.  
Starting from a basic sequential implementation, the code is progressively optimized using compiler flags, OpenMP parallelization, and finally, a **GPGPU variant** implemented to achieve maximum speedup.

The project is written in C++ and includes a custom parser for 24-bit BMP files, as well as a detailed mechanism for performance measurement and analysis.

---

## ✨ Key Features

* **2D Convolution**: Implementation of an algorithm for applying arbitrary convolution kernels to images (e.g., *Box Blur*, *Sharpen*, *Edge Detection*).
* **Custom BMP Parser**: Manually implemented parser for loading and saving uncompressed 24-bit BMP images.
* **Command-line Parameterization**: Supports specifying input/output paths and convolution kernel values.
* **Comprehensive Performance Analysis**: Systematic comparison of different algorithm variants:
    * Sequential vs. Compiler-optimized (`-O0` to `-O3`, SIMD).
    * Sequential vs. Multicore (OpenMP).
    * CPU (OpenMP) vs. **GPGPU (CUDA)**.
* **Automated Measurement Script**: Includes a `bash` script that automatically executes all benchmarks for different input sizes and configurations, and stores the results.

---

## 🛠️ Implemented Algorithm Variants

1. **Sequential variant**: The baseline, unoptimized implementation used as the starting point for performance measurements.  
2. **Compiler-optimized variant**: Uses high-level compiler optimizations (`-O3`) and automatic SIMD vectorization (`-march=native`).  
3. **Multicore (OpenMP) variant**: CPU parallelization using the OpenMP library to scale across multiple cores.  
4. **GPGPU variant**: The fastest version implemented in **CUDA C++**, performing convolution on the massively parallel GPU architecture.

---

## 🚀 How to Run

### Prerequisites
* GCC/G++ compiler  
* NVIDIA CUDA Toolkit  

### Execution
The program is executed from the command line.  
The convolution kernel is passed as a string of numbers separated by commas.

```bash
# The script script.sh runs all benchmarks for a kernel that must be specified in the command line.
# It requires images of the following sizes to be generated: 1000, 10000, 100000, 1000000, 10000000 pixels.
# To generate images, use the generateImages program by providing the input image path and the output folder path.
---
```

📊 Measurement Automation

The run_benchmarks.sh script runs all defined tests for different algorithm variants and image sizes, generating a results.csv file ready for plotting charts.

## 📈 Results and Analysis

The analysis showed that the GPGPU variant provides a drastic speedup compared to all CPU-based implementations, including the OpenMP parallelized version.
The acceleration increases with image size, confirming the efficiency of the GPGPU approach for large-scale data processing.

Detailed charts and analysis are provided in the accompanying report (`Izvjestaj.pdf`).

![Grafik Ubrzanja](performance_comparison.png)  

---

## ⚙️ Technical Details

* **BMP Parser**: The parser reads the `BMP File Header` and `DIB Header`to obtain image dimensions and the pixel array offset. It supports 24-bit format (8 bits per channel – BGR).
* **GPGPU Implementation**:
    * **Kernel**: CUDA C++ kernel is designed so that each thread processes one output pixel.
    * **Memory**: Optimized memory transfer between `host` (CPU) and `device` (GPU) memory.
    * **Thread Structure**: A 2D grid of thread blocks is launched to match the image dimensions.

---
