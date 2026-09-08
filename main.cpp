#include "mainwindow.h"
#include "global.h"
#include <QApplication>
#include "logindialog.h"
#include<QWidget>
/******************************************************************************
 *
 * @file       main.cpp
 * @brief      XXXX Function
 *
 * @author     小火锅
 * @date       2026/09/06 13:34:03
 * @history
 *****************************************************************************/
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    LoginDialog loginDialog;
    loginDialog.show();
    return a.exec();
}
