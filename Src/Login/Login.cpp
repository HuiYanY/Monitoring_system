#include "Login.h"
#include "ui_Login.h"

Login::Login(QWidget *parent)
    : QWidget(parent), ui(new Ui::Login)
{
    ui->setupUi(this);
    // // 在窗口构造函数中添加
    // this->setAttribute(Qt::WA_DeleteOnClose);
    // 透明化窗体
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint);

    // 控件设置阴影
    // 实例阴影shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    // 设置阴影距离
    shadow->setOffset(7,7);
    // 设置阴影颜色
    shadow->setColor(QColor("#444444"));
    // 设置阴影圆角
    shadow->setBlurRadius(10);
    // 给pushButton设置阴影
    ui->pushButton_3->setGraphicsEffect(shadow);
    ui->pushButton_5->setGraphicsEffect(shadow);

    // 设置按钮的焦点策略为 NoFocus，避免选中时出现白边
    ui->pushButton_6->setFocusPolicy(Qt::NoFocus);

    // 默认显示登录界面
    ui->widget_3->hide();
    
    // 信号绑定
    connect(ui->pushButton,&QPushButton::clicked,this,&Login::show_login);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&Login::show_register);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&Login::loging);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &Login::loging);
    connect(ui->lineEdit_2, &QLineEdit::returnPressed, this, &Login::loging);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&Login::regist);
    connect(ui->lineEdit_3,&QLineEdit::returnPressed,this,&Login::regist);
    connect(ui->lineEdit_4,&QLineEdit::returnPressed,this,&Login::regist);
    connect(ui->lineEdit_5,&QLineEdit::returnPressed,this,&Login::regist);
    connect(ui->pushButton_6,&QPushButton::clicked,this,&Login::showPassWord);
}

Login::~Login()
{
}

void Login::loging()
{
    // 对比配置文件中的账户和密码
    // 登录成功就显示主界面窗口
    if (this->compare())
    {
        /* code */
        // 发出登录成功的信号
        emit this->loginSuccess(this->userName);
    }
}

bool Login::accountIsExists(QString userName)
{
    QSettings settings("./ini/user.ini", QSettings::IniFormat);
    settings.setIniCodec("utf-8"); // 防止中文分组或键名乱码

    QStringList groups = settings.childGroups();
    for (int i = 0; i < groups.size(); i++)
    {
        if (settings.value(groups[i]+"/userName") == userName)
        {
            return true;
        }
    }
    return false;
}

void Login::regist()
{
    // 读取输入框
    this->userName = ui->lineEdit_3->text();
    if (accountIsExists(this->userName))
    {
        /* code */
        QMessageBox::warning(this,"警告","该用户名已存在！",QMessageBox::Ok);
        return;
    }
    
    if (ui->lineEdit_4->text() == ui->lineEdit_5->text())
    {
        /* code */
        this->passWord = ui->lineEdit_4->text();
        // 将其写入配置文件
        this->writeInIni();
    }else{
        // 弹窗提醒
        QMessageBox::warning(this,"警告","请确认密码，两次输入不一致！",QMessageBox::Ok);
    }
}

// 窗口移动函数

/*****************切换窗体的函数共两个*******************/
/*
* 登录的widget
*/
void Login::show_login(){
    ui->widget_3->hide();
    ui->widget_2->show();
}
/*
* 注册的widget
*/
void Login::show_register()
{
    ui->widget_2->hide();
    ui->widget_3->show();
}

void Login::showPassWord()
{
    if (ui->lineEdit_2->echoMode() == QLineEdit::Password)
    {
        ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
    }else{
        ui->lineEdit_2->setEchoMode(QLineEdit::Password);
    }
}

/*************实现窗口拖拽***************/

void Login::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void Login::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void Login::mouseReleaseEvent(QMouseEvent *event){
    dragPosition = QPoint();
    event->accept();
}


/*****************Ini文件处理*****************/
void Login::writeInIni()
{
    QSettings settings("./ini/user.ini", QSettings::IniFormat);
    settings.setIniCodec("utf-8"); // 防止中文分组或键名乱码

    QStringList groups = settings.childGroups();
    if (!groups.isEmpty()) 
    {
        int num = groups.last().toInt() + 1;
        QString userInfo = QString("%1").arg(num, 3, 10,QChar('0'));
        settings.setValue(userInfo+"/userName", this->userName); // 自动创建"NewGroup"
        settings.setValue(userInfo+"/passWord", QString::fromStdString((bcrypt::generateHash(this->passWord.toStdString())))); // 自动创建"NewGroup"
        // 弹窗提醒userid
        QMessageBox::information(this,"信息","请记住您的用户名！",QMessageBox::Ok);
        show_login();
    }
    else
    {
        settings.setValue("001/userName", this->userName); // 自动创建"NewGroup"
        settings.setValue("001/passWord", QString::fromStdString((bcrypt::generateHash(this->passWord.toStdString())))); // 自动创建"NewGroup"
        // 弹窗提醒userid
        QMessageBox::information(this,"信息","请记住您的用户名！",QMessageBox::Ok);
        show_login();
    }
}

void Login::readIni(QString user)
{
    QSettings settings("./ini/user.ini", QSettings::IniFormat);

    // 获取所有的分组
    QStringList groupList = settings.childGroups();
    // 遍历分组
    for(auto group : groupList)
    {
        settings.beginGroup(group);
        if(user == settings.value("userName").toString())
        {
            // 读取用户的参数
            this->userName = settings.value("userName").toString();
            this->passWord = settings.value("passWord").toString();
            settings.endGroup();
            break;
        }
        settings.endGroup();
    }
    
}

bool Login::compare()
{
    // 获取user
    QString user = ui->lineEdit->text();
    // 读取配置文件
    this->readIni(user);
    // 将密码与输入密码对比
    QString inPassWord = ui->lineEdit_2->text();
    if (bcrypt::validatePassword(inPassWord.toStdString(),this->passWord.toStdString()))
    {
        qDebug() << "Login!";
        return true;
    }
    QMessageBox::warning(this,"错误","密码或用户名错误，请重新输入！",QMessageBox::Ok);
    return false;
}
