#include "httpmgr.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <spdlog/spdlog.h>

//req_id请求编号用来区分是哪一次请求返回了
//mod区分业务模块
void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    //创建一个HTTP POST请求，并设置请求头和请求体
    //把 QJsonObject 序列化成 JSON 字符串字节流，返回 QByteArray
    QByteArray data = QJsonDocument(json).toJson();
    //通过url构造请求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    //封装响应.post发起异步post请求，QNetworkReply*：就是这次请求对应的响应对象
    //当请求结束（收到服务器响应 / 网络出错），reply对象自动 emit finished 信号；
    //HttpMgr 是静态单例，生命周期=整个程序，直接捕获 this 即可（勿用 shared_from_this）
    QNetworkReply * reply = _manager.post(request, data);
    //设置信号和槽等待发送完成
    QObject::connect(reply, &QNetworkReply::finished, [reply, this, req_id, mod](){
        //处理错误的情况
        if(reply->error() != QNetworkReply::NoError){
            // fmt 没有枚举的默认格式化器，req_id/mod 需转成 int 才能打进日志
            spdlog::error("network error: {},请求id为:{},模块为:{}",
                          reply->errorString().toStdString(),
                          static_cast<int>(req_id), static_cast<int>(mod));
            //emit 是 Qt 的宏，专门用来发射信号。
            emit this->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);
            //等当前这个事件处理完之后，再delete 这个 QObject 对象
            reply->deleteLater();
            return;
        }

        //无错误则读回请求
        //readAll() 读取的是响应体（body），不是 HTTP 状态行、响应头。
        QString res = reply->readAll();

        //发送信号通知完成
        emit this->sig_http_finish(req_id, res, ErrorCodes::SUCCESS,mod);
        spdlog::debug("Post请求发送完成，收到回包,接下来分析回包");
        reply->deleteLater();
        return;
    });
}

HttpMgr::HttpMgr() {

     connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod == Modules::REGISTERMOD){
        //发送信号通知指定模块http响应结束
        emit sig_reg_mod_finish(id, res, err);
    }

}



HttpMgr::~HttpMgr() {}
