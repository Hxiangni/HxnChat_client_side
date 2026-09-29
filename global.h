#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
//extern声明看你的笔记去
//repolish是一个变量、能被返回值为void接收参数为QWidget*的可调用对象给赋值
//重新设置QT显示的样式颜色
extern std::function<void(QWidget*)> repolish;
// 定义放 cpp!!!!!!!!!!!!!!!
extern QString gate_url_prefix;


enum ReqId{
    ID_GET_VARIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
};
enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1, //Json解析失败
    ERR_NETWORK = 2,
};
enum Modules{//模块
    REGISTERMOD = 0,
};
#endif // GLOBAL_H
