#include "inference_GPU.h"
#include <regex>

std::mutex YOLO_V11_GPU::mutex;  // 保护实例创建和销毁的互斥锁

#define benchmark
#define min(a,b)            (((a) < (b)) ? (a) : (b))
YOLO_V11_GPU::YOLO_V11_GPU() {

}


YOLO_V11_GPU::~YOLO_V11_GPU() {
    delete session;
}

#ifdef USE_CUDA
namespace Ort
{
    template<>
    struct TypeToTensorType<half> { static constexpr ONNXTensorElementDataType type = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16; };
}
#endif


template<typename T>
char* BlobFromImage(cv::Mat& iImg, T& iBlob) {
    int channels = iImg.channels();
    int imgHeight = iImg.rows;
    int imgWidth = iImg.cols;

    for (int c = 0; c < channels; c++)
    {
        for (int h = 0; h < imgHeight; h++)
        {
            for (int w = 0; w < imgWidth; w++)
            {
                iBlob[c * imgWidth * imgHeight + h * imgWidth + w] = typename std::remove_pointer<T>::type(
                    (iImg.at<cv::Vec3b>(h, w)[c]) / 255.0f);
            }
        }
    }
    return RET_OK;
}


char* YOLO_V11_GPU::PreProcess(cv::Mat& iImg, std::vector<int> iImgSize, cv::Mat& oImg)
{
    // 检查输入图像的通道数
    if (iImg.channels() == 3)
    {
        // 如果是三通道图像（BGR格式），则克隆图像并将其转换为RGB格式
        oImg = iImg.clone();
        cv::cvtColor(oImg, oImg, cv::COLOR_BGR2RGB);
    }
    else
    {
        // 如果是单通道图像（灰度图），则将其转换为RGB格式
        cv::cvtColor(iImg, oImg, cv::COLOR_GRAY2RGB);
    }

    // 根据模型类型进行不同的预处理
    switch (modelType)
    {
    case YOLO_DETECT_V8:
    case YOLO_POSE:
    case YOLO_DETECT_V8_HALF:
    case YOLO_POSE_V8_HALF://LetterBox
    {
        // LetterBox处理：保持宽高比，填充至目标尺寸
        if (iImg.cols >= iImg.rows)
        {
            // 计算缩放比例
            resizeScales = iImg.cols / (float)iImgSize.at(0);
            // 调整图像尺寸
            cv::resize(oImg, oImg, cv::Size(iImgSize.at(0), int(iImg.rows / resizeScales)));
        }
        else
        {
            // 计算缩放比例
            resizeScales = iImg.rows / (float)iImgSize.at(0);
            // 调整图像尺寸
            cv::resize(oImg, oImg, cv::Size(int(iImg.cols / resizeScales), iImgSize.at(1)));
        }
        // 创建一个目标尺寸的空白图像
        cv::Mat tempImg = cv::Mat::zeros(iImgSize.at(0), iImgSize.at(1), CV_8UC3);
        // 将调整后的图像复制到空白图像的中心区域
        oImg.copyTo(tempImg(cv::Rect(0, 0, oImg.cols, oImg.rows)));
        // 更新输出图像
        oImg = tempImg;
        break;
    }
    case YOLO_CLS://CenterCrop
    {
        // CenterCrop处理：裁剪中心区域并调整至目标尺寸
        int h = iImg.rows;
        int w = iImg.cols;
        int m = min(h, w);
        int top = (h - m) / 2;
        int left = (w - m) / 2;
        // 裁剪中心区域并调整尺寸
        cv::resize(oImg(cv::Rect(left, top, m, m)), oImg, cv::Size(iImgSize.at(0), iImgSize.at(1)));
        break;
    }
    }
    // 返回处理成功标志
    return RET_OK;
}


char* YOLO_V11_GPU::CreateSession(DL_INIT_PARAM& iParams) {
    // 初始化返回值为RET_OK，表示成功
    char* Ret = RET_OK;
    // 定义一个正则表达式模式，用于匹配中文字符
    std::regex pattern("[\u4e00-\u9fa5]");
    // 使用std::lock_guard来保证线程安全
    std::lock_guard<std::mutex> lock(mutex);
    // 检查模型路径中是否包含中文字符
    bool result = std::regex_search(iParams.modelPath, pattern);
    if (result)
    {
        // 如果包含中文字符，设置错误信息并返回
        Ret = "[YOLO_V11_GPU]:Your model path is error.Change your model path without chinese characters.";
        std::cout << Ret << std::endl;
        return Ret;
    }
    try
    {
        // 从输入参数中获取并设置相关参数
        rectConfidenceThreshold = iParams.rectConfidenceThreshold;
        iouThreshold = iParams.iouThreshold;
        imgSize = iParams.imgSize;
        modelType = iParams.modelType;
        cudaEnable = iParams.cudaEnable;
        // 创建ONNX运行环境
        env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Yolo");
        Ort::SessionOptions sessionOption;
        // 如果启用CUDA，设置CUDA执行提供程序
        if (this->cudaEnable)
        {
            OrtCUDAProviderOptions cudaOption;
            cudaOption.device_id = 0;
            sessionOption.AppendExecutionProvider_CUDA(cudaOption);
        }
        // 设置图优化级别和线程数
        sessionOption.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sessionOption.SetIntraOpNumThreads(iParams.intraOpNumThreads);
        sessionOption.SetLogSeverityLevel(iParams.logSeverityLevel);

#ifdef _WIN32
        // 在Windows平台上，将UTF-8字符串转换为宽字符字符串
        int ModelPathSize = MultiByteToWideChar(CP_UTF8, 0, iParams.modelPath.c_str(), static_cast<int>(iParams.modelPath.length()), nullptr, 0);
        wchar_t* wide_cstr = new wchar_t[ModelPathSize + 1];
        MultiByteToWideChar(CP_UTF8, 0, iParams.modelPath.c_str(), static_cast<int>(iParams.modelPath.length()), wide_cstr, ModelPathSize);
        wide_cstr[ModelPathSize] = L'\0';
        const wchar_t* modelPath = wide_cstr;
#else
        // 在非Windows平台上，直接使用模型路径
        const char* modelPath = iParams.modelPath.c_str();
#endif // _WIN32

        // 创建ONNX会话
        session = new Ort::Session(env, modelPath, sessionOption);
        Ort::AllocatorWithDefaultOptions allocator;
        // 获取输入节点名称
        size_t inputNodesNum = session->GetInputCount();
        for (size_t i = 0; i < inputNodesNum; i++)
        {
            Ort::AllocatedStringPtr input_node_name = session->GetInputNameAllocated(i, allocator);
            char* temp_buf = new char[50];
            strcpy(temp_buf, input_node_name.get());
            inputNodeNames.push_back(temp_buf);
        }
        // 获取输出节点名称
        size_t OutputNodesNum = session->GetOutputCount();
        for (size_t i = 0; i < OutputNodesNum; i++)
        {
            Ort::AllocatedStringPtr output_node_name = session->GetOutputNameAllocated(i, allocator);
            char* temp_buf = new char[10];
            strcpy(temp_buf, output_node_name.get());
            outputNodeNames.push_back(temp_buf);
        }
        // 设置运行选项
        options = Ort::RunOptions{ nullptr };
        // 预热会话
        WarmUpSession();
        return RET_OK;
    }
    catch (const std::exception& e)
    {
        // 捕获并处理异常
        const char* str1 = "[YOLO_V11_GPU]:";
        const char* str2 = e.what();
        std::string result = std::string(str1) + std::string(str2);
        char* merged = new char[result.length() + 1];
        std::strcpy(merged, result.c_str());
        std::cout << merged << std::endl;
        delete[] merged;
        return "[YOLO_V11_GPU]:Create session failed.";
    }

}


char* YOLO_V11_GPU::RunSession(cv::Mat& iImg, std::vector<DL_RESULT>& oResult) {
#ifdef benchmark
    clock_t starttime_1 = clock();
#endif // benchmark

    char* Ret = RET_OK;
    cv::Mat processedImg;
    PreProcess(iImg, imgSize, processedImg);
    if (modelType < 4)
    {
        float* blob = new float[processedImg.total() * 3];
        BlobFromImage(processedImg, blob);
        std::vector<int64_t> inputNodeDims = { 1, 3, imgSize.at(0), imgSize.at(1) };
        TensorProcess(starttime_1, iImg, blob, inputNodeDims, oResult);
    }
    else
    {
#ifdef USE_CUDA
        half* blob = new half[processedImg.total() * 3];
        BlobFromImage(processedImg, blob);
        std::vector<int64_t> inputNodeDims = { 1,3,imgSize.at(0),imgSize.at(1) };
        TensorProcess(starttime_1, iImg, blob, inputNodeDims, oResult);
#endif
    }

    return Ret;
}


template<typename N>
char* YOLO_V11_GPU::TensorProcess(clock_t& starttime_1, cv::Mat& iImg, N& blob, std::vector<int64_t>& inputNodeDims,
    std::vector<DL_RESULT>& oResult) {
    // 创建输入张量，使用CPU内存
    Ort::Value inputTensor = Ort::Value::CreateTensor<typename std::remove_pointer<N>::type>(
        Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU), blob, 3 * imgSize.at(0) * imgSize.at(1),
        inputNodeDims.data(), inputNodeDims.size());
#ifdef benchmark
    // 记录预处理结束时间
    clock_t starttime_2 = clock();
#endif // benchmark
    // 运行模型推理
    auto outputTensor = session->Run(options, inputNodeNames.data(), &inputTensor, 1, outputNodeNames.data(),
        outputNodeNames.size());
#ifdef benchmark
    // 记录推理结束时间
    clock_t starttime_3 = clock();
#endif // benchmark

    // 获取输出张量的类型信息
    Ort::TypeInfo typeInfo = outputTensor.front().GetTypeInfo();
    auto tensor_info = typeInfo.GetTensorTypeAndShapeInfo();
    std::vector<int64_t> outputNodeDims = tensor_info.GetShape();
    // 获取输出张量的可变数据指针
    auto output = outputTensor.front().GetTensorMutableData<typename std::remove_pointer<N>::type>();
    // 释放输入数据内存
    delete[] blob;
    // 根据模型类型处理输出结果
    switch (modelType)
    {
    case YOLO_DETECT_V8:
    case YOLO_DETECT_V8_HALF:
    {
        // 获取输出结果的维度信息
        int signalResultNum = outputNodeDims[1];//84
        int strideNum = outputNodeDims[2];//8400
        std::vector<int> class_ids;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        cv::Mat rawData;
        if (modelType == YOLO_DETECT_V8)
        {
            // FP32
            rawData = cv::Mat(signalResultNum, strideNum, CV_32F, output);
        }
        else
        {
            // FP16
            rawData = cv::Mat(signalResultNum, strideNum, CV_16F, output);
            rawData.convertTo(rawData, CV_32F);
        }
        // Note:
        // ultralytics add transpose operator to the output of yolov8 model.which make yolov8/v5/v7 has same shape
        // https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.pt
        rawData = rawData.t();

        float* data = (float*)rawData.data;

        for (int i = 0; i < strideNum; ++i)
        {
            float* classesScores = data + 4;
            cv::Mat scores(1, this->classes.size(), CV_32FC1, classesScores);
            cv::Point class_id;
            double maxClassScore;
            cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
            if (maxClassScore > rectConfidenceThreshold)
            {
                confidences.push_back(maxClassScore);
                class_ids.push_back(class_id.x);
                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];

                int left = int((x - 0.5 * w) * resizeScales);
                int top = int((y - 0.5 * h) * resizeScales);

                int width = int(w * resizeScales);
                int height = int(h * resizeScales);

                boxes.push_back(cv::Rect(left, top, width, height));
            }
            data += signalResultNum;
        }
        std::vector<int> nmsResult;
        cv::dnn::NMSBoxes(boxes, confidences, rectConfidenceThreshold, iouThreshold, nmsResult);
        for (int i = 0; i < nmsResult.size(); ++i)
        {
            int idx = nmsResult[i];
            DL_RESULT result;
            result.classId = class_ids[idx];
            result.confidence = confidences[idx];
            result.box = boxes[idx];
            oResult.push_back(result);
        }

#ifdef benchmark
        clock_t starttime_4 = clock();
        double pre_process_time = (double)(starttime_2 - starttime_1) / CLOCKS_PER_SEC * 1000;
        double process_time = (double)(starttime_3 - starttime_2) / CLOCKS_PER_SEC * 1000;
        double post_process_time = (double)(starttime_4 - starttime_3) / CLOCKS_PER_SEC * 1000;
        if (cudaEnable)
        {
            std::cout << "[YOLO_V11_GPU(CUDA)]: " << pre_process_time << "ms pre-process, " << process_time << "ms inference, " << post_process_time << "ms post-process." << std::endl;
        }
        else
        {
            std::cout << "[YOLO_V11_GPU(CPU)]: " << pre_process_time << "ms pre-process, " << process_time << "ms inference, " << post_process_time << "ms post-process." << std::endl;
        }
#endif // benchmark

        break;
    }
    case YOLO_CLS:
    case YOLO_CLS_HALF:
    {
        cv::Mat rawData;
        if (modelType == YOLO_CLS) {
            // FP32
            rawData = cv::Mat(1, this->classes.size(), CV_32F, output);
        } else {
            // FP16
            rawData = cv::Mat(1, this->classes.size(), CV_16F, output);
            rawData.convertTo(rawData, CV_32F);
        }
        float *data = (float *) rawData.data;

        DL_RESULT result;
        for (int i = 0; i < this->classes.size(); i++)
        {
            result.classId = i;
            result.confidence = data[i];
            oResult.push_back(result);
        }
        break;
    }
    default:
        std::cout << "[YOLO_V11_GPU]: " << "Not support model type." << std::endl;
    }
    return RET_OK;

}


char* YOLO_V11_GPU::WarmUpSession() {
    // 记录开始时间
    clock_t starttime_1 = clock();
    // 创建一个与图像大小匹配的3通道图像
    cv::Mat iImg = cv::Mat(cv::Size(imgSize.at(0), imgSize.at(1)), CV_8UC3);
    // 用于存储预处理后的图像
    cv::Mat processedImg;
    // 对图像进行预处理
    PreProcess(iImg, imgSize, processedImg);
    // 根据模型类型选择不同的处理方式
    if (modelType < 4)
    {
        // 创建一个用于存储图像数据的浮点数组
        float* blob = new float[iImg.total() * 3];
        // 将图像数据转换为blob格式
        BlobFromImage(processedImg, blob);
        // 定义输入张量的维度
        std::vector<int64_t> YOLO_input_node_dims = { 1, 3, imgSize.at(0), imgSize.at(1) };
        // 创建输入张量
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU), blob, 3 * imgSize.at(0) * imgSize.at(1),
            YOLO_input_node_dims.data(), YOLO_input_node_dims.size());
        // 运行模型并获取输出张量
        auto output_tensors = session->Run(options, inputNodeNames.data(), &input_tensor, 1, outputNodeNames.data(),
            outputNodeNames.size());
        // 释放blob数组内存
        delete[] blob;
        // 记录结束时间
        clock_t starttime_4 = clock();
        // 计算处理时间（毫秒）
        double post_process_time = (double)(starttime_4 - starttime_1) / CLOCKS_PER_SEC * 1000;
        // 如果启用了CUDA，输出CUDA预热时间
        if (cudaEnable)
        {
            std::cout << "[YOLO_V11_GPU(CUDA)]: " << "Cuda warm-up cost " << post_process_time << " ms. " << std::endl;
        }
    }
    else
    {
#ifdef USE_CUDA
        // 创建一个用于存储图像数据的半精度浮点数组
        half* blob = new half[iImg.total() * 3];
        // 将图像数据转换为blob格式
        BlobFromImage(processedImg, blob);
        // 定义输入张量的维度
        std::vector<int64_t> YOLO_input_node_dims = { 1,3,imgSize.at(0),imgSize.at(1) };
        // 创建输入张量
        Ort::Value input_tensor = Ort::Value::CreateTensor<half>(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU), blob, 3 * imgSize.at(0) * imgSize.at(1), YOLO_input_node_dims.data(), YOLO_input_node_dims.size());
        // 运行模型并获取输出张量
        auto output_tensors = session->Run(options, inputNodeNames.data(), &input_tensor, 1, outputNodeNames.data(), outputNodeNames.size());
        // 释放blob数组内存
        delete[] blob;
        // 记录结束时间
        clock_t starttime_4 = clock();
        // 计算处理时间（毫秒）
        double post_process_time = (double)(starttime_4 - starttime_1) / CLOCKS_PER_SEC * 1000;
        // 如果启用了CUDA，输出CUDA预热时间
        if (cudaEnable)
        {
            std::cout << "[YOLO_V11_GPU(CUDA)]: " << "Cuda warm-up cost " << post_process_time << " ms. " << std::endl;
        }
#endif
    }
    // 返回成功标志
    return RET_OK;
}
