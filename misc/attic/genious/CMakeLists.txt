project(tankgame)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Release)

find_package(FMT REQUIRED)
find_package(LLVM REQUIRED)
find_package(CLANG REQUIRED)
find_package(clang REQUIRED)
find_package(Clang REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE genious_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    )

add_executable(genious ${genious_sources})
target_include_directories(genious PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    #${LLVM_INCLUDE_DIRS}
    )
target_link_libraries(genious PRIVATE
    Threads::Threads
    fmt::fmt
    #llvm::llvm
    #${LLVM_LIBRARIES}
    clangTooling
    clangBasic
    clangASTMatchers
    )

if(WIN32)
    target_compile_definitions(server PRIVATE
        WINDOWS=1
        _USE_MATH_DEFINES=1
        NOMINMAX=1
        NOGDI=1
        )
    target_link_libraries(genious PRIVATE ws2_32)
    target_compile_options(genious PRIVATE /MP)
else()
    target_compile_definitions(server PRIVATE LINUX=1)
endif()

target_compile_features(server PRIVATE cxx_std_20)
