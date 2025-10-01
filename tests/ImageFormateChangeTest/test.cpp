#include "ImageFormateChange.h"
#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// 测试ARGB32格式
TEST_CASE("QImage2cvMatTestARGB32", "[QImage2cvMatTest]") {
    QImage image(100, 80, QImage::Format_ARGB32);
    image.fill(QColor(255, 128, 64, 200)); // 填充ARGB颜色
    
    cv::Mat mat = QImage2cvMat(image);

    int a = 1, b = 2, c = 3;
    WARN("按键的值是 " << a << " " << b << " " << c);
    CAPTURE( a, b, c, a + b, c > b, a == 1);
    
    INFO("QImage2cvMatTestARGB32");
    REQUIRE(mat.rows == 80);
    UNSCOPED_INFO("QImage2cvMatTestARGB32 UNSCOPED_INFO");
    REQUIRE(mat.cols == 100);
    REQUIRE(mat.type() == CV_8UC4);
    REQUIRE(mat.channels() == 4);
    REQUIRE(mat.empty() == 0);
}

// 测试RGB32格式
TEST_CASE("QImage2cvMatTestRGB32", "[QImage2cvMatTest]") {
    QImage image(100, 80, QImage::Format_RGB32);
    image.fill(QColor(255, 0, 128));
    
    cv::Mat mat = QImage2cvMat(image);
    
    REQUIRE(mat.rows == 80);
    REQUIRE(mat.cols == 100);
    REQUIRE(mat.type() == CV_8UC4);
    REQUIRE(mat.channels() == 4);
    REQUIRE(mat.empty() == 0);
}

// 测试RGB888格式
TEST_CASE("QImage2cvMatTestRGB888", "[QImage2cvMatTest]") {
    QImage image(120, 90, QImage::Format_RGB888);
    image.fill(QColor(255, 128, 64));
    
    cv::Mat mat = QImage2cvMat(image);
    
    REQUIRE(mat.rows == 90);
    REQUIRE(mat.cols == 120);
    REQUIRE(mat.type() == CV_8UC3);
    REQUIRE(mat.channels() == 3);
    REQUIRE(mat.empty() == 0);
    
    // 验证颜色通道交换是否正确（RGB转BGR）
    cv::Vec3b pixel = mat.at<cv::Vec3b>(0, 0);
    // 由于进行了RGB到BGR的转换，红色通道应该在最后
    REQUIRE(pixel[0] == 64); // B < R (原本的红色现在在B位置)
    REQUIRE(pixel[1] == 128); // B < R (原本的红色现在在B位置)
    REQUIRE(pixel[2] == 255); // B < R (原本的红色现在在B位置)
}

// 测试Indexed8格式（灰度图）
TEST_CASE("QImage2cvMatTestIndexed8", "[QImage2cvMatTest]") {
    QImage image(80, 100, QImage::Format_Indexed8);
    
    // 创建灰度调色板
    QVector<QRgb> colorTable;
    for (int i = 0; i < 256; ++i) {
        colorTable.append(qRgb(i, i, i));
    }
    image.setColorTable(colorTable);
    image.fill(128); // 填充中等灰度
    
    cv::Mat mat = QImage2cvMat(image);
    
    REQUIRE(mat.rows == 100);
    REQUIRE(mat.cols == 80);
    REQUIRE(mat.type() == CV_8UC1);
    REQUIRE(mat.channels() == 1);
    REQUIRE(mat.empty() == 0);
}