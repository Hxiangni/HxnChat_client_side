#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMap>
#include <QJsonObject>
#include <functional>   // std::function
#include "global.h"
//Qt 的命名空间宏
//声明Ui::LoginDialog类
QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QPropertyAnimation;





class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();


private:
    void initUI();
    void initConnect();
    void slideTo(bool toRegister);   // 滑块滑动到目标侧 + 文案翻转
    void showTip(const QString &str, bool isOk = false);   // err_tip 提示：红(err)/绿(ok)
    void initHttpHandler();
    Ui::LoginDialog *ui;//资源管理器指针管理里面的控件

private slots:
    // 接收「注册模块」HTTP 结果的软槽（在 initConnect 里 connect）
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    //软槽（由 initConnect 手动 connect）
    void onGetCodeClicked ();          // 获取验证码
    // 注册
    void slot_registerBtn_clicked();
private:
    //滑块（M3/M4）
    QWidget           *slider_        = nullptr;   // 滑块本体（浅粉渐变欢迎卡）
    QLabel            *sliderTitle_   = nullptr;   // 顶部小号 overline
    QLabel            *sliderUser_    = nullptr;   // 用户名（上次登录，微信式主视觉）
    QLabel            *sliderDesc_    = nullptr;   // 小字副文
    QLabel            *slideOverlay_  = nullptr;   // 滑动快照层：动画期间只搬这张截图，避免每帧重绘滑块控件
    QPropertyAnimation *slideAnim_    = nullptr;   // 滑动动画句柄

    // 注册/登录面板的控件集合——只属于本对话框，故做成私有内嵌类 + 值对象：
    // 结构体对象随 LoginDialog 自动构造/析构，无需手动 new/delete；
    // 结构体里存放的控件指针指向的控件本体，由 Qt 父子树在销毁对话框时统一回收。
    struct RegisterFields {                        // 注册面板(左半边，平时被滑块盖住)
        QLineEdit   *usernameEdit_ = nullptr;      // 用户名
        QLineEdit   *emailEdit_    = nullptr;      // 邮箱
        QLineEdit   *codeEdit_     = nullptr;      // 验证码
        QPushButton *codeBtn_      = nullptr;      // 获取验证码
        QLabel      *errTip_       = nullptr;      // 红/绿两态提示（state 属性驱动 QSS）错误提示
        QLineEdit   *passwordEdit_ = nullptr;      // 密码
        QLineEdit   *confirmEdit_  = nullptr;      // 确认密码
        QPushButton *registerBtn_  = nullptr;      // 注册
        QPushButton *toLoginLink_  = nullptr;      // 已有账号？去登录
    };
    struct LoginFields {                           // 登录面板(右半边，初始可见)
        QLineEdit   *accountEdit_   = nullptr;     // 账号
        QLineEdit   *passwordEdit_  = nullptr;     // 密码
        QPushButton *loginBtn_      = nullptr;     // 登录
        QPushButton *toRegisterLink_ = nullptr;    // 没有账号？去注册
    };

    RegisterFields registerFields_;                // 注册面板控件打包（值对象，免管理）
    LoginFields    loginFields_;                   // 登录面板控件打包（值对象，免管理）
    QPushButton    *closeBtn_      = nullptr;      // 右上角关闭按钮
    //std::map
    //根据ReqId分配对应的接口函数
    //接收一个QJsonObject返回为void
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
};

#endif // LOGINDIALOG_H
