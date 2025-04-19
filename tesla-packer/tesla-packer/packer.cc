#include "packer.hpp"
#include <iostream>
#include <zlib.h>

namespace tesla_packer {

static void write(uint16_t num, std::ostream &out) {
    uint8_t bytes[2] = { (uint8_t)(num >> 8), (uint8_t)num };
    out.write((const char *)bytes, 2);
}

static void write(std::string const& str, std::ostream &out) {
    write((uint16_t)str.size(), out);
    out.write(str.c_str(), str.size());
}

void pack(std::filesystem::path src, std::ostream &out) {
    std::filesystem::directory_iterator iter { src };
    for (auto entry : iter) {
        if (!entry.is_regular_file()) {
            continue;
        }
        auto path = entry.path();
        if (path.filename().string()[0] == '.') {
            continue;
        }
        if (path.extension() != ".png") {
            continue;
        }
        std::cout << "packing: " << path << std::endl;
        int x, y, comp;
        auto data = stbi_load(path.c_str(), &x, &y, &comp, 4);
        if (data == NULL) {
            const char *err = stbi_failure_reason();
            if (err == NULL) err = "(null)";
            std::cerr << "ERROR: stbi_load() failed: " << err << std::endl;
            continue;
        }
        write((uint16_t)x, out);
        write((uint16_t)y, out);
        write(path.stem().string(), out);

        // in-place rgba8888 -> rgba4444 conversion
        for (int i=0; i<x*y; ++i) {
            uint8_t r = data[i*4], g = data[i*4+1], b = data[i*4+2], a = data[i*4+3];
            data[i*2] = (r & 0xF0) | (g >> 4);
            data[i*2+1] = (b & 0xF0) | (a >> 4);
        }

        // compress
        uLongf compressedSize = (uLongf)(x * y * 2);
        compress(&data[x * y * 2], &compressedSize, &data[0],
            compressedSize);

        // write
        write(compressedSize, out);
        out.write((const char *)&data[x * y * 2], compressedSize);

        // cleanup
        stbi_image_free(data);
    }
}

}