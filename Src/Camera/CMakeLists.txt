project(Camera)

SET(LIBRARY_OUTPUT_PATH ${lib_path}/mylib)

add_library(${PROJECT_NAME} Camera.cpp CameraSave.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}) 

get_property(INCLUDE_DIRS TARGET ${PROJECT_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Core Qt6::Gui)

# 链接ffmpeg
target_link_libraries(${PROJECT_NAME} PUBLIC avformat avcodec avutil swscale)

#判断
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
 # Debug 配置
    # 链接OpenCV库
    target_link_libraries(${PROJECT_NAME} PUBLIC opencv_world4110d)
 elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
 # Release 配置
    # 链接OpenCV库
    target_link_libraries(${PROJECT_NAME} PUBLIC opencv_world4110)
 else ()
 # 其他配置
 endif ()

# 链接自定义库
target_link_libraries(${PROJECT_NAME} PUBLIC PublicModule AI)

