cmake_minimum_required(VERSION 3.18)

if(CMAKE_BUILD_TYPE EQUAL "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

project(
  Shijima-NX
  VERSION 1.0.0
  LANGUAGES CXX
)

# libnx
find_library(NX nx REQUIRED)
add_executable(${PROJECT_NAME})

# libtesla
include_directories(libs/libtesla/include)

target_sources(${PROJECT_NAME} PRIVATE
  source/main.cc
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${NX}
)

# Here we go, builds executable
nx_create_nro(${PROJECT_NAME})
