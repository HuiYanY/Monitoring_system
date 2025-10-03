#ifndef CAMERA_H
#define CAMERA_H

#include <QMutex>
#include <QImage>
#include <QDebug>
#include <QThread>
#include <QDate>
#include <QFile>
#include <QSettings>
#include <QTimer>

#include "ShareQueue.h"
#include "ImageFormateChange.h"
#include "AI.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/hwcontext.h> // 硬件加速
}

class Camera : public QThread
{
    Q_OBJECT
private:
    /* data */
    QString cameraName {""}; // 摄像头名称
    int cameraID {-1}; // 摄像头ID
    int screenID {-1}; // 屏幕ID

    static QMutex mutex; // 用于保护共享资源的互斥锁

    // 解码所需的变量
    AVFormatContext *_avformatContext {nullptr}; // 存储输入视频的格式信息
    AVBufferRef *hw_device_ctx = nullptr; // 硬件设备上下文
    AVFrame *hw_frame = av_frame_alloc(); // 分配硬件帧
    AVFrame *_avframe {nullptr}; // 存储解码后的数据
    AVCodecParameters* _avCodecParameter {nullptr}; // 解码器参数
    AVCodecContext* _avCodeContext {nullptr}; // 解码器上下文
    AVPacket* _aVPacket {nullptr}; // 存储读取到的数据包

    QString url; // 视频流URL
    int videoStreamIndex {-1}; // 视频流索引

    // AI识别
    // 创建Ai对象
    AI* ai {nullptr};

    bool isOnAI {false};
    QString path = "";
    // std::vector<std::string> classes{"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};
    std::vector<std::string> classes{"person"};
    Model* yoloDetector {nullptr};
    // 初始化深度学习参数结构体
    DL_INIT_PARAM params;
    bool runOnGPU {false};
    std::vector<DL_RESULT> res; // 存储识别结果

    std::atomic<bool> isStop {false}; // 原子操作替代bool
    bool isShow {false};
    bool isEncode {false};

    int frameCount {1}; // 每隔多少帧进行一次AI识别

    // FPS
    QTimer *timer {nullptr};
    int FPS {0};
    int FPS_Show {0};
    bool isShowFPS {false}; // 是否显示FPS

public:
    SharedQueue<QImage> frameQueue; // 共享队列
    SharedQueue<cv::Mat> PredictedQueue; // 共享队列

public:
    explicit Camera(QObject *parent = nullptr);
    explicit Camera(const QString url_, QObject *parent = nullptr);
    explicit Camera(const int id, QObject *parent = nullptr);
    explicit Camera(const int id, const QString url_, const QString cameraName, QObject *parent = nullptr);
    // explicit Camera(const int id, const QString url_, QObject *parent, QSlider *slider, QLabel *label); // 带进度条
    ~Camera();

    bool Init_Camera();
    void run() override;

    void readCamera();
    void addCamera(QString cameraName, QString url);

    // getters
    void SetURL(QString str){this->url = str;}
    void SetStop(bool stop){mutex.lock();this->isStop = stop;mutex.unlock();}
    void SetIsOnAI(bool onAI){this->isOnAI = onAI;}
    void SetAi(const QString &path,bool runOnGPU);

    // setters
    QString getCameraName() const {return this->cameraName;}
    QString getURL() const {return this->url;}
    bool getIsStop() const {return this->isStop;}
    bool getIsShow() const {return this->isShow;}

private:
    QImage Ai(cv::Mat image); // AI识别

    void readConfig(); // 读取配置文件
    void writeConfig(QString cameraName, QString url); // 写入配置文件

public slots:
    void onStopCameraThread();
    void onStopCameraThread(const int index); // 重载
    void setShowScreenIndex(int screenID, int screenIndex); 
    void updateFrameRate();
    void showStop(int page); // 显示停止
    void showStopOne(int CameraID); // 显示停止
};

#endif