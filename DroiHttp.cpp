/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiHttp.h"
#include "DroiRESTfulAPIDefinition.h"
#include "DroiHelper.h"
#include "cJSONHelper.h"
#include <algorithm>

std::string HTTPQueueThread = "DroiHttpQueue";
std::string HTTPTag = "DroiHttp";
DroiHttp* DroiHttp::mInstance = nullptr;

// DroiHttpResponse
std::string DroiHttpResponse::getData()
{
    return _data;
}

long DroiHttpResponse::getHttpCode()
{
    return _httpcode;
}

std::string DroiHttpResponse::getHttpError()
{
    return _http_error;
}

std::string DroiHttpResponse::getTicket()
{
    return _ticket;
}

// DroiHttpRequest
DroiHttpRequest* DroiHttpRequest::requestWithService(const std::string& service, const std::string& data, cocos2d::network::HttpRequest::Type type)
{
    DroiHttpRequest* req = new DroiHttpRequest;
    req->service = service;
    req->data = data;
    req->type = type;

    return req;
}

void DroiHttpRequest::addHeader(const std::string& key, const std::string&  value)
{
    std::string tmp;
    tmp.append(key).append(": ").append(value);
    headers.push_back(tmp);
}

// DroiHttp
DroiHttp* DroiHttp::instance()
{
    if (mInstance == nullptr) {
        mInstance = new DroiHttp;
    }

    return mInstance;
}

DroiHttp::DroiHttp()
{
    requestList = new RefMap();
    initResponseCallbacks();
}

void DroiHttp::initResponseCallbacks() {
    cb = [=] (cocos2d::network::HttpClient* client, cocos2d::network::HttpResponse* response) {
        if (!response) {
            log("no reponse output");
            return;
        }

        // Get response by tag
        auto reqIt = requestList->find(string(response->getHttpRequest()->getTag()));
        if (reqIt == requestList->end()) {
            log("Wired, drop abnormal response");
            return;
        }
        DroiHttpResponse* res = dynamic_cast<DroiHttpResponse*>(reqIt->second);
        std::condition_variable* cond = res->cond;

        // You can get original request type from: response->request->reqType
        //        if (0 != strlen(response->getHttpRequest()->getTag())) {
        //            log("%s completed", response->getHttpRequest()->getTag());
        //        }

        res->_httpcode = response->getResponseCode();
//        log("Http code: %ld", response->getResponseCode());

        if (!response->isSucceed()) {
            log("response failed");
            log("error buffer: %s", response->getErrorBuffer());
            res->_http_error = response->getErrorBuffer();
            std::unique_lock<std::mutex> lock(*res->locker);
            cond->notify_all();
            return;
        }

        // Retrive data
        std::vector<char>* buffer = response->getResponseData();
        res->_data = std::string(buffer->begin(), buffer->end());
//        log("Response: %s", res->_data.c_str());
        buffer = response->getResponseHeader();
        RefPtrAutoReleaser<RefMap> headers = parseHeaders(std::string(buffer->begin(), buffer->end()));
        RefMap::iterator it = headers->find("X-Droi-ReqID");
        if (it!=headers->end())
            res->_ticket = dynamic_cast<RefValue*>(it->second)->asString();

        std::unique_lock<std::mutex> lock(*res->locker);
        cond->notify_all();
    };
}

std::string DroiHttp::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}

RefMap* DroiHttp::parseHeaders(const std::string& headers)
{
    RefMap* map = new RefMap();

    std::istringstream resp(headers.c_str());
    std::string header;
    std::string::size_type index;
    while (std::getline(resp, header) && header != "\r") {
        index = header.find(':', 0);
        if(index != std::string::npos) {
            RefPtrAutoReleaser<RefValue> val = new RefValue( Value(trim(header.substr(index + 1))) );
            map->insert(trim(header.substr(0, index)),
                        val );
        }
    }

    return map;
}

DroiHttpResponse* DroiHttp::sendRequest(DroiHttpRequest* req, DroiError* err)
{
    if (req == nullptr) {
        *err = DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "");
        log("request is null");
        return nullptr;
    }

    // Setup tag for pairing.
    std::condition_variable csCondition;
    std::mutex csLocker;
    std::string tag = DroiHelper::generateUUID();
    DroiHttpResponse* response = new DroiHttpResponse();
    req->tag = tag;
    response->tag = tag;
    response->cond = &csCondition;
    response->locker = &csLocker;
    requestList->insert(tag, response);
    
    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher(HTTPQueueThread);
    std::unique_lock<std::mutex> lock(csLocker);
    td.enqueueTask([this, req, &err] {
        requestImpl( req, err );
    });

    csCondition.wait(lock);
    response->cond = nullptr;
    response->locker = nullptr;
    
    // Clean up
    auto it = requestList->find(tag);
    if ( it != requestList->end() )
        requestList->erase(it);
    else {
        CCLOG("Cannot find tag in requestList" );
    }

    return response;
}

void DroiHttp::requestImpl(DroiHttpRequest* req, DroiError* err)
{
    RefPtrAutoReleaser<cocos2d::network::HttpRequest> request = new (std :: nothrow) cocos2d::network::HttpRequest();
    request->setUrl(req->service);
    request->setHeaders(req->headers);
    request->setRequestType(req->type);

    switch (req->type) {
        case cocos2d::network::HttpRequest::Type::PUT:
        case cocos2d::network::HttpRequest::Type::POST:
            request->setRequestData(req->data.c_str(), req->data.size());
//            log("outbody: %s", req->data.c_str());
            break;
        default:
            break;
    }
    request->setResponseCallback(cb);
    request->setTag(req->tag);
    cocos2d::network::HttpClient::getInstance()->send(request);

}

std::string deviceIdToString(long long high, long long low)
{
    if ( high == 0 && low == 0 )
        return "";
    char tmp[36];
    sprintf(tmp, "%016llX%016llX", high, low);
    std::string res(tmp);
    res.insert(8, "-");
    res.insert(13, "-");
    res.insert(16, "-");
    res.insert(23, "-");

    return res;
}

DeviceId DroiHttp::getDeviceId()
{
    DroiError err = DroiError::createDroiError(DROICODE_OK, "", "");
    std::string tmp(DroiCore::DroiEntry);
    tmp.append(DroiRESTfulAPIDefinition::REG_DEVICE);
    RefPtrAutoReleaser<DroiHttpRequest> request = DroiHttpRequest::requestWithService(tmp, "", cocos2d::network::HttpRequest::Type::GET);
    if ( request.get() == nullptr )
        return "";

    RefPtrAutoReleaser<DroiHttpResponse> resp = this->sendRequest(request, &err);
    RefPtrAutoReleaser<RefVector> map = cJSONHelper::fromJSONToVector(resp->getData());
    if ( map.get() == nullptr || map->size() < 2 )
        return "";
    long long high = atoll(dynamic_cast<RefValue*>(map->at(0))->asString().c_str());
    long long low = atoll(dynamic_cast<RefValue*>(map->at(1))->asString().c_str());

    return deviceIdToString(high, low);
}

