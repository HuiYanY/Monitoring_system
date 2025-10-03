#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_AddCamera.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //初始化窗口
    ui->setupUi(this);
    this->AddCamera = new QWidget();
    this->uiAddCamera = new Ui::AddCamera;
    this->uiAddCamera->setupUi(this->AddCamera);

    // 初始化参数
    this->stackedWidget = ui->stackedWidget;

    // 菜单
    // 创建菜单项
    ui->menubar->addAction("实时显示");
    ui->menubar->addAction("视频回放");
    ui->menubar->addAction("设置");

    // 设置TableView
    this->initTableView();

    this->loginWin = new Login();
    this->loginWin->show();
    this->AddCamera->hide();

    this->readConfigFile();

    // 初始化设置
    ui->checkBox->setChecked(this->isOnPlayAI);
    ui->checkBox_2->setChecked(this->isOnPlayBackAI);
    ui->checkBox_3->setChecked(this->isOnGPU);

    this->stackedWidget->setCurrentIndex(0); // 默认显示实时显示页面

    // 创建一个定时器，用于每秒更新帧率
    this->timer = new QTimer(this);
    this->timer->setInterval(10); // 每10ms触发一次
    connect(timer, &QTimer::timeout, this, &MainWindow::showCameraIamge); // 连接定时器的timeout信号到槽函数
    this->timer->start(); // 启动定时器

    // // 绑定信号
    connect(this->loginWin, &Login::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(ui->menubar, &QMenuBar::triggered, this, &MainWindow::pageChange);
    connect(ui->pushButton, &QPushButton::clicked, this, [=]()->void{
        this->AddCamera->show();
    });
    connect(this->uiAddCamera->pushButton, &QPushButton::clicked, this, &MainWindow::addCameraSlot);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::cleckedOpen);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::cleckedDelete);
    connect(this, &MainWindow::refreshCameraList, this, &MainWindow::refreshCamera);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::saveConfigFile);
}

MainWindow::~MainWindow()
{
    // 保存配置文件
    writeConfigFile();

    this->timer->stop(); // 停止定时器

    // 发送停止信号
    emit stopCameraThread(); // 发送停止信号

    // 等待所有线程停止
    for (int i = 0; i < this->cameraNumber; i++)
    {
        if (_camera[i]) {
            _camera[i]->wait();
        }
    }

    // 等待回放线程结束
    if (this->_cameraBack != nullptr)
    {
        /* code */
        this->_cameraBack->wait();
    }

    // 录制线程
    for (int i = 0; i < this->cameraNumber; i++)
    {
        if (this->save_camera_video[i]) {
            this->save_camera_video[i]->stopWork();
            _camera[i]->wait();
        }
    }
    

    // 删除所有对象
    for (int i = 0; i < this->cameraNumber; i++)
    {
        if (_camera[i]) {
            delete _camera[i];
            _camera[i] = nullptr;
        }
    }

    if (AddCamera != nullptr)
    {
        /* code */
        delete (AddCamera);
    }
    

    if (loginWin != nullptr)
    {
        delete (loginWin);
    }
    
    delete ui;
}

// 页面切换
void MainWindow::pageChange(QAction *action)
{
    if (action->text() == "实时显示")
    {
        /* code */
        this->stackedWidget->setCurrentIndex(0);
        // 发送刷新界面信号，通知其他部分刷新摄像头列表
        emit refreshCameraList("实时显示");
        emit stopShow(1); // 显示
        this->currentPage = 1; 
    }
    else if (action->text() == "视频回放")
    {
        /* code */
        this->stackedWidget->setCurrentIndex(1);
        // 发送刷新界面信号，通知其他部分刷新摄像头列表
        emit refreshCameraList("视频回放");
        emit stopShow(2); // 显示
        this->currentPage = 2; 
    }
    else if (action->text() == "设置")
    {
        /* code */
        this->stackedWidget->setCurrentIndex(2);
        emit refreshCameraList("设置");
        emit stopShow(3); // 停止显示
        this->currentPage = 3; 
    }
    
}

int MainWindow::getCameraNumber()
{
    // 创建一个QSettings对象，用于读取配置文件
    // QSettings::IniFormat 指定配置文件的格式为INI格式
    QSettings settings("./ini/cameraInfo.ini", QSettings::IniFormat);

    // 获取所有的分组
    // childGroups() 方法返回一个QStringList，包含配置文件中所有的分组名称
    QStringList groupList = settings.childGroups();
    // 读取参数
    // 返回分组列表的大小，即分组数量，这里代表相机数量
    return groupList.size();
}

void MainWindow::addCameraSlot()
{
    // 从用户界面获取摄像头名称
    QString cameraName = this->uiAddCamera->lineEdit_2->text();
    // 从用户界面获取摄像头URL
    QString url = this->uiAddCamera->lineEdit->text();
    // 创建一个新的Camera对象，并将当前窗口作为其父对象
    Camera *newCamera = new Camera(this->_camera.size(), url, cameraName, this);
    // 调用Camera对象的addCamera方法，传入摄像头名称和URL,将摄像头信息存储到文件中
    newCamera->addCamera(cameraName, url);
    if (newCamera != nullptr)
    {
        connect(this, static_cast<void (MainWindow::*)()>(&MainWindow::stopCameraThread), newCamera,static_cast<void (Camera::*)()>(&Camera::onStopCameraThread));
        connect(this, &MainWindow::setShowScreenIndex, newCamera,&Camera::setShowScreenIndex);
        connect(this, &MainWindow::stopShowOneCamera, newCamera,&Camera::showStopOne);
        connect(this, &MainWindow::stopShow, newCamera,&Camera::showStop);
    }
    // 将新的Camera对象添加到_camera数组中
    _camera.push_back(newCamera);
    _camera.last()->SetIsOnAI(this->isOnPlayAI);
    _camera.last()->SetAi("yolo11n.onnx",this->isOnGPU);
    _camera.last()->start();
    // 发送刷新界面信号，通知其他部分刷新摄像头列表
    emit refreshCameraList("实时显示");
    this->AddCamera->hide();
}

void MainWindow::initTableView()
{
    // 设置tableView的标题
    // 创建一个 QStandardItemModel 对象
    this->cameraListModel.push_back(new QStandardItemModel(this));
    this->cameraListModel.push_back(new QStandardItemModel(this));
    this->cameraListModel.push_back(new QStandardItemModel(this));

    // 设置模型的列数和列标题
    this->cameraListModel[0]->setColumnCount(2);
    this->cameraListModel[0]->setHeaderData(0, Qt::Horizontal, "摄像头名称");
    this->cameraListModel[0]->setHeaderData(1, Qt::Horizontal, "摄像头URL");
    this->cameraListModel[1]->setColumnCount(1);
    this->cameraListModel[1]->setHeaderData(0, Qt::Horizontal, "文件名称");
    this->cameraListModel[2]->setColumnCount(1);
    this->cameraListModel[2]->setHeaderData(0, Qt::Horizontal, "登录用户");
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 设置列宽自动调整
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置单行选择模式和选择整行的行为
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // 设置单行选择模式
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  // 设置选择行为为选择整行
}

void MainWindow::addItem2TableView(QString cameraName, QString url)
{
    QStandardItem *nameItem = new QStandardItem(cameraName);
    QStandardItem *urlItem = new QStandardItem(url);
    // 获取模型的实例
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (model) {
        // 在模型的实例上调用 appendRow 函数
        model->appendRow(QList<QStandardItem*>() << nameItem << urlItem);
    }
}

void MainWindow::addItem2TableView(QString fileName)
{
    // 创建一个 QStandardItem 对象，用于存储文件名
    QStandardItem *nameItem = new QStandardItem(fileName);
    // 获取模型的实例
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (model) {
        // 在模型的实例上调用 appendRow 函数，将文件名添加到表格中
        model->appendRow(QList<QStandardItem*>() << nameItem);
    }
}

void MainWindow::openCamera()
{
    // 获取当前选中的行
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        /* code */
        return;
    }
    // 获取当前选中的行的索引
    QModelIndex selectedIndex = selectedIndexes.first();
    // 获取当前选中的行的行号
    int index = selectedIndex.row();

    // 发送信号，通知摄像头设置显示屏幕index
    if (this->showScreenIndex < 4 && this->showScreenIndex >= 0)
    {
        // 判断当前屏幕是否有摄像头显示
        if (this->showingCameraIndex[this->showScreenIndex] == -1)
        {
            /* code */
            if (_camera[index] != nullptr)
            {
                emit setShowScreenIndex(index, this->showScreenIndex); // 发送信号
                this->showingCameraIndex[this->showScreenIndex] = index;
                this->showScreenIndex++;
            }
        }
    }
    else{
        // 摄像头数量超过当前显示界面数量，关闭一个摄像头，打开新的摄像头
        // 关闭当前屏幕的摄像头
        closeCamera(this->showingCameraIndex[this->showScreenIndex%4]);
        // 打开新的摄像头
        if (_camera[index] != nullptr)
        {
            emit setShowScreenIndex(index, this->showScreenIndex%4); // 发送信号
            this->showingCameraIndex[this->showScreenIndex%4] = index;
            this->showScreenIndex++;
        }
    }
}

void MainWindow::closeCamera(int index)
{
    // 发送信号，通知摄像头设置显示屏幕index
    emit stopShowOneCamera(index);
    QThread::msleep(100); // 等待摄像头关闭
    emit setShowScreenIndex(index, -1); // 发送信号
    // 将摄像头的显示屏幕index置为-1
    this->showingCameraIndex[index] = -1;
}

void MainWindow::refreshCamera(QString screenName)
{
    // 根据传入的screenName参数决定刷新哪个界面的摄像头列表
    if (screenName == "实时显示")
    {
        /* code */
        // 清空tableView
        if (this->cameraListModel[0] != nullptr) {
            this->cameraListModel[0]->removeRows(0,cameraListModel[0]->rowCount()); //删除所有行; // 清空当前模型
            ui->tableView->setModel(this->cameraListModel[0]); 
            ui->tableView->update(); // 刷新视图
        }
        // 读取INI文件
        QSettings settings("./ini/cameraInfo.ini", QSettings::IniFormat);

        // 获取所有的分组
        QStringList groupList = settings.childGroups();

        // 遍历每个分组
        for (int i = 0; i < groupList.size(); i++)
        {
            // 进入分组
            settings.beginGroup(groupList[i]);

            // 读取参数
            QString cameraName = settings.value("cameraName").toString();
            QString url = settings.value("url").toString();

            // 将参数添加到列表中
            this->addItem2TableView(cameraName, url);

            // 退出分组
            settings.endGroup();
        }
    }
    if (screenName == "视频回放")
    {
        /* code */
        // 清空tableView
        if (this->cameraListModel[1] != nullptr) {
            this->cameraListModel[1]->removeRows(0,cameraListModel[1]->rowCount()); //删除所有行; // 清空当前模型
            ui->tableView->setModel(this->cameraListModel[1]); 
            ui->tableView->update(); // 刷新视图
        }
        // 读取Save路径下的所有文件
        QDir dir("./Save");
        QStringList filters;
        filters << "*.h265";
        dir.setNameFilters(filters);
        QStringList fileList = dir.entryList();
        // 将文件名添加到列表中
        for (int i = 0; i < fileList.size(); i++)
        {
            /* code */
            this->addItem2TableView(fileList[i]);
        }
    }
    if (screenName == "设置")
    {
        /* code */
        // 清空tableView
        if (this->cameraListModel[2] != nullptr) {
            this->cameraListModel[2]->removeRows(0,cameraListModel[2]->rowCount()); //删除所有行
            ui->tableView->setModel(this->cameraListModel[2]); 
            ui->tableView->update(); // 刷新视图
        }
        // 将用户名添加到列表中
        this->addItem2TableView(this->userName);  
    }
    
    
    
}

void MainWindow::readConfigFile()
{
    QSettings settings("./ini/APPSetting.ini", QSettings::IniFormat);

    // 读取用户的参数
    // 读取参数
    this->isOnPlayAI = settings.value("AI/isOnPlayAI").toBool();
    this->isOnPlayBackAI = settings.value("AI/isOnPlayBackAI").toBool();
    this->isOnGPU = settings.value("AI/isOnGPU").toBool();
}

void MainWindow::writeConfigFile()
{
    QSettings settings("./ini/APPSetting.ini", QSettings::IniFormat);
    // Qt5 启用
    // settings.setIniCodec("utf-8"); // 防止中文分组或键名乱码

    settings.setValue("AI/isOnPlayAI", ui->checkBox->isChecked()); // 自动创建"NewGroup"
    settings.setValue("AI/isOnPlayBackAI", ui->checkBox_2->isChecked()); // 自动创建"NewGroup"
    settings.setValue("AI/isOnGPU", ui->checkBox_3->isChecked()); // 自动创建"NewGroup"
}

void MainWindow::saveConfigFile()
{
    writeConfigFile();
}

void MainWindow::deleteCamera()
{
    // 获取当前选中的行
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        /* code */
        return;
    }
    // 获取当前选中的行的索引
    QModelIndex selectedIndex = selectedIndexes.first();
    // 获取当前选中的行的行号
    int index = selectedIndex.row();
    // 删除INI文件中的对应项
    QSettings settings("./ini/cameraInfo.ini", QSettings::IniFormat);
    QStringList groups = settings.childGroups();
    if (index < groups.size()) {
        settings.beginGroup(groups[index]);
        settings.remove(""); // 删除当前组
        settings.endGroup();

        // 如果要删除的组不是最后一个组，需要对后面的组进行重命名
        for (int i = index + 1; i < groups.size(); i++) {
            QString currentGroup = groups[i];
            QString newGroupName = QString("%1").arg(i - 1, 3, 10, QLatin1Char('0')); // 生成新的组名，例如将 [002] 改为 [001]

            settings.beginGroup(currentGroup);
            QString cameraName = settings.value("cameraName").toString();
            QString url = settings.value("url").toString();
            QString outFileName = settings.value("outputFile").toString();
            settings.endGroup();

            settings.beginGroup(newGroupName);
            settings.setValue("cameraName", cameraName); // 自动创建"NewGroup"
            settings.setValue("url", url); // 自动创建"NewGroup"
            settings.setValue("outputFile", outFileName); // 自动创建"NewGroup"
            settings.endGroup();

            settings.remove(currentGroup);
        }
    }
    // 删除当前行
    this->cameraListModel[0]->removeRow(index);
    // 刷新tableView
    ui->tableView->update();
}

void MainWindow::deleteVideo()
{
    // 获取当前选中的行
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        /* code */
        return;
    }
    // 获取当前选中的行的索引
    QModelIndex selectedIndex = selectedIndexes.first();
    // 获取当前选中的行的行号
    int index = selectedIndex.row();
    // 删除Save文件夹中的对应文件
    QString fileName = this->cameraListModel[1]->item(index, 0)->text();
    QFile::remove("./Save/" + fileName); // 删除文件
    // 删除当前行
    this->cameraListModel[1]->removeRow(index);
    // 刷新tableView
    ui->tableView->update();
}

void MainWindow::cleckedOpen()
{
    switch (ui->stackedWidget->currentIndex())
    {
    case 0:
        this->openCamera(); // 打开摄像头
        break;
    
    case 1:
        this->PlayBack(); // 视频回放
        break;
    
    default:
        break;
    }
    
}

void MainWindow::cleckedDelete()
{
    int currentIndex = ui->stackedWidget->currentIndex(); // 获取当前页面的索引
    switch (currentIndex)
    {
    case 0:
        /* code */
        this->deleteCamera(); // 删除摄像头
        break;

    case 1:
        /* code */
        this->deleteVideo(); // 删除视频
        break;

    default:
        break;
    }
}

void MainWindow::showCameraIamge()
{
    int tempHeight = ui->label_0->height();
    int tempWidth = ui->label_0->width();
    QImage image;
    QPixmap pixmap;
    for (int i = 0; i < SCREEN_NUMBER-1; i++)
    {
        if(!this->_camera.isEmpty() && this->showingCameraIndex[i] != -1 && this->currentPage == 1)
        {
            image = this->_camera[this->showingCameraIndex[i]]->frameQueue.pop();
            switch (i)  
            {
            case 0:
                if (image.height() > 0)
                {
                    pixmap = QPixmap::fromImage(image.scaled(tempWidth,tempHeight));
                    ui->label_0->clear();
                    ui->label_0->setPixmap(pixmap);
                }
                break;
            case 1:
                if (image.height() > 0)
                {
                    pixmap = QPixmap::fromImage(image.scaled(tempWidth,tempHeight));
                    ui->label_1->clear();
                    ui->label_1->setPixmap(pixmap);
                }
                break;
            case 2:
                if (image.height() > 0)
                {
                    pixmap = QPixmap::fromImage(image.scaled(tempWidth,tempHeight));
                    ui->label_2->clear();
                    ui->label_2->setPixmap(pixmap);
                }
                break;
            case 3:
                if (image.height() > 0)
                {
                    pixmap = QPixmap::fromImage(image.scaled(tempWidth,tempHeight));
                    ui->label_3->clear();
                    ui->label_3->setPixmap(pixmap);
                }
                break;
            default:
                break;
            }
        }
    }
    if (this->showingCameraIndex[SCREEN_NUMBER-1] == SCREEN_NUMBER && this->currentPage == 2)
    {
        image = this->_cameraBack->frameQueue.pop();
        tempHeight = ui->label_4->height();
        tempWidth = ui->label_4->width();
        if (image.height() > 0)
        {
            QPixmap pixmap = QPixmap::fromImage(image.scaled(tempWidth,tempHeight));
            ui->label_4->clear();
            ui->label_4->setPixmap(pixmap);
        }
    }
}

void MainWindow::onLoginSuccess(const QString userName)
{
    this->show();
    this->loginWin->hide();
    this->userName = userName; // 保存用户名
    //监控显示
    this->Play();
}

void MainWindow::Play()
{
    // 获取摄像头数量
    this->cameraNumber = this->getCameraNumber();
    // 解码视频流数据
    for (int i = 0; i < cameraNumber; i++)
    {
        _camera.push_back(new Camera(i,this));
        _camera.last()->readCamera();

        _camera.last()->SetIsOnAI(this->isOnPlayAI);
        _camera.last()->SetAi("yolo11n.onnx",this->isOnGPU);

        // 添加进TableView
        ui->tableView->setModel(this->cameraListModel[0]); // 设置模型
        this->addItem2TableView(_camera.last()->getCameraName(),_camera.last()->getURL());
        
        if (_camera[i] != nullptr)
        {
            // 显示实时视频流
            connect(this, static_cast<void (MainWindow::*)()>(&MainWindow::stopCameraThread), _camera[i],static_cast<void (Camera::*)()>(&Camera::onStopCameraThread));
            connect(this, &MainWindow::setShowScreenIndex, _camera[i],&Camera::setShowScreenIndex);
            connect(this, &MainWindow::stopShowOneCamera, _camera[i],&Camera::showStopOne);
            connect(this, &MainWindow::stopShow, _camera[i],&Camera::showStop);
            _camera.last()->start();
            // 存储视频流
            this->save_camera_video.push_back(new CameraSave(_camera.last()->getURL(),("./Save/"+_camera.last()->getCameraName())));
            this->save_camera_video.last()->start();
        }
    }
}

void MainWindow::PlayBack()
{
    QString file = "./Save/"; // 获取回放视频文件名
    // 从TableView获取回放视频文件名
    // 获取当前选中的行
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        /* code */
        return;
    }
    // 获取当前选中的行的索引 // 获取当前选中的行的行号
    int selectedIndex = selectedIndexes.first().row();
    // 获取该行的某一列的内容
    QModelIndex index = ui->tableView->model()->index(selectedIndex, 0);

    // 获取该单元格的内容
    QVariant data = index.data();
    if (data.isValid())
    {
        // 处理获取到的数据，例如转换为QString
        file += data.toString();
    }
    else
    {
        return;
    }
    // 设置回放视频路径
    if (this->_cameraBack != nullptr) //如果回放摄像头存在，则停止回放摄像头
    {
        /* code */
        emit stopCameraThread(-1);
        // 等待摄像头线程结束
        this->_cameraBack->wait();
        delete this->_cameraBack;
        this->_cameraBack = nullptr;
        this->showingCameraIndex[SCREEN_NUMBER-1] = -1;
    }
    // 创建回放摄像头
    this->_cameraBack = new Camera(-1,file,"backPlay",this);
    // 回放视频
    this->_cameraBack->SetIsOnAI(this->isOnPlayBackAI); // 设置AI是否开启
    this->_cameraBack->SetAi("yolo11n.onnx",this->isOnGPU); // 设置AI模型
    connect(this, static_cast<void (MainWindow::*)(const int)>(&MainWindow::stopCameraThread), this->_cameraBack,static_cast<void (Camera::*)(const int)>(&Camera::onStopCameraThread)); // 连接信号和槽, 停止回放摄像头
    connect(this, static_cast<void (MainWindow::*)()>(&MainWindow::stopCameraThread), this->_cameraBack,static_cast<void (Camera::*)()>(&Camera::onStopCameraThread)); // 连接信号和槽,停止全部摄像头
    connect(this, &MainWindow::setShowScreenIndex, this->_cameraBack,&Camera::setShowScreenIndex);
    connect(this, &MainWindow::stopShow, _cameraBack,&Camera::showStop);
    emit setShowScreenIndex(-1,4);
    this->showingCameraIndex[SCREEN_NUMBER-1] = SCREEN_NUMBER;
    this->_cameraBack->start();
}


