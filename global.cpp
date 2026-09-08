#include "global.h"


//C++ 不同翻译单元（不同 cpp）之间全局动态初始化顺序是未定义的。
//创建一个lambda去初始化repolish
std::function<void(QWidget*)> repolish =[](QWidget *w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};
