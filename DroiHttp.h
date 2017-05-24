/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef DroiHttp_h
#define DroiHttp_h

#include <stdio.h>
#include <mutex>
#include "network/HttpClient.h"
#include "network/HttpRequest.h"

#include "RefValue.h"
#include "DroiError.h"
#include "DroiTaskDispatcher.h"
#include "DroiCore.h"

#define DROI_KEY_APP_ID "X-Droi-AppID"
#define DROI_KEY_API_KEY "X-Droi-Api-Key"
#define DROI_KEY_PLATFORM_ID "X-Droi-Platform-Key"
#define DROI_KEY_DEVICE_ID "X-Droi-DeviceID"
#define DROI_SESSION_TOKEN "X-Droi-Session-Token"

typedef std::string DeviceId;

class DroiHttpResponse : public Ref
{
public:
    std::string getData();
    long getHttpCode();
    std::string getTicket();
    std::string getHttpError();

private:
    friend class DroiHttp;
    long _httpcode;
    std::string _http_error;
    std::string _data;
    std::string _ticket;
    std::string tag;
    std::condition_variable* cond;
    std::mutex* locker;
};



class DroiHttpRequest : public Ref
{
public:
    static DroiHttpRequest* requestWithService(const std::string& service, const std::string& data, cocos2d::network::HttpRequest::Type type);
    void addHeader(const std::string& key, const std::string& value);
    cocos2d::network::ccHttpRequestCallback responseCallback;

private:
    friend class DroiHttp;
//    void initResponseCallbacks();
    DroiHttpRequest() {};
    std::string service;
    std::string data;
    cocos2d::network::HttpRequest::Type type;
    std::vector<std::string> headers;
    std::string tag;
};

class DroiHttp : public Ref
{
public:
    static DroiHttp* instance();

    DeviceId getDeviceId();
    DroiHttpResponse* sendRequest(DroiHttpRequest* req, DroiError* err);
    cocos2d::network::ccHttpRequestCallback cb;

private:
    void requestImpl(DroiHttpRequest* req, DroiError* err);
    std::string trim(const std::string& str);
    RefMap* parseHeaders(const std::string& str);
    void initResponseCallbacks();
    DroiHttp();

    static DroiHttp* mInstance;
    RefPtrAutoReleaser<RefMap> requestList;
    bool isCallback;
    uint32_t times;
};

#endif /* DroiHttp_h */
