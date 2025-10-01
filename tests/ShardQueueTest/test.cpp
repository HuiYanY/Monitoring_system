#include "ShareQueue.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"


TEST_CASE("ShareQueue", "[ShareQueue]") {
    SharedQueue<int> queue(10); // 10 is the size of the queue

    SECTION("push 1 element") {
        queue.push(1);
        REQUIRE(queue.pop() == 1);
    }
    SECTION("push 5 element") {
        queue.push(1);
        queue.push(2);
        queue.push(3);
        queue.push(4);
        queue.push(5);
        
        REQUIRE(queue.pop() == 1);
        REQUIRE(queue.pop() == 2);
        REQUIRE(queue.pop() == 3);
        REQUIRE(queue.pop() == 4);
        REQUIRE(queue.pop() == 5);
    }
    SECTION("push 10 element") {
        queue.push(1);
        queue.push(2);
        queue.push(3);
        queue.push(4);
        queue.push(5);
        queue.push(6);
        queue.push(7);
        queue.push(8);
        queue.push(9);
        queue.push(10);
        
        
        REQUIRE(queue.pop() == 1);
        REQUIRE(queue.pop() == 2);
        REQUIRE(queue.pop() == 3);
        REQUIRE(queue.pop() == 4);
        REQUIRE(queue.pop() == 5);
        REQUIRE(queue.pop() == 6);
        REQUIRE(queue.pop() == 7);
        REQUIRE(queue.pop() == 8);
        REQUIRE(queue.pop() == 9);
        REQUIRE(queue.pop() == 10);

    }
    SECTION("push 11 element") {
        queue.push(1);
        queue.push(2);
        queue.push(3);
        queue.push(4);
        queue.push(5);
        queue.push(6);
        queue.push(7);
        queue.push(8);
        queue.push(9);
        queue.push(10);
        
        
        REQUIRE(queue.pop() == 1);
        REQUIRE(queue.pop() == 2);
        REQUIRE(queue.pop() == 3);
        REQUIRE(queue.pop() == 4);
        REQUIRE(queue.pop() == 5);
        REQUIRE(queue.pop() == 6);
        REQUIRE(queue.pop() == 7);
        REQUIRE(queue.pop() == 8);
        REQUIRE(queue.pop() == 9);
        REQUIRE(queue.pop() == 10);
        REQUIRE(queue.pop() == 10); // 10 is the size of the queue
    }
}

TEST_CASE("ShareQueue2", "[ShareQueue2]") {
    SharedQueue<int> queue(10); // 10 is the size of the queue
   
    for (int i = 0; i < 10; i++)
    {
        queue.push(i);
    }
    for (int i = 0; i < 10; i++)
    {
        REQUIRE(queue.pop() == i );
    }
}
