project(catch)

set(CMAKE_CXX_STANDARD 17)

SET(LIBRARY_OUTPUT_PATH ${catch})

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
INTERFACE
${CMAKE_CURRENT_LIST_DIR}
)