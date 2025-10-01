project(Src)

# Qt5配置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin)
set(Qt5_DIR C:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# 设置lib路径
set(lib_path ${CMAKE_CURRENT_LIST_DIR}/Lib)

# FFmpeg配置
include_directories(${lib_path}/LibFFmpeg/include)
link_directories(${lib_path}/LibFFmpeg/lib)

# CUDA配置
find_package(CUDAToolkit  REQUIRED)
include_directories(${CUDAToolkit_INCLUDE_DIRS})
# CUDNN配置
include_directories(${lib_path}/LibCUDNN/include)
link_directories(${lib_path}/LibCUDNN/lib)

# OpenCV配置
set(OpenCV_LIB_DIR "${lib_path}/opencv")
# 查找OpenCV库文件
link_directories(${OpenCV_LIB_DIR}/lib)
# 设置opencv的lib的头文件位置
INCLUDE_DIRECTORIES(${OpenCV_LIB_DIR}/include)

# onnxruntime配置
include_directories(${lib_path}/LibONNXRuntime/include)
include_directories(${lib_path}/LibONNXRuntime/include/core/providers)
include_directories(${lib_path}/LibONNXRuntime/include/core/providers/cuda)
link_directories(${lib_path}/LibONNXRuntime/lib)

# 项目所需源文件和头文件查找
include_directories(${CMAKE_CURRENT_LIST_DIR}/AI)
include_directories(${CMAKE_CURRENT_LIST_DIR}/Camera)
include_directories(${CMAKE_CURRENT_LIST_DIR}/Login)
include_directories(${CMAKE_CURRENT_LIST_DIR}/MainWindow)
include_directories(${CMAKE_CURRENT_LIST_DIR}/PublicModule)
file(GLOB ProjectResource "${CMAKE_CURRENT_LIST_DIR}/Resource/resource.qrc")
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_LIST_DIR}/Resource/Forms")
link_directories(${lib_path}/mylib/Debug)

add_subdirectory(AI)
add_subdirectory(Camera)
add_subdirectory(Login)
add_subdirectory(MainWindow)
add_subdirectory(PublicModule)

message("Build Type: ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_executable(${PROJECT_NAME} main.cpp resource.rc ${ProjectResource})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets MainWindow opencv_world4110d)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_executable(${PROJECT_NAME} WIN32 main.cpp resource.rc ${ProjectResource})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets MainWindow opencv_world4110)
endif()

