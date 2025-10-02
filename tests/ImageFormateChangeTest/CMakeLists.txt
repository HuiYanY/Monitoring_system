project(ImageFormateChangeTest)

set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH_TESTS}/ImageFormateChangeTest)

set(lib_path "C:/Users/HuiYan/Documents/VSCode/Monitoring_system/Src/Lib")

# OpenCV配置
set(OpenCV_LIB_DIR "${lib_path}/opencv")
# 查找OpenCV库文件
link_directories(${OpenCV_LIB_DIR}/lib)
# 设置opencv的lib的头文件位置
INCLUDE_DIRECTORIES(${OpenCV_LIB_DIR}/include)

# FFmpeg配置
include_directories(${lib_path}/LibFFmpeg/include)
link_directories(${lib_path}/LibFFmpeg/lib)

add_executable(${PROJECT_NAME} test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE PublicModule Qt6::Core Qt6::Gui catch)
#判断
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
 # Debug 配置
    # 链接OpenCV库
    target_link_libraries(${PROJECT_NAME} PRIVATE opencv_world4110d)
 elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
 # Release 配置
    target_link_libraries(${PROJECT_NAME} PRIVATE opencv_world4110)
 else ()
 # 其他配置
 endif ()

add_test(NAME test2 COMMAND ImageFormateChangeTest)
