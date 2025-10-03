#ifndef CAMERA_SAVE_H
#define CAMERA_SAVE_H

#include <QThread>
#include <QDebug>
#include <QString>
#include <QDateTime>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class CameraSave : public QThread
{
    Q_OBJECT
private:
    // 输入地址
    QString url {""};

    AVFormatContext *ps {nullptr};
    AVDictionary *options {nullptr};

    AVCodecParameters* video_codecparm {nullptr};
    const AVCodec* video_codec {nullptr};
    AVCodecContext* video_codec_ctx {nullptr};
    int video_stream_index {-1};
    AVStream *in_stream {nullptr};

    // 输出使用的变量
    QString output_name {"output.mp4"};
    AVFormatContext *output_ctx {nullptr};
    AVStream *out_stream {nullptr};

    bool m_stop {false};

public:
    static int outtime;

public:
    CameraSave(QString url);
    CameraSave(QString url, QString output_name);
    ~CameraSave();
    void run() override;
    void stopWork() { m_stop = true; } 
private:
    int open_Video_Stream();
    int find_Video_stream();
    void open_avcodec();
    bool init_save();
    void save_mp4();
};

#endif