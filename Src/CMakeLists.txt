project(Src)

set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH_EXE})

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

message("-- Build Type: ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_executable(${APP_NAME} main.cpp resource.rc ${ProjectResource})
    target_link_libraries(${APP_NAME} PRIVATE Qt6::Widgets MainWindow opencv_world4110d)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_executable(${APP_NAME} WIN32 main.cpp resource.rc ${ProjectResource})
    target_link_libraries(${APP_NAME} PRIVATE Qt6::Widgets MainWindow opencv_world4110)
endif()

