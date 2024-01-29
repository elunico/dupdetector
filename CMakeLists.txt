cmake_minimum_required(VERSION 3.21)

project(dupdetect)

# set(CMAKE_CXX_COMPILER "/usr/local/Cellar/gcc/13.2.0/bin/g++-13")
set(CMAKE_CXX_FLAGS "-std=c++20 -g -fpermissive")
include_directories("/usr/local/Cellar/openssl@3/3.2.0_1/include/")
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(main ${SRC_FILES})

find_package(OpenSSL REQUIRED)
target_link_libraries(main OpenSSL::SSL)

# target_link_libraries(dupdetect OpenSSL::crypto)
