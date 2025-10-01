#ifndef IMAGE_FORMATE_CHANGE_H
#define IMAGE_FORMATE_CHANGE_H

#include <QImage>
#include <opencv2/opencv.hpp>
extern "C" {
#include <libavutil/frame.h> // AVFrame
#include <libswscale/swscale.h> // SwsContext
#include <libavutil/pixfmt.h> // AVPixelFormat
}

inline QImage avframeToQImage(AVFrame *sw_frame) 
{
    SwsContext* sws_ctx_Qimage {nullptr}; // 图像转换上下文

    // 检查输入格式是否有效
    if (sw_frame->format != AV_PIX_FMT_YUV420P && sw_frame->format != AV_PIX_FMT_NV12) {
        qWarning("Unsupported pixel format");
        return QImage();
    }

    // 初始化转换上下文
    if (!sws_ctx_Qimage) {
        // 首次初始化
        sws_ctx_Qimage = sws_getContext(
            sw_frame->width, sw_frame->height, (AVPixelFormat)sw_frame->format,
            1280, 720, AV_PIX_FMT_RGB32,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    if (!sws_ctx_Qimage) {
        qWarning("Failed to create SwsContext");
        return QImage();
    }

    // 分配 RGB 缓冲区
    uint8_t* rgb_data = new uint8_t[1280 * 720 * 4];
    uint8_t* dst_data[1] = { rgb_data };
    int dst_linesize[1] = { 1280 * 4 };

    // 执行转换
    sws_scale(
        sws_ctx_Qimage,
        sw_frame->data, sw_frame->linesize,
        0, sw_frame->height,
        dst_data, dst_linesize
    );

    // 创建 QImage
    QImage image(
        rgb_data,
        1280, 
        720,
        QImage::Format_RGB32
    );

    // 深拷贝以避免内存问题
    QImage image_copy = image.copy();

    // 释放资源
    delete[] rgb_data;

    return image_copy;
}

inline cv::Mat avframeToCvMat(AVFrame *sw_frame)
{
    SwsContext* sws_ctx_CVMat {nullptr}; // 图像转换上下文

   // 检查输入格式
    if (sw_frame->format != AV_PIX_FMT_YUV420P && 
        sw_frame->format != AV_PIX_FMT_NV12) {
        throw std::runtime_error("Unsupported pixel format");
    }

    // 初始化转换上下文（YUV420P/NV12 → BGR24）
    if (!sws_ctx_CVMat) {
            sws_ctx_CVMat = sws_getContext(
            sw_frame->width, sw_frame->height, (AVPixelFormat)sw_frame->format,
            sw_frame->width, sw_frame->height, AV_PIX_FMT_BGR24,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    // 分配目标 AVFrame
    AVFrame* bgr_frame = av_frame_alloc();
    bgr_frame->format = AV_PIX_FMT_BGR24;
    bgr_frame->width = sw_frame->width;
    bgr_frame->height = sw_frame->height;
    av_frame_get_buffer(bgr_frame, 0);

    // 执行转换
    sws_scale(
        sws_ctx_CVMat,
        sw_frame->data, sw_frame->linesize,
        0, sw_frame->height,
        bgr_frame->data, bgr_frame->linesize
    );

    // 创建 cv::Mat（深拷贝）
    cv::Mat image(
        sw_frame->height,
        sw_frame->width,
        CV_8UC3,
        bgr_frame->data[0],
        bgr_frame->linesize[0]
    );
    cv::Mat image_copy = image.clone();

    // 释放资源
    av_frame_free(&bgr_frame);

    return image_copy;
}

inline cv::Mat QImage2cvMat(const QImage& image) {
    cv::Mat mat;
    switch (image.format()) {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
            mat = cv::Mat(image.height(), image.width(), CV_8UC4, 
                          const_cast<uchar*>(image.constBits()), image.bytesPerLine());
            break;
        case QImage::Format_RGB888:
            mat = cv::Mat(image.height(), image.width(), CV_8UC3, 
                          const_cast<uchar*>(image.constBits()), image.bytesPerLine());
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR); // 交换 B/R 通道
            break;
        case QImage::Format_Indexed8:
            mat = cv::Mat(image.height(), image.width(), CV_8UC1, 
                          const_cast<uchar*>(image.constBits()), image.bytesPerLine());
            break;
        default:
            throw std::invalid_argument("Unsupported QImage format");
    }
    return mat;
}

inline QImage Mat2QImage(const cv::Mat& mat) {
    switch (mat.type()) {
        case CV_8UC1: {
            return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        }
        case CV_8UC3: {
            QImage image(mat.data, mat.cols, mat.rows, mat.step,QImage::Format_RGB888);
            // 
            return image.rgbSwapped(); // BGR → RGB
        }
        case CV_8UC4: {
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return image.copy();
        }
        default:
            throw std::invalid_argument("Unsupported cv::Mat type");
    }
}

#endif // IMAGE_FORMATE_CHANGE_H