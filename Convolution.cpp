#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <omp.h>
#include <math.h>

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


// Primjena konvolucije
void applyConvolution(const std::vector<uint8_t> &input, std::vector<uint8_t> &output, int width, int height, const std::vector<int> &kernel, int kernelSize) {
    int kHalf = kernelSize / 2;

    #pragma omp parallel for
    for (int y = kHalf; y < height - kHalf; ++y) {
        for (int x = kHalf; x < width - kHalf; ++x) {
            int sumR = 0, sumG = 0, sumB = 0;
            for (int ky = -kHalf; ky <= kHalf; ++ky) {
                for (int kx = -kHalf; kx <= kHalf; ++kx) {
                    int pixelIndex = ((y + ky) * width + (x + kx)) * 3;
                    int kernelValue = kernel[(ky + kHalf) * kernelSize + (kx + kHalf)];
                    sumR += input[pixelIndex] * kernelValue;
                    sumG += input[pixelIndex + 1] * kernelValue;
                    sumB += input[pixelIndex + 2] * kernelValue;
                }
            }
            int outputIndex = (y * width + x) * 3;
            output[outputIndex] = std::min(std::max(sumR, 0), 255);
            output[outputIndex + 1] = std::min(std::max(sumG, 0), 255);
            output[outputIndex + 2] = std::min(std::max(sumB, 0), 255);
        }
    }
}

#include <chrono>
#include <iostream>

int main(int argc, char *argv[]) {
   // Ako nema argumenata komande linije koristicemo podrazumjevane vrijdenosti
   if(argc == 1) {
        BMP bmp;
        if (!bmp.load("test.bmp")) {
            return 1;
        }  
        std::vector<int> kernel = {0, -1, 0, -1, 5, -1, 0, -1, 0}; // Default sharpening kernel
        //std::vector<int> kernel = {-1, -1, -1, -1, 8, -1, -1, -1, -1}; 
        std::vector<uint8_t> output(bmp.data.size());

        for (int i = 0; i < 10; i++) {
            applyConvolution(bmp.data, output, bmp.infoHeader.width, bmp.infoHeader.height, kernel, 3);
        }

        auto start = std::chrono::high_resolution_clock::now();
        applyConvolution(bmp.data, output, bmp.infoHeader.width, bmp.infoHeader.height, kernel, 3);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        std::cout << "Time taken: " << diff.count() << " s" << std::endl;

        bmp.data = output;
        bmp.save("output.bmp");
   }
   else {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " <input.bmp> <output.bmp> [kernel values...]" << std::endl;
            return 1;
        }

        BMP bmp;
        if (!bmp.load(argv[1])) {
            return 1;
        }   

        //std::vector<int> kernel = {0, -1, 0, -1, 5, -1, 0, -1, 0}; // Default sharpening kernel
        std::vector<int> kernel = {-1, -1, -1, -1, 8, -1, -1, -1, -1}; // Edge detection kernel
        if (argc > 3) {
            kernel.clear();
            for (int i = 3; i < argc; ++i) {
                kernel.push_back(std::stoi(argv[i]));
            }
        }

        std::vector<uint8_t> output(bmp.data.size());
        for (int i = 0; i < 10; i++) {
            applyConvolution(bmp.data, output, bmp.infoHeader.width, bmp.infoHeader.height, kernel, 3);
        }

        double total = 0.0;
        double *times = new double[10];
        for(int i=0; i<10; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            applyConvolution(bmp.data, output, bmp.infoHeader.width, bmp.infoHeader.height, kernel, 3);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            double time = diff.count();
            total += time;
            times[i] = time;
        }

        double avg_time = total/10.0;
        double variance = 0.0;

        for(int i=0; i<10; i++) {
            variance += pow(times[i] - avg_time, 2);
        }
        
        variance = variance/10;
        std::cout << "Vrijeme: " << avg_time  << " Varijansa: " << variance  << std::endl;
        bmp.data = output;
        bmp.save(argv[2]);
    }

    return 0;
}