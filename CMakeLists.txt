cmake_minimum_required(VERSION 3.12)
project(rdma_zero_copy_ipc C)

# Set C standard to C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable compiler warnings
add_compile_options(-Wall -Wextra -O2)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Explicitly find libibverbs with fallback names and standard library search paths
find_library(IBVERBS_LIB 
    NAMES verbs ibverbs
    PATHS 
        /usr/lib 
        /usr/lib/x86_64-linux-gnu 
        /usr/lib64
        /usr/local/lib
    REQUIRED
)

# Create core static library from src/
add_library(rdma_core STATIC
    src/rdma_context.c
    src/rdma_memory.c
)

# Day 1 Executable Target
add_executable(test_day1 tests/test_day1_init.c)
target_link_libraries(test_day1 PRIVATE rdma_core ${IBVERBS_LIB})

# Day 2 Executable Target (MANDATORY: must link rdma_core and IBVERBS_LIB)
add_executable(test_day2 tests/test_day2_memory.c)
target_link_libraries(test_day2 PRIVATE rdma_core ${IBVERBS_LIB})
