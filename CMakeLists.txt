cmake_minimum_required(VERSION 3.18)

if(CMAKE_BUILD_TYPE EQUAL "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

project(
  Shijima-NX
  VERSION 1.0.0
  LANGUAGES CXX
)

# libshijima
set(SHIJIMA_USE_PUGIXML NO)
set(SHIJIMA_BUILD_EXAMPLES NO)
add_subdirectory(libs/libshijima)
include_directories(libs/libshijima)

# qutex
add_subdirectory(libs/qutex/loader)
include_directories(libs/qutex)

# libnx
find_library(NX nx REQUIRED)
add_executable(${PROJECT_NAME})

# libtesla
include_directories(libs/libtesla/include)

target_sources(${PROJECT_NAME} PRIVATE
  source/main.cc
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
add_dependencies(${PROJECT_NAME} shijima qutex-loader)
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${NX}
  shijima
  qutex-loader
)

# Here we go, builds executable
nx_create_nro(${PROJECT_NAME})
