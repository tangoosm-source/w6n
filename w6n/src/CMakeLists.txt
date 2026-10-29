cmake_minimum_required(VERSION 3.15)
project(asmanalyzer CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAPSTONE REQUIRED capstone)

add_executable(asmanalyzer
    src/main.cpp
    src/disasm/engine.cpp
    src/loader/elf_loader.cpp
    src/analysis/roundtrip.cpp
    src/asm/opcode_byte_builder.cpp
    src/asm/opcode_byte.cpp
)
target_include_directories(asmanalyzer PRIVATE src ${CAPSTONE_INCLUDE_DIRS})
target_link_directories(asmanalyzer PRIVATE ${CAPSTONE_LIBRARY_DIRS})
target_link_libraries(asmanalyzer PRIVATE ${CAPSTONE_LIBRARIES})
