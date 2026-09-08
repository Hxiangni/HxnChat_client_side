#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

//Qt 的命名空间宏
//声明Ui::LoginDialog类
QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE
class QLabel;
class QPushButton;
class QPropertyAnimation;
class QLineEdit;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();


    //软槽（由 initConnect 手动 connect）
    void onGetCodeClicked();          // 获取验证码
private:
    void initUI();
    void initConnect();
    void slideTo(bool toRegister);   // 滑块滑动到目标侧 + 文案翻转
    Ui::LoginDialog *ui;//资源管理器指针管理里面的控件

    //滑块（M3/M4）
    QWidget           *slider_        = nullptr;   // 滑块本体（浅粉渐变欢迎卡）
    QLabel            *sliderTitle_   = nullptr;   // 顶部小号 overline
    QLabel            *sliderUser_    = nullptr;   // 用户名（上次登录，微信式主视觉）
    QLabel            *sliderDesc_    = nullptr;   // 小字副文
    QLabel            *slideOverlay_  = nullptr;   // 滑动快照层：动画期间只搬这张截图，避免每帧重绘滑块控件
    QPropertyAnimation *slideAnim_    = nullptr;   // 滑动动画句柄

    // 代码创建的控件按"面板"打包（迷你版 Ui:: 聚合，避免平铺一堆裸指针）
    struct RegisterFields {                        // 注册面板(左半边，平时被滑块盖住)
        QLineEdit   *usernameEdit_ = nullptr;      // 用户名
        QLineEdit   *emailEdit_    = nullptr;      // 邮箱
        QLineEdit   *codeEdit_     = nullptr;      // 验证码
        QPushButton *codeBtn_      = nullptr;      // 获取验证码
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

    RegisterFields registerFields_;                // 注册面板控件打包
    LoginFields    loginFields_;                   // 登录面板控件打包
    QPushButton    *closeBtn_      = nullptr;      // 右上角关闭按钮
};

#endif // LOGINDIALOG_H
