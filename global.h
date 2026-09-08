#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
//extern声明看你的笔记去
//repolish是一个变量、能被返回值为void接收参数为QWidget*的可调用对象给赋值
extern std::function<void(QWidget*)> repolish;

#endif // GLOBAL_H
