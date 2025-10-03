#include "CameraSave.hpp"
#include <QCoreApplication>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("CameraSave", "[CameraSave]") {

    // 1. 让 Qt 线程系统初始化（必需）
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    CameraSave save("rtsp://172.32.0.93/live/0");
    save.start();
    // 3. 等待 10 秒即可观察
    Sleep(15000);
    save.stopWork();
    REQUIRE(save.wait(10'000)); 
}