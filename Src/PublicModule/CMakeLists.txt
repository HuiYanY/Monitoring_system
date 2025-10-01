project (PublicModule)

SET(LIBRARY_OUTPUT_PATH ${lib_path}/mylib)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
INTERFACE
${CMAKE_CURRENT_LIST_DIR}
)