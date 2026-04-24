cmake_minimum_required(VERSION 3.10)
project(VexobenOrangeJuice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(code solution.cpp)

# Compiler optimizations
set_target_properties(code PROPERTIES
    CXX_FLAGS_RELEASE "-O3 -DNDEBUG"
)

target_compile_options(code PRIVATE
    -O3
    -march=native
    -mtune=native
    -ffast-math
)
