#ifndef INFERENCE_GPU_H
#define INFERENCE_GPU_H

#include "model.h"

#include <string>
#include <vector>
#include <cstdio>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "onnxruntime_cxx_api.h"


#define USE_CUDA

#ifdef USE_CUDA
#include <cuda_fp16.h>
#endif

class YOLO_V11_GPU : public Model
{
public:
    // 构造函数
    YOLO_V11_GPU();

    // 析构函数
    ~YOLO_V11_GPU();

public:
    // 创建会话，传入初始化参数，返回会话创建状态
    char* CreateSession(DL_INIT_PARAM& iParams);

    // 运行会话，传入图像和结果容器，返回会话运行状态
    char* RunSession(cv::Mat& iImg, std::vector<DL_RESULT>& oResult);

    // 预热会话，返回预热状态
    char* WarmUpSession();

    // 模板函数，用于处理张量，传入开始时间、图像、张量、输入节点维度和结果容器，返回处理状态
    template<typename N>
    char* TensorProcess(clock_t& starttime_1, cv::Mat& iImg, N& blob, std::vector<int64_t>& inputNodeDims,
        std::vector<DL_RESULT>& oResult);

    // 预处理函数，传入图像、图像尺寸和输出图像，返回预处理状态
    char* PreProcess(cv::Mat& iImg, std::vector<int> iImgSize, cv::Mat& oImg);

private:
    // ONNX运行时环境
    Ort::Env env;
    // ONNX会话指针
    Ort::Session* session;
    // 是否启用CUDA
    bool cudaEnable;
    // 运行选项
    Ort::RunOptions options;
    // 输入节点名称列表
    std::vector<const char*> inputNodeNames;
    // 输出节点名称列表
    std::vector<const char*> outputNodeNames;

    // 模型类型
    MODEL_TYPE modelType;
    // 图像尺寸
    std::vector<int> imgSize;
    // 矩形置信度阈值
    float rectConfidenceThreshold;
    // IOU阈值
    float iouThreshold;
    float resizeScales;//letterbox scale

    static std::mutex mutex;
};

#endif //INFERENCE_H

