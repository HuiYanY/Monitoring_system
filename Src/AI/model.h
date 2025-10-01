#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#define    RET_OK nullptr

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0A00 // Windows 10
#define _WIN32_WINNT 0x0A00 // Windows 10
#include <windows.h>
#include <Windows.h>
#include <direct.h>
#include <io.h>
#endif

// 定义一个枚举类型MODEL_TYPE，用于表示不同的模型类型
enum MODEL_TYPE
{
    //FLOAT32 MODEL
    YOLO_DETECT_V8 = 1,
    YOLO_POSE = 2,
    YOLO_CLS = 3,

    //FLOAT16 MODEL
    YOLO_DETECT_V8_HALF = 4,
    YOLO_POSE_V8_HALF = 5,
    YOLO_CLS_HALF = 6
};


typedef struct _DL_INIT_PARAM
{
    std::string modelPath;
    MODEL_TYPE modelType = YOLO_DETECT_V8;
    std::vector<int> imgSize = { 640, 640 };
    float rectConfidenceThreshold = 0.6;
    float iouThreshold = 0.5;
    int	keyPointsNum = 2;//Note:kpt number for pose
    bool cudaEnable = false;
    int logSeverityLevel = 3;
    int intraOpNumThreads = 4;
} DL_INIT_PARAM;


typedef struct _DL_RESULT
{
    int classId;
    float confidence;
    cv::Rect box;
    std::vector<cv::Point2f> keyPoints;
} DL_RESULT;

class Model{
public:
    // 创建会话，传入初始化参数，返回会话创建状态
    virtual char* CreateSession(DL_INIT_PARAM& iParams) = 0;

    // 运行会话，传入图像和结果容器，返回会话运行状态
    virtual char* RunSession(cv::Mat& iImg, std::vector<DL_RESULT>& oResult) = 0;

    // 类别列表
    std::vector<std::string> classes{};

};

#endif // MODEL_H