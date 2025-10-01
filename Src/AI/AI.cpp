#include "Ai.h"

QMutex AI::mutex; // 创建一个全局的互斥锁

AI::AI(SharedQueue<cv::Mat> &frameQueue,bool runOnGPU)
    :frameQueue(frameQueue)
{
    // 设置矩形置信度阈值
    this->params.rectConfidenceThreshold = 0.5;
    // 设置IOU阈值
    this->params.iouThreshold = 0.5;
    // 设置模型路径
    this->params.modelPath = "./models/yolo11n.onnx";
    // 设置图像尺寸
    this->params.imgSize = { 640, 640 };
    if(runOnGPU){
        // 如果定义了USE_CUDA，则启用CUDA
        this->params.cudaEnable = true;
        // GPU FP32 inference
        this->params.modelType = YOLO_DETECT_V8;
        this->yoloDetector = std::move(std::make_unique<YOLO_V11_GPU>()); // 创建YOLO检测器
        this->yoloDetector->classes = this->classes; // 设置类别名称
        this->yoloDetector->CreateSession(params); // 创建YOLO检测会话
    }
    else
    {
        // 设置模型类型
        this->params.modelType = YOLO_DETECT_V8;
        this->params.cudaEnable = false; // 禁用CUDA
        this->yoloDetector = std::move(std::make_unique<YOLO_V11>()); // 创建YOLO检测器
        this->yoloDetector->classes = this->classes; // 设置类别名称
        this->yoloDetector->CreateSession(params); // 创建YOLO检测会话
    }
}

AI::~AI()
{
}

void AI::run()
{
    cv::Mat frame;
    while (this->isRunning)
    {
        frame = this->frameQueue.pop();
        frameCount++;
        // 每5帧进行一次图像识别
        if (frameCount > 5){
            frameCount = 0;
                // 图像识别
                this->res.clear();
                // 对GPU加锁
                this->mutex.lock();
                this->yoloDetector->RunSession(frame, this->res);
                this->mutex.unlock();
                // 解锁GPU
        }
        
        // 遍历检测结果
        for (auto& re : res)
        {    
            cv::Scalar color(0, 255, 0); // 设置颜色为绿色

            // 在图像上绘制检测框
            cv::rectangle(frame, re.box, color, 3);

            // 计算置信度并格式化输出
            float confidence = floor(100 * re.confidence) / 100;
            std::cout << std::fixed << std::setprecision(2);
            std::string label = this->yoloDetector->classes[re.classId] + " " +
                std::to_string(confidence).substr(0, std::to_string(confidence).size() - 4);

            // 在检测框上方绘制标签背景
            cv::rectangle(
                frame,
                cv::Point(re.box.x, re.box.y - 25),
                cv::Point(re.box.x + label.length() * 15, re.box.y),
                color,
                cv::FILLED
            );

            // 在标签背景上绘制标签文本
            cv::putText(
                frame,
                label,
                cv::Point(re.box.x, re.box.y - 5),
                cv::FONT_HERSHEY_SIMPLEX,
                0.75,
                cv::Scalar(0, 0, 0),
                2
            );
        }
        // Inference ends here
        // return Mat2QImage(frame);
        
        this->outQueue.push(Mat2QImage(frame)); // 将结果推入输出队列
    }
}
