#ifndef MAIN_WINDOWS_H
#define MAIN_WINDOWS_H

#include <QMainWindow>
#include <QImage>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QDir>
#include <QTimer>
#include "Camera.h"
#include "model.h"
#include "Login.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Ui { class AddCamera; }
QT_END_NAMESPACE

#define SCREEN_NUMBER 5 // 显示的屏幕个数
// 最后一个为回放显示

class MainWindow : public QMainWindow
{
Q_OBJECT
private:
    /* data */
    Ui::MainWindow *ui {nullptr};
    
    QVector<Camera*> _camera;
    Camera* _cameraBack {nullptr};

    Login *loginWin {nullptr};
    QString userName; // 用户名

    QWidget *AddCamera {nullptr};
    Ui::AddCamera *uiAddCamera {nullptr};

    QStackedWidget *stackedWidget {nullptr};

    int cameraNumber {0}; // 摄像头个数

    unsigned int showScreenIndex {0}; // 显示的屏幕索引
    int showingCameraIndex[SCREEN_NUMBER] {-1,-1,-1,-1,-1}; // 正在显示的摄像头索引

    QVector<QStandardItemModel*> cameraListModel; // 摄像头列表

    bool isOnPlayAI {false}; // 是否启用实时监控AI
    bool isOnPlayBackAI {false}; // 是否启用回放AI
    bool isOnGPU {false}; // 是否启用GPU

    QTimer *timer {nullptr};
    int currentPage {true}; // 当前页面是否为主页面
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // 执行播放操作
    void Play();
    // 回放
    void PlayBack();

    // 读取摄像头个数
    int getCameraNumber();

    // 加载ListView中的摄像头
    void initTableView();  // 初始化表格
    void addItem2TableView(QString cameraName, QString url);    
    void addItem2TableView(QString fileName);
    void openCamera(); // 打开摄像头
    void closeCamera(int index); // 关闭摄像头
    void refreshCamera(QString screenName); // 刷新摄像头列表

    // 读取配置文件
    void readConfigFile();
    // 写入配置文件
    void writeConfigFile();
    // 保存配置文件
    void saveConfigFile();
    
    // 删除摄像头
    void deleteCamera();
    // 删除视频
    void deleteVideo();

signals:
    void stopCameraThread(); // 停止摄像头线程
    void stopCameraThread(const int index); // 停止摄像头线程回放线程
    void refreshCameraList(QString screenName); // 刷新摄像头列表
    void setShowScreenIndex(int index, int screenIndex); // 设置显示的屏幕索引
    void stopShow(int page); // 停止显示画面
    void stopShowOneCamera(int cameraID); // 停止某一个设备显示画面

private slots: // 槽函数声明
    // 功能：在登录成功时被调用的函数，用于处理登录成功后的逻辑
    void onLoginSuccess(const QString userName);
    // 页面切换
    void pageChange(QAction* action);
    // 添加摄像头
    void addCameraSlot();
    // 点击打开按钮
    void cleckedOpen();
    // 点击删除按钮
    void cleckedDelete();

    void showCameraIamge(); // 显示摄像头画面
};

#endif
