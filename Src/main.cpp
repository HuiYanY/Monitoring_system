#include <QApplication>
#include <QDebug>

#include <MainWindow.h>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
    qDebug() << "Monitoring_system V 2.0.0";
    qDebug() << "Opencv version: " << CV_VERSION;
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "FFmpeg version:" << av_hwdevice_find_type_by_name("hevc_dxva2");
    //获取avutil数字版本号
	int version = avutil_version();
	//获取avutil三个子版本号
	int a = version / (int) pow(2, 16);
	int b = (int) (version - a * pow(2, 16)) / (int) pow(2, 8);
	int c = version % (int) pow(2, 8);
	//拼接avutil完整版本号
	QString versionStr = QString::number(a) + '.' + QString::number(b) + '.' + QString::number(c);
	qDebug() << "MeidaPlayer ffmpeg/avutil version " << versionStr;
    qDebug() << "HuiYan";

    //start code
    QApplication app(argc, argv);
    
    MainWindow w;

    //end code
    return app.exec();
}
