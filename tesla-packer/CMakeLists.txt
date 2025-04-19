cmake_minimum_required(VERSION 3.14)
project(tesla-packer-cli)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(tesla-packer)
include_directories(.)

add_executable(tesla-packer-cli
    main.cc
)

target_compile_options(tesla-packer-cli PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

add_dependencies(tesla-packer-cli tesla-packer)
target_link_libraries(tesla-packer-cli tesla-packer)

