project(AI)

SET(LIBRARY_OUTPUT_PATH ${lib_path}/mylib)

add_library(${PROJECT_NAME} AI.cpp inference.cpp inference_GPU.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_lIST_DIR})

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui)

#判断
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
 # Debug 配置
    # 链接OpenCV库
    target_link_libraries(${PROJECT_NAME} PRIVATE opencv_world4110d)
 elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
 # Release 配置
    # 链接OpenCV库
    target_link_libraries(${PROJECT_NAME} PRIVATE opencv_world4110)
 else ()
 # 其他配置
 endif ()

# 链接PublicModule
target_link_libraries(${PROJECT_NAME} PRIVATE PublicModule)

# 链接onnxruntime
target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime onnxruntime_providers_cuda onnxruntime_providers_shared onnxruntime_providers_tensorrt)