#include "logindialog.h"
#include "ui_logindialog.h"
#include "global.h"      // repolish：改完动态属性后强制重算样式

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QRegularExpression>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QLinearGradient>
#include <QColor>
#include <QFont>

// TODO(后续接真实数据): 从「记住我 / 最近登录」配置读取上次登录用户名与头像路径，
// 替换掉下面这个占位演示常量，以及 makeRoundAvatar() 里的默认 logo。
static const QString kLastLoginUserName = QStringLiteral("HxnChat 用户");

// 生成 116×116 圆形头像：白色细描边圆 + 浅粉渐变默认 logo（白字 H）
static QLabel *makeRoundAvatar(QWidget *parent)
{
    const int size = 116;
    auto *avatar = new QLabel(parent);
    avatar->setFixedSize(size, size);

    QPixmap pm(size, size);
    pm.fill(Qt::transparent);

    {
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(255, 255, 255, 230), 3));     // 白色细描边

        QLinearGradient g(0, 0, size, size);               // 与 #slider 同色系低饱和渐变
        g.setColorAt(0.0, QColor(239, 192, 207));   // #efc0cf
        g.setColorAt(1.0, QColor(224, 156, 180));   // #e09cb4
        p.setBrush(g);
        p.drawEllipse(QRectF(2, 2, size - 4, size - 4));

        QFont f = p.font();
        f.setPixelSize(50);
        f.setBold(true);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(pm.rect(), Qt::AlignCenter, QStringLiteral("H"));
    }

    avatar->setPixmap(pm);
    return avatar;
}

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    //把 ui 文件定义的全部控件 new 出来，布局、属性全部设置好，挂载到this上。
    ui->setupUi(this);
    // registerFields_ / loginFields_ 是值对象成员，已随对象自动构造完毕，
    // initUI() 只需直接往里填控件指针即可。
    initUI();//这里为什么不放在ui->setupUi上面？
    initConnect();//信号绑定
}

LoginDialog::~LoginDialog()
{
    // registerFields_ / loginFields_ 是值对象，自动析构，无需手动 delete；
    // 控件本体由 Qt 父子树统一回收。这里只需释放 new 出来的 ui 包装。
    delete ui;
}

void LoginDialog::initUI()
{
    // ========== 1. 窗口基本属性 ==========
    // 真·无边框：去掉系统标题栏，且窗口背景透明 —— 只留下悬浮的圆角白卡，
    // 四周是透明留白（用来显示投影），不再有"浅粉背景板"的画中画感
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(940, 600);   // 860x520 卡片 + 四周 40px 透明留白放投影

    // ========== 2. 全局样式表（作用于本窗口及其所有子控件） ==========
    // 配色分组：粉系(左/注册/滑块) / 蓝系(右/登录) / 中性，改色只动这一处
    this->setStyleSheet(R"(
        #card {                                        /* 中性：白色圆角卡片 */
            background: #ffffff;
            border-radius: 24px;
        }

        #shadowHost {                                  /* 静止投影层：白色圆角，仅供模糊出投影 */
            background: #ffffff;
            border-radius: 24px;
        }

        #titleLabel {                                  /* 中性：两侧标题 */
            font-size: 26px;
            font-weight: bold;
            color: #3c3a3d;
        }

        /* ---------- 中性：输入框默认/悬停 ---------- */
        QLineEdit {
            background: #fbf3f6;
            border: 2px solid transparent;
            border-radius: 12px;
            padding: 9px 14px;
            font-size: 15px;
            color: #333333;
            selection-background-color: #f5c7d6;
        }
        QLineEdit:hover {
            border: 2px solid #eed2dc;
        }

        /* ---------- 粉系(左)：注册面板 ---------- */
        #registerPanel QLineEdit { background: #fdf5f8; }
        #registerPanel QLineEdit:focus {
            background: #ffffff;
            border: 2px solid #e297af;
        }
        #registerPanel #textLink {
            background: transparent;
            border: none;
            color: #e297af;
            font-size: 13px;
        }
        #registerPanel #textLink:hover  { color: #c96a93; }
        #registerPanel #textLink:pressed { color: #a84e73; }

        #btnRegister {                                 /* 注册主按钮：浅粉渐变 */
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #f2a9be, stop:1 #e68ba6);
            color: #ffffff;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 22px;
        }
        #btnRegister:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ec9cb3, stop:1 #dc7c99);
        }
        #btnRegister:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #e191ab, stop:1 #cf6b8a);
        }

        #btnCode {                                     /* 获取验证码：浅粉小按钮 */
            background: #f9e3ea;
            color: #c2547d;
            font-size: 13px;
            font-weight: bold;
            border: none;
            border-radius: 21px;
        }
        #btnCode:hover  { background: #f3cdd9; color: #a84266; }
        #btnCode:pressed { background: #ebb7c9; }

        /* ---------- 注册表单提示 err_tip：同一标签靠 state 属性切红/绿 ---------- */
        #registerPanel #regErrTip[state="err"] {
            color: #d94f5a;
            font-size: 12px;
            font-weight: bold;
        }
        #registerPanel #regErrTip[state="ok"] {
            color: #2e9e5b;
            font-size: 12px;
            font-weight: bold;
        }

        /* ---------- 蓝系(右)：登录面板 ---------- */
        #loginPanel QLineEdit { background: #f3f8fd; }
        #loginPanel QLineEdit:focus {
            background: #ffffff;
            border: 2px solid #7fb0da;
        }
        #loginPanel #textLink {
            background: transparent;
            border: none;
            color: #7fb0da;
            font-size: 13px;
        }
        #loginPanel #textLink:hover  { color: #4f87be; }
        #loginPanel #textLink:pressed { color: #3a6fa5; }

        #btnLogin {                                    /* 登录主按钮：浅蓝渐变 */
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #a5c6ea, stop:1 #86b0da);
            color: #ffffff;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 22px;
        }
        #btnLogin:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #92b9e2, stop:1 #6f9fd0);
        }
        #btnLogin:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #7ba7d2, stop:1 #558cc2);
        }

        /* ---------- 粉系：滑块欢迎卡 ---------- */
        #slider {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #efc0cf, stop:1 #e09cb4);
            border-radius: 24px;                       /* 与卡片圆角一致 */
        }
        #slider QLabel { background: transparent; }
        #sliderTitle {                                 /* 顶部小号欢迎语 */
            color: rgba(255, 255, 255, 235);
            font-size: 15px;
            font-weight: bold;
        }
        #sliderUser {                                  /* 用户名：主视觉白字 */
            color: #ffffff;
            font-size: 24px;
            font-weight: bold;
        }
        #sliderDesc {                                  /* 小字副文 */
            color: rgba(255, 255, 255, 210);
            font-size: 13px;
        }

        #btnClose {                                    /* 右上角关闭按钮 */
            background: transparent;
            border: none;
            border-radius: 17px;
            color: #b9a3ae;
            font-size: 16px;
            font-weight: bold;
        }
        #btnClose:hover {
            background: #ffe9f0;                       /* 浅粉底 + 深粉 X */
            color: #c2547d;
        }
        #btnClose:pressed {
            background: #f7d6e1;
            color: #a84266;
        }
    )");

    // ========== 3. 白色圆角卡片（带柔和投影）居中 ==========
    // 性能关键：把投影放在"静止投影层"上，而不是挂在 card 上。
    // 之前 QGraphicsDropShadowEffect 挂在 card：滑块每动一帧，Qt 都要把整张
    // 卡片离屏重渲染 + 模糊一遍 → 滑动卡顿。现在投影层永不移动，动画期间
    // Qt 只需重画普通卡片内容，每帧开销骤降，滑动就流畅了。
    auto *shadowHost = new QWidget(this);
    shadowHost->setObjectName("shadowHost");     // QSS：白色圆角，作为模糊源
    shadowHost->setGeometry(40, 40, 860, 520);
    auto *cardShadow = new QGraphicsDropShadowEffect(shadowHost);
    cardShadow->setBlurRadius(32);
    cardShadow->setOffset(0, 10);
    cardShadow->setColor(QColor(138, 92, 115, 80));
    shadowHost->setGraphicsEffect(cardShadow);

    //在这里card是一个局部变量出了init之后new的控件就没有指针指了！
    auto *card = new QWidget(this);
    card->setObjectName("card");            // 让上面 #card 样式找到它
    card->setFixedSize(860, 520);

    auto *outer = new QVBoxLayout(this);    // 最外层布局，四周留 40 透明留白（放投影）
    outer->setContentsMargins(40, 40, 40, 40);
    outer->addWidget(card, 0, Qt::AlignCenter);

    // ========== 4. 注册面板：占左半边 (0,0,430,520)，平时被滑块盖住 ==========
    auto *registerPanel = new QWidget(card);
    registerPanel->setObjectName("registerPanel");
    registerPanel->setGeometry(0, 0, 430, 520);

    auto *regLayout = new QVBoxLayout(registerPanel);
    regLayout->setContentsMargins(34, 26, 34, 24);
    regLayout->setSpacing(12);

    auto *regTitle = new QLabel(QStringLiteral("创建账号"), registerPanel);
    regTitle->setObjectName("titleLabel");
    regLayout->addWidget(regTitle, 0, Qt::AlignHCenter);
    regLayout->addSpacing(6);

    // 用户名
    registerFields_.usernameEdit_ = new QLineEdit(registerPanel);
    registerFields_.usernameEdit_->setObjectName("regUser");
    registerFields_.usernameEdit_->setPlaceholderText(QStringLiteral("用户名"));
    registerFields_.usernameEdit_->setFixedSize(320, 42);
    regLayout->addWidget(registerFields_.usernameEdit_, 0, Qt::AlignHCenter);

    // 邮箱
    registerFields_.emailEdit_ = new QLineEdit(registerPanel);
    registerFields_.emailEdit_->setObjectName("regEmail");
    registerFields_.emailEdit_->setPlaceholderText(QStringLiteral("邮箱"));
    registerFields_.emailEdit_->setFixedSize(320, 42);
    regLayout->addWidget(registerFields_.emailEdit_, 0, Qt::AlignHCenter);

    // 验证码行：验证码输入框 + 「获取验证码」同行
    auto *codeRow = new QWidget(registerPanel);
    auto *codeRowLayout = new QHBoxLayout(codeRow);
    codeRowLayout->setContentsMargins(0, 0, 0, 0);
    codeRowLayout->setSpacing(8);

    registerFields_.codeEdit_ = new QLineEdit(codeRow);
    registerFields_.codeEdit_->setObjectName("regCode");
    registerFields_.codeEdit_->setPlaceholderText(QStringLiteral("验证码"));
    registerFields_.codeEdit_->setMaxLength(6);

    registerFields_.codeBtn_ = new QPushButton(QStringLiteral("获取验证码"), codeRow);
    registerFields_.codeBtn_->setObjectName("btnCode");   // 真实发码逻辑见 onGetCodeClicked()
    registerFields_.codeBtn_->setCursor(Qt::PointingHandCursor);
    registerFields_.codeBtn_->setFixedSize(104, 42);
    codeRowLayout->addWidget(registerFields_.codeEdit_, 1);
    codeRowLayout->addWidget(registerFields_.codeBtn_, 0);
    codeRow->setFixedSize(320, 42);
    regLayout->addWidget(codeRow, 0, Qt::AlignHCenter);

    // 提示行：红字报错 / 绿字成功。固定 18px 高，出现/消失不上下顶动其它控件
    registerFields_.errTip_ = new QLabel(registerPanel);
    registerFields_.errTip_->setObjectName("regErrTip");
    registerFields_.errTip_->setAlignment(Qt::AlignCenter);
    registerFields_.errTip_->setFixedHeight(18);
    registerFields_.errTip_->setProperty("state", "ok");   // 初始态：空文本，绿字规则不显形
    regLayout->addWidget(registerFields_.errTip_, 0, Qt::AlignHCenter);

    // 密码 + 确认密码
    registerFields_.passwordEdit_ = new QLineEdit(registerPanel);
    registerFields_.passwordEdit_->setObjectName("regPass");
    registerFields_.passwordEdit_->setPlaceholderText(QStringLiteral("密码"));
    registerFields_.passwordEdit_->setEchoMode(QLineEdit::Password);   // 圆点显示
    registerFields_.passwordEdit_->setFixedSize(320, 42);
    regLayout->addWidget(registerFields_.passwordEdit_, 0, Qt::AlignHCenter);

    registerFields_.confirmEdit_ = new QLineEdit(registerPanel);
    registerFields_.confirmEdit_->setObjectName("regPass2");
    registerFields_.confirmEdit_->setPlaceholderText(QStringLiteral("确认密码"));
    registerFields_.confirmEdit_->setEchoMode(QLineEdit::Password);
    registerFields_.confirmEdit_->setFixedSize(320, 42);
    regLayout->addWidget(registerFields_.confirmEdit_, 0, Qt::AlignHCenter);

    regLayout->addSpacing(4);
    registerFields_.registerBtn_ = new QPushButton(QStringLiteral("注  册"), registerPanel);
    registerFields_.registerBtn_->setObjectName("btnRegister");
    registerFields_.registerBtn_->setCursor(Qt::PointingHandCursor);
    registerFields_.registerBtn_->setFixedSize(320, 44);
    regLayout->addWidget(registerFields_.registerBtn_, 0, Qt::AlignHCenter);

    registerFields_.toLoginLink_ = new QPushButton(QStringLiteral("已有账号？去登录"), registerPanel);
    registerFields_.toLoginLink_->setObjectName("textLink");
    registerFields_.toLoginLink_->setCursor(Qt::PointingHandCursor);
    regLayout->addWidget(registerFields_.toLoginLink_, 0, Qt::AlignHCenter);
    regLayout->addStretch(1);

    // ========== 5. 登录面板：占右半边 (430,0,430,520) ==========
    auto *loginPanel = new QWidget(card);
    loginPanel->setObjectName("loginPanel");
    loginPanel->setGeometry(430, 0, 430, 520);

    auto *loginLayout = new QVBoxLayout(loginPanel);
    loginLayout->setContentsMargins(34, 62, 34, 32);
    loginLayout->setSpacing(14);

    auto *loginTitle = new QLabel(QStringLiteral("账号登录"), loginPanel);
    loginTitle->setObjectName("titleLabel");
    loginLayout->addWidget(loginTitle, 0, Qt::AlignHCenter);
    loginLayout->addSpacing(10);

    loginFields_.accountEdit_ = new QLineEdit(loginPanel);
    loginFields_.accountEdit_->setObjectName("loginAccount");
    loginFields_.accountEdit_->setPlaceholderText(QStringLiteral("账号"));
    loginFields_.accountEdit_->setFixedSize(320, 42);
    loginLayout->addWidget(loginFields_.accountEdit_, 0, Qt::AlignHCenter);

    loginFields_.passwordEdit_ = new QLineEdit(loginPanel);
    loginFields_.passwordEdit_->setObjectName("loginPass");
    loginFields_.passwordEdit_->setPlaceholderText(QStringLiteral("密码"));
    loginFields_.passwordEdit_->setEchoMode(QLineEdit::Password);
    loginFields_.passwordEdit_->setFixedSize(320, 42);
    loginLayout->addWidget(loginFields_.passwordEdit_, 0, Qt::AlignHCenter);

    loginFields_.loginBtn_ = new QPushButton(QStringLiteral("登  录"), loginPanel);
    loginFields_.loginBtn_->setObjectName("btnLogin");
    loginFields_.loginBtn_->setCursor(Qt::PointingHandCursor);
    loginFields_.loginBtn_->setFixedSize(320, 44);
    loginLayout->addWidget(loginFields_.loginBtn_, 0, Qt::AlignHCenter);

    loginFields_.toRegisterLink_ = new QPushButton(QStringLiteral("没有账号？去注册"), loginPanel);
    loginFields_.toRegisterLink_->setObjectName("textLink");
    loginFields_.toRegisterLink_->setCursor(Qt::PointingHandCursor);
    loginLayout->addWidget(loginFields_.toRegisterLink_, 0, Qt::AlignHCenter);
    loginLayout->addStretch(1);

    // ========== 6. 滑块：浅粉渐变「身份卡」，盖住被遮挡的那一侧 ==========
    // 故意不加进任何布局，用 setGeometry 手动定位 —— 布局会自动重排，破坏叠层
    // 初始 x=0：盖住左半边注册表单 → 右侧登录表单可见
    slider_ = new QWidget(card);
    slider_->setObjectName("slider");
    slider_->setGeometry(0, 0, 430, 520);
    slider_->raise();

    auto *sliderLayout = new QVBoxLayout(slider_);
    sliderLayout->setContentsMargins(40, 30, 40, 28);
    sliderLayout->setSpacing(0);

    sliderLayout->addStretch(2);                       // 上留白

    sliderTitle_ = new QLabel(QStringLiteral("欢 迎 回 来"), slider_);
    sliderTitle_->setObjectName("sliderTitle");
    sliderTitle_->setAlignment(Qt::AlignCenter);
    sliderLayout->addWidget(sliderTitle_);
    sliderLayout->addSpacing(16);

    auto *avatar = makeRoundAvatar(slider_);           // 微信式：圆头像（TODO 换真实头像）
    sliderLayout->addWidget(avatar, 0, Qt::AlignHCenter);
    sliderLayout->addSpacing(16);

    sliderUser_ = new QLabel(kLastLoginUserName, slider_);   // 头像正下方 = 用户名
    sliderUser_->setObjectName("sliderUser");
    sliderUser_->setAlignment(Qt::AlignCenter);
    sliderLayout->addWidget(sliderUser_);
    sliderLayout->addSpacing(10);

    sliderDesc_ = new QLabel(QStringLiteral("老朋友，继续我们的对话吧"), slider_);
    sliderDesc_->setObjectName("sliderDesc");
    sliderDesc_->setAlignment(Qt::AlignCenter);
    sliderLayout->addWidget(sliderDesc_);

    sliderLayout->addStretch(3);                       // 中下留白

    // ========== 7. 所有 connect 统一放到 initConnect()（见文件底部），
    //            UI 创建函数里不再散落接线 ==========

    // ========== 8. 右上角关闭按钮（无边框窗口没有系统关闭钮，需自绘） ==========
    // 在 card 上最后创建并 raise()，保证无论滑块滑到哪一侧都能点中
    closeBtn_ = new QPushButton(QStringLiteral("✕"), card);
    closeBtn_->setObjectName("btnClose");
    closeBtn_->setCursor(Qt::PointingHandCursor);
    closeBtn_->setFixedSize(34, 34);
    closeBtn_->setGeometry(card->width() - 34 - 14, 14, 34, 34);
    closeBtn_->raise();
}

// 滑块动画：toRegister==true → 滑到右侧露出注册；false → 滑回左侧露出登录。
// 滑动同时翻转滑块上的 标题/用户名/副文 文案。
//
// 流畅度做法：滑块本体（渐变圆角 + 多层文字 + 头像）若每帧都 setGeometry，
// 每次都会整块重绘，在半透明窗口里会有可感知卡顿。所以滑动时只动一张
// grab() 截出的"快照层"(slideOverlay_)——每帧只是搬运一张位图，真实滑块
// 等动画落定后再一次性放到终点，视觉完全一致但开销低很多。
void LoginDialog::slideTo(bool toRegister)
{
    // 0) 上次滑动还没跑完就再次触发：把真实滑块复位到快照当前所在的位置
    if (slideOverlay_) {
        if (slideAnim_) {
            slideAnim_->stop();
            slideAnim_->deleteLater();
            slideAnim_ = nullptr;
        }
        slider_->setGeometry(slideOverlay_->geometry());
        slider_->show();
        slider_->raise();
        closeBtn_->raise();
        slideOverlay_->deleteLater();
        slideOverlay_ = nullptr;
    }

    const int targetX = toRegister ? 430 : 0;
    if (slider_->x() == targetX)
        return;                                        // 已停在目标位置，仅刷新文案

    // 1) 文案按目标侧即时翻转
    if (toRegister) {
        sliderTitle_->setText(QStringLiteral("加 入 HxnChat"));
        sliderUser_->setText(QStringLiteral("注册新账号"));
        sliderDesc_->setText(QStringLiteral("创建账号，开启新的对话"));
    } else {
        sliderTitle_->setText(QStringLiteral("欢 迎 回 来"));
        sliderUser_->setText(kLastLoginUserName);
        sliderDesc_->setText(QStringLiteral("老朋友，继续我们的对话吧"));
    }

    // 2) 截快照 → 隐藏真实滑块 → 用快照层来滑
    slider_->show();
    slider_->raise();
    const QPixmap shot = slider_->grab();          // 一次性离屏渲染，之后每帧只搬像素

    slideOverlay_ = new QLabel(slider_->parentWidget());   // 父级 = card
    slideOverlay_->setObjectName("slideOverlay");          // 无 QSS 命中，纯画图
    slideOverlay_->setPixmap(shot);
    slideOverlay_->setGeometry(slider_->geometry());
    slideOverlay_->show();
    slideOverlay_->raise();
    closeBtn_->raise();                            // 关闭按钮始终可点在最上层

    slider_->hide();

    // 3) 动画：把快照层从当前位置滑到 targetX
    slideAnim_ = new QPropertyAnimation(slideOverlay_, "geometry", this);
    slideAnim_->setDuration(400);
    slideAnim_->setStartValue(slideOverlay_->geometry());
    slideAnim_->setEndValue(QRect(targetX, 0, 430, 520));
    slideAnim_->setEasingCurve(QEasingCurve::InOutCubic);
    connect(slideAnim_, &QPropertyAnimation::finished, this, [this] {
        if (!slideOverlay_)
            return;                                // 已被新一轮 slideTo 接管
        slider_->setGeometry(slideOverlay_->geometry());   // 真实滑块落到终点
        slider_->show();
        slider_->raise();
        closeBtn_->raise();
        slideOverlay_->deleteLater();
        slideOverlay_ = nullptr;
        slideAnim_->deleteLater();
        slideAnim_ = nullptr;
    });
    slideAnim_->start();
}

void LoginDialog::initConnect()
{
    // —— 滑动切换：只由两侧表单里的链接触发 ——
    // 登录表单(右)「没有账号？去注册」→ 滑到右半 x=430，露出左侧注册表单
    connect(loginFields_.toRegisterLink_, &QPushButton::clicked, this,
            [this] { slideTo(true); });
    // 注册表单(左)「已有账号？去登录」→ 滑回左半 x=0，露出右侧登录表单
    connect(registerFields_.toLoginLink_, &QPushButton::clicked, this,
            [this] { slideTo(false); });

    // —— 无边框窗口的关闭按钮 ——
    connect(closeBtn_, &QPushButton::clicked, this, &QDialog::close);

    // —— 注册表单：获取验证码 ——
    connect(registerFields_.codeBtn_, &QPushButton::clicked, this,
            &LoginDialog::onGetCodeClicked);
}

void LoginDialog::showTip(const QString &str, bool isOk)
{
    registerFields_.errTip_->setText(str);
    // 同一个标签：err→红字规则，ok→绿字规则（见 QSS [state=...] 两段）
    registerFields_.errTip_->setProperty("state", isOk ? "ok" : "err");
    repolish(registerFields_.errTip_);   // 属性变了，必须强制 Qt 重算样式才会变色
}

void LoginDialog::onGetCodeClicked()
{
    // 读注册面板里「邮箱」输入框的内容（registerFields_.emailEdit_ 才是邮箱框）
    //text()返回的是QString,QString.trimmed()返回一个新字符串，把字符串开头、结尾的空白字符全部删掉。
    const QString email = registerFields_.emailEdit_
                              ? registerFields_.emailEdit_->text().trimmed()
                              : QString();

    if (email.isEmpty()) {
        showTip(tr("请输入邮箱"));
        return;
    }

    // 邮箱地址正则：^\w+(\.\w+)*@\w+(\.\w+)+$
    // 首尾加锚定，确保吃下整个字符串，避免 "乱写user@x.com乱写" 这种子串也通过
    QRegularExpression regex(R"(^\w+(\.\w+)*@\w+(\.\w+)+$)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(match){
        // TODO(下一步): 发送http请求获取验证码。成功回调里显示绿字并禁用按钮倒计时：
        //   showTip(tr("验证码已发送，请查收邮箱"), true);
        //   并把 codeBtn_ 切成 60 秒倒计时（期间不可再点）
    }else{
        showTip(tr("邮箱地址不正确"));
    }
}
