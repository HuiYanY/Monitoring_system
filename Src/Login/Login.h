#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QSettings>
#include <QStringList>
#include <QDebug>
#include <QDialog>
#include <QMessageBox>

#include "bcrypt.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT
private:
    /* data */
    Ui::Login *ui;
    QPoint dragPosition;

    QString userName;
    QString passWord;

private:
    // 账户是否存在
    bool accountIsExists(QString userName);
    // 存入配置文件
    void writeInIni();
    // 读取配置文件
    void readIni(QString user);
    // 对比账户密码文件
    bool compare();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
public:
    Login(QWidget *parent = nullptr);
    ~Login();
    // 登录验证
    void loging();
    // 注册
    void regist();
signals:
    void loginSuccess(const QString &username);
private slots:
    void show_login();
    void show_register();
    void showPassWord(); // 显示密码
};

#endif