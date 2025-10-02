project(ShardQueueTest)

set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH_TESTS}/ShardQueueTest)

add_executable(${PROJECT_NAME} test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE PublicModule catch)

add_test(NAME test1 COMMAND ShardQueueTest)