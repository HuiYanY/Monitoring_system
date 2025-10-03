#include "Camera.h"
#include <QPainter>

QMutex Camera::mutex; // 全局互斥锁

static AVPixelFormat get_hw_format(AVCodecContext *ctx, const AVPixelFormat *formats) {
    for (const AVPixelFormat *p = formats; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == AV_PIX_FMT_DXVA2_VLD)
            return *p;
    }
    return AV_PIX_FMT_NONE;
}

Camera::Camera(QObject *parent)
{
}

Camera::Camera(const QString url_, QObject *parent)
    : url(url_), QThread(parent)
{  
}

Camera::Camera(const int id, QObject *parent)
    : cameraID(id), QThread(parent)
{
}

Camera::Camera(const int id, const QString url_, const QString cameraName_, QObject *parent)
    : cameraID(id), url(url_),cameraName(cameraName_), QThread(parent)
{
    this->frameQueue.setMaxSize(26); // 设置最大帧数为26帧
}

Camera::~Camera()
{
    avformat_close_input(&this->_avformatContext);  // 先关闭输入流
    avformat_free_context(this->_avformatContext);  // 再释放上下文
    av_frame_free(&(this->_avframe));
    av_packet_free(&(this->_aVPacket));
    av_buffer_unref(&hw_device_ctx);
    avformat_network_deinit();
    delete this->yoloDetector; // 释放yoloDetector指针
}

bool Camera::Init_Camera()
{
    this->videoStreamIndex = -1;
    this->_avframe = av_frame_alloc();
    this->_aVPacket = av_packet_alloc();

    // 设置超时选项
    AVDictionary *options = NULL;
    av_dict_set(&options, "timeout", "5000", 0); // 设置超时时间为 5 秒
    qDebug() << this->url;
    //打开视频流
    if(avformat_open_input(&(this->_avformatContext),this->url.toStdString().c_str(),NULL,&options) < 0){
        qDebug() << "open video stream failed!";
        return false;
    }
    av_dict_free(&(options));

    //获取视频信息
    if(avformat_find_stream_info(this->_avformatContext,NULL) < 0){
        qDebug() << "Failed to get video information!";
        return false;
    }

    // av_dump_format(this->_avformatContext,0,this->url.toStdString().c_str(),0); //打印视频信息

    //获取视频流Index
    for(int i = 0; i < this->_avformatContext->nb_streams; i++){
        //如果是视频流
        if (this->_avformatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            this->videoStreamIndex = i;
            break;
        }
    }

    if (this->videoStreamIndex == -1)
    {
        qDebug() << "Video acquisition failed!";
        return false;
    }

    //获取视频流的分辨率大小
    this->_avCodecParameter = this->_avformatContext->streams[this->videoStreamIndex]->codecpar;

    // 获取硬件设备上下文
    av_hwdevice_ctx_create(&this->hw_device_ctx, AV_HWDEVICE_TYPE_DXVA2, nullptr, nullptr, 0);
    if (!hw_device_ctx) {
        // 处理错误
        return false;
    }

    // 获取解码器
    const AVCodec *avcode;
    avcode = avcodec_find_decoder(this->_avCodecParameter->codec_id); //查找解码器
    if (!avcode) {
        avcode = avcodec_find_decoder(this->_avCodecParameter->codec_id);
        qDebug() << "Using software decoder fallback";
        if (!avcode) {
            qDebug() << "Could not find codec!";
            return false; // 返回错误
        }
    }

    //打开解码器
    this->_avCodeContext = avcodec_alloc_context3(avcode);
    if (!this->_avCodeContext) {
        // 处理分配编解码器上下文失败的情况
        qDebug() << "Failed to assign codec context!";
        return false;
    }
    if (avcodec_parameters_to_context(this->_avCodeContext, this->_avCodecParameter) < 0) {
        // 处理参数复制失败的情况
        qDebug() << "Failed to process parameter replication!";
        return false;
    }

    this->_avCodeContext->hw_device_ctx  = av_buffer_ref(hw_device_ctx); // 设置硬件设备上下文
    this->_avCodeContext->get_format = get_hw_format; // 回调函数获取支持的像素格式

    if (avcodec_open2((this->_avCodeContext),avcode,nullptr) < 0)
    {
        qDebug() << "Decoder failed to open!";
        return false;
    }

    qDebug() << "Initialize the stream successfully!";
    return true;
}

void Camera::run()
{
    if(!Init_Camera()){
        qDebug() << "Init Camera failed!";
        return;        
    } // 初始化失败
    int ret = -1;
    // 设置队列最大存储数据量  帧
    this->frameQueue.setMaxSize(25);
    
    // 解封装
    // 解码
    // 颜色空间转换
    // 生成image
    
    while (((ret = av_read_frame(this->_avformatContext,this->_aVPacket)) >= 0))
    {
        if(this->isStop) break;
        if (this->_aVPacket->stream_index == this->videoStreamIndex){
            //将包发给解码器
            if ((ret = avcodec_send_packet(this->_avCodeContext,this->_aVPacket)) < 0)
            {
                qDebug() << "Packet——>Codec failed!";
                av_packet_unref(_aVPacket);  // 错误时也需要释放
                continue;
            }

            while (ret >= 0)
            {
                ret = avcodec_receive_frame(this->_avCodeContext,this->hw_frame);
                if (ret == AVERROR_EOF)
                {
                    this->isEncode = false;
                    break;
                }
                
                if (ret == AVERROR(EAGAIN))
                {
                    break;
                }
                
                if (av_hwframe_transfer_data(this->_avframe, this->hw_frame, 0) < 0) {
                    av_frame_unref(this->_avframe);
                    av_frame_unref(this->hw_frame);
                    continue;
                }

                //发送image进行显示
                if (this->isShow)
                if (this->isOnAI)
                {
                    // 进行图像识
                    PredictedQueue.push_skip(avframeToCvMat(this->_avframe)); // 将识别结果添加到队列中
                }
                else{
                    // 发送image进行显示
                    frameQueue.push_skip(avframeToQImage(this->_avframe)); // 将结果添加到队列中
                }
            } 
        }
        av_packet_unref((this->_aVPacket)); // 释放资源
    }
    this->isEncode = false;

}

void Camera::readCamera()
{
    this->readConfig();
}

void Camera::addCamera(QString cameraName, QString url)
{
    this->writeConfig(cameraName, url);
}

void Camera::SetAi(const QString &path,bool runOnGPU)
{
    this->path = path;
    this->runOnGPU = runOnGPU;

    this->ai = new AI((this->PredictedQueue), this->runOnGPU); // 创建AI对象

    if (this->isOnAI)
    {
        // 设置矩形置信度阈值
        this->params.rectConfidenceThreshold = 0.5;
        // 设置IOU阈值
        this->params.iouThreshold = 0.5;
        // 设置模型路径
        this->params.modelPath = this->path.toStdString();
        // 设置图像尺寸
        this->params.imgSize = { 640, 640 };
        if(runOnGPU){
            // 如果定义了USE_CUDA，则启用CUDA
            this->params.cudaEnable = true;
            // GPU FP32 inference
            this->params.modelType = YOLO_DETECT_V8;

            this->yoloDetector = new YOLO_V11_GPU; 
            this->yoloDetector->classes = this->classes; // 设置类别名称
            this->yoloDetector->CreateSession(params); // 创建YOLO检测会话
        }
        else
        {
            // 设置模型类型
            this->params.modelType = YOLO_DETECT_V8;
            this->params.cudaEnable = false; // 禁用CUDA

            this->yoloDetector = new YOLO_V11; 
            this->yoloDetector->classes = this->classes; // 设置类别名称
            this->yoloDetector->CreateSession(params); // 创建YOLO检测会话
        }
    }
}

// 图像识别
QImage Camera::Ai(cv::Mat frame)
{
    frameCount++;
    // 每5帧进行一次图像识别
    if (frameCount > 5){
        frameCount = 0;
        // 图像识别
        this->res.clear();
        // 对GPU加锁
        mutex.lock();
        this->yoloDetector->RunSession(frame, this->res);
        mutex.unlock();
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

    if (this->isShowFPS)
    {
        QString label = "FPS: " + QString::number(FPS_Show); // 计算帧率
        // 绘制FPS背景
        cv::rectangle(
            frame,
            cv::Point(10, 10),
            cv::Point(10 + label.length() * 15, 45),
            cv::Scalar(255,255,255),
            cv::FILLED
        );

        // 在FPS背景上绘制FPS
        cv::putText(
            frame,
            label.toStdString(),
            cv::Point(13 , 30),
            cv::FONT_HERSHEY_SIMPLEX,
            0.75,
            cv::Scalar(0, 0, 0),
            2
        );
    }
    return Mat2QImage(frame);
}

void Camera::readConfig()
{
    QSettings settings("./ini/cameraInfo.ini", QSettings::IniFormat);

    // 获取所有的分组
    QStringList groupList = settings.childGroups();
    // 读取用户的参数
    // 读取参数
    QString id = QString("%1").arg(this->cameraID, 3, 10,QChar('0'));
    this->cameraName = settings.value(id+"/cameraName").toString();
    this->url = settings.value(id+"/url").toString();
}

void Camera::writeConfig(QString cameraName, QString url)
{
    QSettings settings("./ini/cameraInfo.ini", QSettings::IniFormat);
    // Qt5 启用
    // settings.setIniCodec("utf-8"); // 防止中文分组或键名乱码

    QStringList groups = settings.childGroups();
    if (groups.isEmpty()) {
        QString id = "000";
        settings.setValue(id+"/cameraName", cameraName); // 自动创建"NewGroup"
        settings.setValue(id+"/url", url); // 自动创建"NewGroup"
        settings.setValue(id+"/outputFile", cameraName); // 自动创建"NewGroup"
    }
    else
    {
        int num = groups.last().toInt() + 1;
        QString id = QString("%1").arg(num, 3, 10,QChar('0'));
        settings.setValue(id+"/cameraName", cameraName); // 自动创建"NewGroup"
        settings.setValue(id+"/url", url); // 自动创建"NewGroup"
        settings.setValue(id+"/outputFile", cameraName); // 自动创建"NewGroup"
    }
}

void Camera::setShowScreenIndex(int screenID, int screenIndex)
{
    if (screenID == this->cameraID)
    {
        if (-1 == screenIndex)
        {
            this->isShow = false; // 设置当前不显示
            this->isEncode = false; // 设置当前不显示
        }
        else
        {
            this->isShow = true; // 设置当前显示
            this->isEncode = true; // 设置当前显示
        }
        this->screenID = screenIndex; // 设置当前显示的屏幕
    }
}

void Camera::updateFrameRate()
{
    this->FPS_Show = this->FPS;
    this->FPS = 0;
}

void Camera::showStop(int page)
{
    switch (page)
    {
        case 1:
            // 实时
            if (this->cameraID < 0)
            {
                this->isShow = false; // 设置当前不显示
            }
            else
            if (this->screenID >= 0)
            {
                this->isShow = true; // 设置当前显示
            }
            break;
        case 2:
            // 回放
            if (this->cameraID >= 0)
            {
                this->isShow = false; // 设置当前不显示
            }
            else
            {
                this->isShow = true; // 设置当前显示
            }
            break;
        case 3:    
            // 设置
            this->isShow = false; // 设置当前不显示
            break;
    }
}

void Camera::showStopOne(int CameraID)
{
    if (CameraID == this->cameraID)
    {
        this->isShow = false; // 设置当前不显示
    }
    
}

void Camera::onStopCameraThread()
{
    SetStop(true);
}

void Camera::onStopCameraThread(const int index)
{
    if (index == this->cameraID)
    {
        /* code */
        SetStop(true); // 设置停止标志
    }
}
