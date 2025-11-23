cmake_minimum_required(VERSION 3.10)

project(SpMV_Project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

include_directories(src)
include_directories(src/mmio)

# mmio.c compilato come libreria C pura
add_library(mmio STATIC src/mmio/mmio.c)

# Sorgenti C++
set(SOURCES
        src/read_mtx.cpp
        src/coo_to_csr.cpp
        src/spmv.cpp
        src/utils/vector_utils.cpp
        src/main.cpp
)

add_executable(spmv ${SOURCES})

target_link_libraries(spmv PRIVATE mmio)