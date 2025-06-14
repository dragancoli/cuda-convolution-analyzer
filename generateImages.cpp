#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Funkcija za promenu veličine slike
bool resize_image(const string& input_path, const string& output_path, int size) {
    int width, height, channels;
    unsigned char* input_image = stbi_load(input_path.c_str(), &width, &height, &channels, 0);
    if (input_image == nullptr) {
        cerr << "Error: Could not open or find the image!" << endl;
        return false;
    }

    unsigned char* output_image = (unsigned char*)malloc(size * size * channels);
    if (output_image == nullptr) {
        cerr << "Error: Could not allocate memory for the output image!" << endl;
        stbi_image_free(input_image);
        return false;
    }

    stbir_resize_uint8_linear(input_image, width, height, 0, output_image, size, size, 0, (stbir_pixel_layout) channels);
    stbi_write_bmp(output_path.c_str(), size, size, channels, output_image);

    stbi_image_free(input_image);
    free(output_image);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Koristimo sliku: " << argv[0] << " <input_image>" << endl;
        return 1;
    }

    string input_path = argv[1];
    vector<pair<string, int>> output_images = {
        {"test_1000.bmp", 100},
        {"test_10000.bmp", 324},
        {"test_100000.bmp", 1000},
        {"test_1000000.bmp", 2048},
        {"test_10000000.bmp", 3084}
    };

    for (const auto& output : output_images) {
        if (resize_image(input_path, output.first, output.second)) {
            cout << "Generated: " << output.first << " with size: " << output.second << "x" << output.second << endl;
        }
    }

    return 0;
}