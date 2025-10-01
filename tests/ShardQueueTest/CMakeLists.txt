project(ShardQueueTest)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin)

add_executable(${PROJECT_NAME} test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE PublicModule catch)

add_test(NAME test1 COMMAND ShardQueueTest)