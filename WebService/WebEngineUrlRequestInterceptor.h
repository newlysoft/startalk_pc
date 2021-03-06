//
// Created by cc on 2020/3/31.
//

#ifndef QTALK_V2_WEBENGINEURLREQUESTINTERCEPTOR_H
#define QTALK_V2_WEBENGINEURLREQUESTINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>
#include <QObject>

class WebView;
class WebEngineUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor{
    Q_OBJECT
public:
    explicit WebEngineUrlRequestInterceptor(WebView* webView, QObject *parent = Q_NULLPTR);
    void interceptRequest(QWebEngineUrlRequestInfo &info);

private:
    WebView* pWebview{};
};


#endif //QTALK_V2_WEBENGINEURLREQUESTINTERCEPTOR_H
