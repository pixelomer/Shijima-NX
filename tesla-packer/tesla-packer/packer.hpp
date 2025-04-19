#pragma once

#include "stb_image.h"
#include <filesystem>
#include <ostream>

namespace tesla_packer {

void pack(std::filesystem::path src, std::ostream &out);

}