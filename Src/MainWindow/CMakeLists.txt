project(MainWindow)

SET(LIBRARY_OUTPUT_PATH ${lib_path}/mylib)

add_library(${PROJECT_NAME} MainWindow.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR} Camera AI Login)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui)

# 链接自定义库
target_link_libraries(${PROJECT_NAME} PRIVATE Camera AI Login)