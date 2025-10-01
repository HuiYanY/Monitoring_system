cmake_minimum_required(VERSION 3.30)

project(Monitoring_system)

if(MSVC)
    add_compile_options(
        "$<$<C_COMPILER_ID:MSVC>:/utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    )
endif()

set(CMAKE_BUILD_TYPE "Release") # Debug Release

add_subdirectory(Src)

enable_testing()
add_subdirectory(tests)