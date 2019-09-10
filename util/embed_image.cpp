#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void write_callback(void *context, void *dvoid, int size)
{
    std::ofstream &fout = *reinterpret_cast<std::ofstream *>(context);
    uint8_t *data = reinterpret_cast<uint8_t *>(dvoid);
    fout << std::hex;
    for (int i = 0; i < size; ++i) {
        fout << "0x" << static_cast<int>(data[i]);
        if (i + 1 < size) {
            fout << ",";
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <img> <name> <embed file.h> [-append]\n";
    }

    bool append = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-append") == 0) {
            append = true;
        }
    }

    int w, h, n;
    uint8_t *data = stbi_load(argv[1], &w, &h, &n, 4);

    std::ios_base::openmode mode = std::ios_base::out;
    if (append) {
        mode |= std::ios_base::app;
    }

    std::ofstream fout(argv[3], mode);
    if (!append) {
        fout << "#pragma once\n";
    }

    fout << "uint8_t " << argv[2] << "[] = {\n";

    stbi_write_png_to_func(write_callback, reinterpret_cast<void *>(&fout), w, 1, 4, data, w * 4);

    fout << "\n};\n";

    stbi_image_free(data);

    return 0;
}

