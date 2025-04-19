cmake_minimum_required(VERSION 3.14)
project(tesla-packer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(ZLIB)

include_directories(..)

add_library(
    tesla-packer
    packer.cc
    stb_image.cc
)

target_compile_options(tesla-packer PRIVATE
    -Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers
)
target_link_libraries(tesla-packer ZLIB::ZLIB)
