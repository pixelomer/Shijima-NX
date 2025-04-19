#include <iostream>
#include <tesla-packer/packer.hpp>
#include <fstream>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <img/> <out.bin>" << std::endl;
        return EXIT_FAILURE;
    }
    std::ofstream out;
    out.open(argv[2], std::ios::binary | std::ios::out);
    tesla_packer::pack(argv[1], out);
    return EXIT_SUCCESS;
}
