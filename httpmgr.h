#ifndef HTTPMGR_H
#define HTTPMGR_H

#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include "global.h"
#include <QJsonObject>
#include <QJsonDocument>
// 注意：HttpMgr 是静态单例（SingLeton 里 static 对象），不归 shared_ptr 管，
// 所以不能继承 enable_shared_from_this、也不能调 shared_from_this()（会抛 bad_weak_ptr）。
class HttpMgr:public QObject, public Singleton<HttpMgr>
{
    Q_OBJECT

public:
    ~HttpMgr();
    // 发送 http post 请求（异步，结果通过 sig_http_finish 信号回调）
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
private:
    //设置父类为友元，父类可以调用自己的私有方法
    //因为在父类中会new自己
    friend class Singleton<HttpMgr>;
    HttpMgr();
    //这里为什么不用指针呢？因为httpMgr是一个单例类让成员随着类一起消亡
    QNetworkAccessManager _manager;


signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);


public slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
   // void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
