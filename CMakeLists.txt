cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_FLAGS_RELEASE "-Os")

if(CMAKE_BUILD_TYPE EQUAL "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

project(
  Shijima-NX
  VERSION 1.0.0
  LANGUAGES CXX
)

# zlib
find_package(ZLIB REQUIRED)

# libshijima
set(SHIJIMA_USE_PUGIXML NO)
set(SHIJIMA_BUILD_EXAMPLES NO)
add_subdirectory(libs/libshijima)
include_directories(libs/libshijima)

# libnx
find_library(NX nx REQUIRED)
add_executable(${PROJECT_NAME})

# libtesla
include_directories(libs/libtesla/include)

target_sources(${PROJECT_NAME} PRIVATE
  source/main.cc
)
set_property(TARGET ${PROJECT_NAME}
  PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PRIVATE
  /opt/devkitpro/portlibs/switch/include)
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Werror -Wno-missing-field-initializers
  -Wno-unused-parameter -Wno-ignored-qualifiers -DSHIJIMA_NO_PUGIXML
  -Os
)
add_dependencies(${PROJECT_NAME} shijima)
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${NX}
  ZLIB::ZLIB
  shijima
)

# Here we go, builds executable
nx_create_nro(${PROJECT_NAME})
