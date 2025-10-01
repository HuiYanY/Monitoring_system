project(tests)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(catch "${CMAKE_CURRENT_LIST_DIR}/lib")

add_subdirectory(main)
add_subdirectory(ImageFormateChangeTest)
add_subdirectory(ShardQueueTest)