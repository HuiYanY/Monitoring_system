project(ImageFormateChangeTest)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin)

# Qt5配置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt5_DIR C:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

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

target_link_libraries(${PROJECT_NAME} PRIVATE PublicModule Qt5::Core Qt5::Gui catch)
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
