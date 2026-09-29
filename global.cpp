#include "global.h"


//C++ 不同翻译单元（不同 cpp）之间全局动态初始化顺序是未定义的。
//创建一个lambda去初始化repolish
//其实就是个重新渲染
std::function<void(QWidget*)> repolish =[](QWidget *w){
    w->style()->unpolish(w);//卸载旧的样式
    w->style()->polish(w);//换上新样式
};

// 定义放 cpp，头文件只写 extern，否则多个 cpp 包含头文件会重复定义
QString gate_url_prefix = QStringLiteral("http://localhost:8000");
