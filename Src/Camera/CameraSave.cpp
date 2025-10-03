#include "CameraSave.hpp"

// 在类外定义并初始化
int CameraSave::outtime = 60000;

CameraSave::CameraSave(QString url)
    :url(url)
{
    
}

CameraSave::CameraSave(QString url, QString output_name)
    :url(url),output_name(output_name)
{
}

CameraSave::~CameraSave()
{
    avcodec_free_context(&(this->video_codec_ctx));
    avformat_close_input(&(this->ps));

}

void CameraSave::run()
{
    if(this->open_Video_Stream() == 0) {
        this->find_Video_stream();
        this->open_avcodec();
        if(this->init_save()){
            this->save_mp4();
            av_write_trailer(this->output_ctx);
            avio_close(this->output_ctx->pb);
        }
    }
}

/*
 * 打开网络视频流
 * 超时时间设置为5S
 */
int CameraSave::open_Video_Stream()
{
    av_dict_set(&(this->options),"timeout","5000000",0);
    return avformat_open_input(&(this->ps),this->url.toStdString().c_str(),NULL,&(this->options));
}

/*
 * 查找视频流
 * 返回值 = -1 没有找到流信息
 * 返回值 = 0 没有找到视频流
 * 返回值 = 1 找到视频流
 *
*/
int CameraSave::find_Video_stream()
{
    // 填充 AVFormatContext 中的 streams 数组
    if(avformat_find_stream_info(this->ps,nullptr) < 0){
        qDebug() << "Couldn't find stream information.";
        return -1;
    }

    // 查找视频流
    // 遍历AVFormatContext的流数组，查看流的类型。
    for (unsigned int i = 0; i < this->ps->nb_streams; i++)
    {
        if (this->ps->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            this->video_stream_index = i;
            this->in_stream = this->ps->streams[i];
            this->video_codecparm = this->ps->streams[i]->codecpar;
            return 1;
        }
    }
    
    return 0;
}

/*
 * 打开编解码器
 */
void CameraSave::open_avcodec()
{
    // 获取编解码器
    this->video_codec = avcodec_find_decoder(video_codecparm->codec_id);
    if (!(this->video_codec))
    {
        qDebug() << "Unsupported codec.\n";
        return;
    }

    // 分配编解码器上下文
    this->video_codec_ctx = avcodec_alloc_context3(video_codec);
    if (!(this->video_codec_ctx)) {
        qDebug() << "Could not allocate video codec context.\n";
        return;
    }

    // 将 AVCodecParameters 结构体中的编解码器参数填充到 AVCodecContext 结构体中
    avcodec_parameters_to_context(this->video_codec_ctx,this->video_codecparm);

    // 打开编解码器
    if (avcodec_open2(this->video_codec_ctx, this->video_codec, nullptr) < 0) {
        qDebug() << "Could not open codec.\n";
        return;
    }


}

bool CameraSave::init_save()
{
    // 创建输出上下文
    this->output_ctx = avformat_alloc_context();
    if (!(this->output_ctx)) {
        qDebug() << "Could not create output context.\n";
        return false;
    }
    avformat_alloc_output_context2(&(this->output_ctx), nullptr, "mp4", nullptr);
    if (!(this->output_ctx)) {
        qDebug() << "Could not create output context.\n";
        return false;
    }

    // 创建输出流
    this->out_stream = avformat_new_stream(output_ctx,this->video_codec);

    // 将输入流的编解码器参数完整地复制到输出流，从而确保输出文件能够正确地承载视频数据。
    avcodec_parameters_copy(this->out_stream->codecpar, this->in_stream->codecpar);
    return true;
}

void CameraSave::save_mp4()
{
    QDateTime current = QDateTime::currentDateTime();
    // 打开输出流
    if (avio_open(&(this->output_ctx->pb), (this->output_name+"_"+current.toString("yyyy_MM_dd_hh_mm_ss")+".mp4").toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
        qDebug() << "Could not open output file.\n";
        return;
    }

    // 写入文件头
    if (avformat_write_header((this->output_ctx), nullptr) < 0) {
        qDebug() << "Could not write header.\n";
        return;
    }

    AVPacket packet;
    while (av_read_frame(this->ps, &packet) >= 0 && !(this->m_stop)) {
        if (packet.stream_index == video_stream_index) {
            this->in_stream = this->ps->streams[packet.stream_index];
            this->out_stream = output_ctx->streams[packet.stream_index];

            packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
            packet.pos = -1;

            if (av_interleaved_write_frame(output_ctx, &packet) < 0) {
                qDebug() << "Error muxing packet.\n";
                av_packet_unref(&packet);
                break;
            }

            av_packet_unref(&packet);

        }
    }
}
