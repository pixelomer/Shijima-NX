// 
// Shijima-NX - Shimeji desktop pet runner for Nintendo Switch
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

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
