#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

// Provera da li dva fajla imaju identične bajtove.
bool filesAreIdentical(const std::string& filePathA, const std::string& filePathB)
{
    std::ifstream fileA(filePathA, std::ios::binary | std::ios::ate);
    std::ifstream fileB(filePathB, std::ios::binary | std::ios::ate);

    if(!fileA.is_open() || !fileB.is_open()) {
        std::cerr << "Neuspešno otvaranje fajla: " 
                  << filePathA << " ili " << filePathB << std::endl;
        return false;
    }

    std::streamsize sizeA = fileA.tellg();
    std::streamsize sizeB = fileB.tellg();
    if(sizeA != sizeB) {
        return false;
    }

    fileA.seekg(0, std::ios::beg);
    fileB.seekg(0, std::ios::beg);

    const size_t bufferSize = 4096;
    std::vector<char> bufferA(bufferSize);
    std::vector<char> bufferB(bufferSize);

    while (!fileA.eof() && !fileB.eof()) {
        fileA.read(bufferA.data(), bufferSize);
        fileB.read(bufferB.data(), bufferSize);

        std::streamsize bytesReadA = fileA.gcount();
        std::streamsize bytesReadB = fileB.gcount();

        if(bytesReadA != bytesReadB) {
            return false;
        }
        if(!std::equal(bufferA.begin(), bufferA.begin() + bytesReadA, bufferB.begin())) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    std::vector<long long> sizes = { 1000, 10000, 100000, 1000000, 10000000 };
    // Provjera da li je unesena putanja kao argument komande linije
    std::string outputDir = "./output";
    if(argc > 1) {
        outputDir = argv[1];
    }

    for (auto sizeVal : sizes) {
        std::vector<std::string> candidateFiles;
        
        for (const auto& entry : std::filesystem::directory_iterator(outputDir)) {
            if (!entry.is_regular_file()) continue;
            std::string path = entry.path().string();
            std::string token = "output_" + std::to_string(sizeVal) + "_";
            if (path.find(token) != std::string::npos) {
                candidateFiles.push_back(path);
            }
        }

        if (candidateFiles.empty()) {
            std::cout << "Nema fajlova za size " << sizeVal << " u folderu " << outputDir << std::endl;
            continue;
        }

        std::string gpOutputDir = "./gp_output";
        std::string gpOutputFile = gpOutputDir + "/output_" + std::to_string(sizeVal) + ".bmp";
        if (std::filesystem::exists(gpOutputFile)) {
            candidateFiles.push_back(gpOutputFile);
        }

        std::string referenceFile = candidateFiles.front();
        
        bool allMatch = true;
        for (size_t i = 1; i < candidateFiles.size(); i++) {
            if (!filesAreIdentical(referenceFile, candidateFiles[i])) {
                allMatch = false;
                std::cout << "Razlika pronađena za size " << sizeVal
                          << " izmedju:\n  " << referenceFile
                          << "\n  i " << candidateFiles[i] << std::endl;
            }
        }

        if (allMatch && candidateFiles.size() > 1) {
            std::cout << "Svi fajlovi za size " << sizeVal
                      << " su proizveli validan rezultat (" << candidateFiles.size()
                      << " fajlova poređeno).\n";
        } else if (candidateFiles.size() == 1) {
            std::cout << "Postoji samo jedan fajl za size " 
                      << sizeVal << " (" << referenceFile
                      << "), ništa za poređenje.\n";
        }
    }

    return 0;
}