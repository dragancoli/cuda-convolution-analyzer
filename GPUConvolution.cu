#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <algorithm>  
#include <cuda_runtime.h>

// Strukture za BMP
#pragma pack(push, 1)
struct BMPHeader {
    uint16_t fileType{0x4D42}; 
    uint32_t fileSize{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offsetData{0};
};

struct BMPInfoHeader {
    uint32_t size{0};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bitCount{0};
    uint32_t compression{0};
    uint32_t sizeImage{0};
    int32_t xPixelsPerMeter{0};
    int32_t yPixelsPerMeter{0};
    uint32_t colorsUsed{0};
    uint32_t colorsImportant{0};
};
#pragma pack(pop)

// Struktura za BMP podatke
struct BMP {
    BMPHeader header;
    BMPInfoHeader infoHeader;
    std::vector<uint8_t> data;

    bool load(const std::string &filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

        if (header.fileType != 0x4D42) {
            std::cerr << "Error: Not a BMP file" << std::endl;
            return false;
        }

        data.resize(infoHeader.width * infoHeader.height * 3);
        file.seekg(header.offsetData, file.beg);
        file.read(reinterpret_cast<char*>(data.data()), data.size());
        return true;
    }

    bool save(const std::string &filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }
};


// Pomoćna funkcija za clamp
__device__ int clampInt(int value, int low, int high) {
    if (value < low)   return low;
    if (value > high)  return high;
    return value;
}

// CUDA kernel za konvoluciju
__global__ void gpuConvolutionKernel(
    const uint8_t* input, 
    uint8_t* output, 
    const int* kernel,
    int width, 
    int height,
    int kernelSize
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    int kHalf = kernelSize / 2;

    if (x < kHalf || x >= (width - kHalf) || y < kHalf || y >= (height - kHalf)) {
        return;
    }

    int sumR = 0, sumG = 0, sumB = 0;

    for (int ky = -kHalf; ky <= kHalf; ky++) {
        for (int kx = -kHalf; kx <= kHalf; kx++) {
            int inX = x + kx;
            int inY = y + ky;
            int pixelIndex = (inY * width + inX) * 3;
            int kernelValue = kernel[(ky + kHalf) * kernelSize + (kx + kHalf)];

            sumR += static_cast<int>(input[pixelIndex])     * kernelValue;
            sumG += static_cast<int>(input[pixelIndex + 1]) * kernelValue;
            sumB += static_cast<int>(input[pixelIndex + 2]) * kernelValue;
        }
    }

    sumR = clampInt(sumR, 0, 255);
    sumG = clampInt(sumG, 0, 255);
    sumB = clampInt(sumB, 0, 255);

    int outIdx = (y * width + x) * 3;
    output[outIdx]     = static_cast<uint8_t>(sumR);
    output[outIdx + 1] = static_cast<uint8_t>(sumG);
    output[outIdx + 2] = static_cast<uint8_t>(sumB);
}

// GPU Funkcija za konvoluciju
void applyConvolutionGPU(
    const std::vector<uint8_t>& input,
    std::vector<uint8_t>& output,
    const std::vector<int>& kernel,
    int width,
    int height,
    int kernelSize
) {
    uint8_t *d_input = nullptr, *d_output = nullptr;
    int* d_kernel = nullptr;

    size_t imageSize = static_cast<size_t>(width) * height * 3 * sizeof(uint8_t);
    size_t kernelBytes = kernel.size() * sizeof(int);

    cudaMalloc(&d_input, imageSize);
    cudaMalloc(&d_output, imageSize);
    cudaMalloc(&d_kernel, kernelBytes);

    cudaMemcpy(d_input,  input.data(),  imageSize,    cudaMemcpyHostToDevice);
    cudaMemcpy(d_output, output.data(), imageSize,    cudaMemcpyHostToDevice);
    cudaMemcpy(d_kernel, kernel.data(), kernelBytes,  cudaMemcpyHostToDevice);

    dim3 block(16, 16);
    dim3 grid((width  + block.x - 1) / block.x,
              (height + block.y - 1) / block.y);

    gpuConvolutionKernel<<<grid, block>>>(d_input, d_output, d_kernel, width, height, kernelSize);
    cudaDeviceSynchronize();

    cudaMemcpy(output.data(), d_output, imageSize, cudaMemcpyDeviceToHost);

    cudaFree(d_input);
    cudaFree(d_output);
    cudaFree(d_kernel);
}

int main(int argc, char *argv[]) {
    std::string inputFile  = "test.bmp";
    std::string outputFile = "output.bmp";

    std::vector<int> kernel = {
        -1, -1,  -1,
        -1,  8, -1,
        -1, -1, -1
    };

    if (argc == 1) {
        // Nema argumenata, koristi podrazumevane vrednosti
        std::cout << "Using default input: test.bmp and output: output.bmp\n";
    }
    else if (argc >= 3) {
        inputFile  = argv[1];
        outputFile = argv[2];
        if (argc > 3) {
            kernel.clear();
            for (int i = 3; i < argc; ++i) {
                kernel.push_back(std::stoi(argv[i]));
            }
        }
    } else {
        std::cerr << "Usage: " << argv[0]
                  << " <input.bmp> <output.bmp> [kernel values...]\n";
        return 1;
    }

    BMP bmp;
    if (!bmp.load(inputFile)) {
        std::cerr << "Failed to load: " << inputFile << std::endl;
        return 1;
    }

    std::vector<uint8_t> output(bmp.data.size(), 0);

    for (int i = 0; i < 10; i++) {
        applyConvolutionGPU(bmp.data, output, kernel, bmp.infoHeader.width, bmp.infoHeader.height, 3);
    }

    double times[10];
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        applyConvolutionGPU(bmp.data, output, kernel, bmp.infoHeader.width, bmp.infoHeader.height, 3);
        auto end = std::chrono::high_resolution_clock::now();
        times[i] = std::chrono::duration<double>(end - start).count();
    }

    double sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += times[i];
    }
    
    double avg_time = sum/10;

    double variance = 0;
    for (int i = 0; i < 10; i++) {
        variance += pow(times[i] - avg_time, 2);
    }
    variance = variance/10;

    std::cout << "Vrijeme: " << avg_time  << " Varijansa: " << variance  << std::endl;


    bmp.data = output;
    if (!bmp.save(outputFile)) {
        std::cerr << "Failed to save: " << outputFile << std::endl;
        return 1;
    }
    return 0;
}