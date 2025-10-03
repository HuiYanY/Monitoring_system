cmake_minimum_required(VERSION 3.30)

project(Monitoring_system)

set(APP_NAME Monitoring_system)

if(MSVC)
    add_compile_options(
        "$<$<C_COMPILER_ID:MSVC>:/utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    )
endif()

# 设置根目录
set(root_path ${CMAKE_CURRENT_LIST_DIR})

set(Vulkan_SDK "C:/VulkanSDK/1.4.321.1")
set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.4.321.1/Include")
set(Vulkan_LIBRARY "C:/VulkanSDK/1.4.321.1/Lib")
find_package(Vulkan REQUIRED)

if(Vulkan_glslc_FOUND AND Vulkan_glslangValidator_FOUND)
    message(STATUS "Found glslc and glslangValidator")
else()
    message(FATAL_ERROR "Missing required Vulkan components: glslc and glslangValidator")
endif()

set(CMAKE_BUILD_TYPE "Debug") # Debug Release

# bin 文件路径
if(MSVC)
    set(OUTPUT_PATH_EXE ${CMAKE_CURRENT_LIST_DIR}/bin)
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(OUTPUT_PATH_EXE ${CMAKE_CURRENT_LIST_DIR}/bin/Release)
        message(STATUS "-- Release build selected")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OUTPUT_PATH_EXE ${CMAKE_CURRENT_LIST_DIR}/bin/Debug)
        message(STATUS "-- Debug build selected")
    else()
        message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()
set(OUTPUT_PATH_TESTS ${CMAKE_CURRENT_LIST_DIR}/bin/Tests)

# Qt6配置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
      set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/msvc2022_64")
set(Qt6_DIR Qt6_DIR C:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_subdirectory(Src)

enable_testing()
add_subdirectory(tests)