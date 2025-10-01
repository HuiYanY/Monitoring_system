# ifndef AI_H
# define AI_H

#include <QThread>
#include <QSharedMemory>
#include <QMutex>
#include <QImage>

#include <memory> // std::unique_ptr

#include "model.h"
#include "inference.h"
#include "inference_GPU.h"
#include "ShareQueue.h"
#include "ImageFormateChange.h"

class AI : public QThread
{
    Q_OBJECT
// 变量
private:
    unsigned int frameCount {0}; // 采样帧数间隔
    std::vector<std::string> classes{"person"};
    std::unique_ptr<Model> yoloDetector {nullptr};
    // 初始化深度学习参数结构体
    DL_INIT_PARAM params;
    bool runOnGPU {false};
    std::vector<DL_RESULT> res; // 存储识别结果

    //线程锁
    static QMutex mutex;

    bool isRunning {true};

    // 共享内存 数据帧
    SharedQueue<cv::Mat> &frameQueue;

public:
    AI(SharedQueue<cv::Mat> &frameQueue,bool runOnGPU);
    // 禁止拷贝构造函数
    AI(const AI &other) = delete;
    // 重载赋值运算符，并删除该运算符
    AI &operator=(const AI &other) = delete;
    ~AI();

    // Getters
    SharedQueue<QImage> outQueue; // 共享内存 数据帧
private:
    void run() override;
};

# endif // !AI_H