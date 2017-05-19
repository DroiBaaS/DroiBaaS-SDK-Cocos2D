/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */


#include <stdio.h>
#include "DroiRemoteServiceHelper.h"
#include "DroiRESTfulAPIDefinition.h"
#include "cJSON.h"
#include "cJSONHelper.h"


DroiSignUpBody* DroiSignUpBody::bodyFromType(std::string const& type, DroiObject* obj)
{
    DroiSignUpBody* body = new DroiSignUpBody();
    body->type = new RefValue(Value(type));
    body->obj = obj;

    return body;
}

RefMap* DroiSignUpBody::toJson()
{
    RefMap* rm = new RefMap();
    rm->insert("Data", obj);
    rm->insert("Type", type);
    rm->insert("InstallationId", RefPtrAutoReleaser<RefValue>(new RefValue(Value(DroiCore::getInstallationId()))));

    return rm;
}

DroiSignUpOutput* DroiSignUpOutput::fromJson(const std::string& jsonString )
{
    DroiSignUpOutput* output = new DroiSignUpOutput;
    RefPtrAutoReleaser<RefMap> jsonMap = dynamic_cast<RefMap*>(cJSONHelper::fromJSON(jsonString));
    if (jsonMap.get() == nullptr) return nullptr;
    
    RefMap::const_iterator it = jsonMap->find("Code");

    if ( it != jsonMap->end() ) {
        if (dynamic_cast<RefValue*>(it->second) != nullptr) {
            output->code = dynamic_cast<RefValue*>(it->second);
        } else {
            RefPtrAutoReleaser<RefValue> tmp = new RefValue(Value(-1));
            output->code = tmp;
        }
    }

    if (output->code->asInt() == 0 ) {
        it = jsonMap->find("Result");
        if ( it != jsonMap->end() ) {
            if (dynamic_cast<RefMap*>(it->second) != nullptr) {
                RefMap* resultMap = dynamic_cast<RefMap*>(it->second);
                RefMap::const_iterator it2 = resultMap->find("Token");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<RefValue*>(it2->second) != nullptr)
                        output->token =  dynamic_cast<RefValue* >(it2->second);
                }
                it2 = resultMap->find("ExpiredAt");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<RefValue*>(it2->second) != nullptr)
                        output->expireIn =  dynamic_cast<RefValue* >(it2->second);
                }
                it2 = resultMap->find("Token");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<RefValue*>(it2->second) != nullptr)
                        output->token=  dynamic_cast<RefValue* >(it2->second);
                }
                it2 = resultMap->find("Data");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<DroiObject*>(it2->second) != nullptr)
                        output->user = dynamic_cast<DroiObject* >(it2->second);
                }
            }
        }
    } else {
        it = jsonMap->find("Ticket");
        if ( it != jsonMap->end())
            if (dynamic_cast<RefValue*>(it->second) != nullptr)
                output->ticket =  dynamic_cast<RefValue* >(it->second);
    }

    return output;
}

RefValue* DroiSignUpOutput::getCode()
{
    return code;
}

DroiObject* DroiSignUpOutput::getUser()
{
    return user;
}

RefValue* DroiSignUpOutput::getToken()
{
    return token;
}

RefValue* DroiSignUpOutput::getTicket()
{
    return ticket;
}

RefValue* DroiSignUpOutput::getExpire()
{
    return expireIn;
}

DroiLoginOutput::DroiLoginOutput()
{
    code = token =  expireIn = ticket =  nullptr;
    user = nullptr;
}

DroiLoginBody* DroiLoginBody::bodyFromUserId(std::string const& userId, std::string const& password, std::string const& installationId) noexcept
{
    DroiLoginBody* body = new DroiLoginBody();
    body->type = new RefValue(Value("general"));
    body->installationId = new RefValue(Value(installationId));

    body->userId = new RefValue(Value(userId));
    body->password = new RefValue(Value(password));

    return body;
}

DroiLoginBody* DroiLoginBody::bodyFromAnonymousInstallationId(std::string const& installationId) noexcept
{
    DroiLoginBody* body = new DroiLoginBody();
    body->type = new RefValue(Value("anonymous"));
    body->type = new RefValue(Value("anonymous"));
    body->installationId = new RefValue(Value(installationId));
    body->userId = nullptr;
    body->password = nullptr;

    return body;
}

RefMap* DroiLoginBody::toJson()
{
    RefMap* rm = new RefMap();
    rm->insert("Type", type);
    rm->insert("InstallationId", installationId);
    if (userId != nullptr)
        rm->insert("UserId", userId);
    if (password != nullptr)
        rm->insert("Password", password);

    return rm;
}

DroiLoginOutput* DroiLoginOutput::fromJson(const std::string& jsonString)
{
    DroiLoginOutput* output = new DroiLoginOutput;
    RefPtrAutoReleaser<RefMap> jsonMap = dynamic_cast<RefMap*>(cJSONHelper::fromJSON(jsonString));
    if (jsonMap.get() == nullptr) return nullptr;
    RefMap::const_iterator it = jsonMap->find("Code");

    if ( it != jsonMap->end() ) {
        if (dynamic_cast<RefValue*>(it->second) != nullptr) {
            output->code = dynamic_cast<RefValue*>(it->second);
        } else {
            RefPtrAutoReleaser<RefValue> tmp = new RefValue(Value(-1));
            output->code = tmp;
        }
    }

    if (output->code->asInt() == 0 ) {
        it = jsonMap->find("Result");
        if ( it != jsonMap->end() ) {
            if (dynamic_cast<RefMap*>(it->second) != nullptr) {
                RefMap* resultMap = dynamic_cast<RefMap*>(it->second);
                RefMap::const_iterator it2 = resultMap->find("token");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<RefValue*>(it2->second) != nullptr)
                        output->token =  dynamic_cast<RefValue* >(it2->second);
                }
                it2 = resultMap->find("ExpiredAt");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<RefValue*>(it2->second) != nullptr)
                        output->expireIn=  dynamic_cast<RefValue* >(it2->second);
                }
                it2 = resultMap->find("Token");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<RefValue*>(it2->second) != nullptr)
                        output->token=  dynamic_cast<RefValue* >(it2->second);
                }
                it2 = resultMap->find("Data");
                if ( it2 != resultMap->end()) {
                    if (dynamic_cast<DroiObject*>(it2->second) != nullptr)
                        output->user = dynamic_cast<DroiObject* >(it2->second);
                }
            }
        }
    } else {
        it = jsonMap->find("Ticket");
        if ( it != jsonMap->end())
            if (dynamic_cast<RefValue*>(it->second) != nullptr)
                output->ticket =  dynamic_cast<RefValue* >(it->second);
    }

    return output;
}

RefValue* DroiLoginOutput::getCode()
{
    return code;
}

DroiObject* DroiLoginOutput::getUser()
{
    return user;
}

RefValue* DroiLoginOutput::getToken()
{
    return token;
}

RefValue* DroiLoginOutput::getTicket()
{
    return ticket;
}

RefValue* DroiLoginOutput::getExpire()
{
    return expireIn;
}

bool DroiLoginOutput::isSuccessful()
{
    std::chrono::milliseconds expire(0);
    if (expireIn != nullptr)
        DroiDateTime::isValidISO8601String(expireIn->asString().c_str(), expire);
    return code->asInt() == 0 && token->asString().length() > 0 && user != nullptr && expire > DroiDateTime::now();
}

DroiLogoutBody* DroiLogoutBody::bodyFromToken(std::string const& token, std::string const& _Id) noexcept
{
    DroiLogoutBody* body = new DroiLogoutBody();
    body->token = new RefValue(Value(token));
    body->_Id = new RefValue(Value(_Id));

    return body;
}

RefMap* DroiLogoutBody::toJson()
{
    RefMap* rm = new RefMap();
    rm->insert("UserObjectId", _Id);
    rm->insert("Token", token);

    return rm;
}

DroiLogoutOutput* DroiLogoutOutput::fromJson(const std::string& jsonString)
{
    DroiLogoutOutput* output = new DroiLogoutOutput;
    RefPtrAutoReleaser<RefMap> jsonMap = dynamic_cast<RefMap*>(cJSONHelper::fromJSON(jsonString));
    if (jsonMap.get() == nullptr) return nullptr;

    RefMap::const_iterator it = jsonMap->find("Code");

    if ( it != jsonMap->end() ) {
        if (dynamic_cast<RefValue*>(it->second) != nullptr) {
            output->code = dynamic_cast<RefValue*>(it->second);
        } else {
            RefPtrAutoReleaser<RefValue> tmp = new RefValue(Value(-1));
            output->code = tmp;
        }
    }

    if (output->code->asInt() != 0 ) {
        it = jsonMap->find("Ticket");
        if ( it != jsonMap->end())
            if (dynamic_cast<RefValue*>(it->second) != nullptr)
                output->ticket = dynamic_cast<RefValue* >(it->second);
    }

    return output;
}

DroiRestfulOutput* DroiRestfulOutput::fromJson(const std::string& jsonString)
{
    DroiRestfulOutput* output = new DroiRestfulOutput;
    RefPtrAutoReleaser<RefMap> jsonMap = dynamic_cast<RefMap*>(cJSONHelper::fromJSON(jsonString));
    if (jsonMap.get() == nullptr) return nullptr;

    RefMap::const_iterator it = jsonMap->find("Code");
    
    if ( it != jsonMap->end() ) {
        if (dynamic_cast<RefValue*>(it->second) != nullptr) {
            output->code = dynamic_cast<RefValue*>(it->second);
        } else {
            RefPtrAutoReleaser<RefValue> tmp = new RefValue(Value(-1));
            output->code = tmp;
        }
    }
    
    if (output->code->asInt() != 0 ) {
        it = jsonMap->find("Ticket");
        if ( it != jsonMap->end())
            if (dynamic_cast<RefValue*>(it->second) != nullptr)
                output->ticket = dynamic_cast<RefValue* >(it->second);
    } else {
        it = jsonMap->find("Result");
        if ( it != jsonMap->end())
            if (dynamic_cast<RefMap*>(it->second) != nullptr)
                output->result = dynamic_cast<RefMap* >(it->second);
    }
    
    return output;
}

RefValue* DroiRestfulOutput::getCode()
{
    return code;
}


RefValue* DroiRestfulOutput::getTicket()
{
    return ticket;
}

RefMap* DroiRestfulOutput::getResult()
{
    return result;
}

std::string DroiRemoteServiceHelper::callServer(const std::string& service,  cocos2d::network::HttpRequest::Type method, Ref* payload, RefMap* additionalHeaders, DroiError* err)
{
    DroiHttp* droihttp = DroiHttp::instance();
    cJSON* jsondata = nullptr;
    std::string tmp = DroiCore::DroiEntry + service;
    std::string data;

    if (dynamic_cast<RefVector*>(payload) != nullptr) {
        jsondata = cJSONHelper::toJSONArray(dynamic_cast<RefVector*>(payload));
    } else if (dynamic_cast<RefMap*>(payload) != nullptr) {
        jsondata = cJSONHelper::toJSONMap(dynamic_cast<RefMap*>(payload));
    } else if (dynamic_cast<RefValue*>(payload) != nullptr) {
        data = (dynamic_cast<RefValue*>(payload))->asString();
    }

    if (jsondata != nullptr) {
        char* res = cJSON_PrintUnformatted(jsondata);
        data = res;
        free(res);
        cJSON_Delete(jsondata);
    }

    RefPtrAutoReleaser<DroiHttpRequest> httpRequest = DroiHttpRequest::requestWithService(tmp, data, method);
    //Add additional headers
    DroiRemoteServiceHelper::appendDefaultHeaderToRequest(httpRequest);
    if (additionalHeaders != nullptr) {
        RefMap::const_iterator iter = additionalHeaders->begin();
        for (; iter != additionalHeaders->end(); ++iter ) {
            Ref* pValue = iter->second;
            if ( dynamic_cast<RefValue*>(pValue) != NULL ) {
                RefValue* val = dynamic_cast<RefValue*>(pValue);
                httpRequest->addHeader(iter->first, val->asString());
            }
        }
    }

    RefPtrAutoReleaser<DroiHttpResponse> resp = droihttp->sendRequest(httpRequest, err);
    DroiError error = DroiRemoteServiceHelper::translateResponseError(resp);
    if (err != nullptr) *err = error;

    return resp->getData();
}

std::string DroiRemoteServiceHelper::callServer(const std::string& service,  cocos2d::network::HttpRequest::Type method, Ref* payload, DroiError* err)
{
    return callServer(service, method, payload, nullptr, err);
}

DroiError DroiRemoteServiceHelper::translateResponseError(DroiHttpResponse* response)
{
    DroiErrorCode errorCode = DroiErrorCode::DROICODE_UNKNOWN_ERROR;
    std::string appendedMessage;
    int32_t httpCode = response->getHttpCode();
    std::string logMsg;
    logMsg.append("HttpStatus: ").append(std::to_string(httpCode));

    if (httpCode == 200) {
        errorCode = DroiErrorCode::DROICODE_OK;
    } else if (httpCode != 0 && httpCode != 200) {
        if (httpCode == 404) {
            errorCode = DroiErrorCode::DROICODE_SERVICE_NOT_FOUND;
        } else if (httpCode == 403 || httpCode == 405) {
            errorCode = DroiErrorCode:: DROICODE_SERVICE_NOT_ALLOWED;
        } else {
            errorCode = DroiErrorCode::DROICODE_HTTP_SERVER_ERROR;
            appendedMessage.append("HttpStatus ").append(std::to_string(httpCode));
        }
    }

    return DroiError::createDroiError(errorCode, response->getTicket(), appendedMessage);
}

void DroiRemoteServiceHelper::appendDefaultHeaderToRequest(DroiHttpRequest* request)
{
    request->addHeader(DROI_KEY_APP_ID, DroiCore::getAppId());
    request->addHeader(DROI_KEY_DEVICE_ID, DroiCore::getDeviceId());
    RefPtrAutoReleaser<DroiUser> user = DroiUser::getCurrentUser();
    string token = "";

    if ( user != nullptr && user->isLoggedIn() )
        token = user->getSessionToken();
    if ( token.length() > 0 )
        request->addHeader(DROI_SESSION_TOKEN, token);

}

DroiSignUpOutput* DroiRemoteServiceHelper::signUp(DroiSignUpBody* body, DroiError* err)
{
    RefPtrAutoReleaser<RefMap> ref = body->toJson();
    if (ref == nullptr) {
        if (err != nullptr) *err = DroiError::createDroiError(DROICODE_ERROR, "", "gen data fail.");
    }

    return DroiSignUpOutput::fromJson(DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_SIGNUP, cocos2d::network::HttpRequest::Type::POST, ref, err));
}

DroiLoginOutput* DroiRemoteServiceHelper::login(DroiLoginBody* body, DroiError* err)
{
    RefPtrAutoReleaser<RefMap> ref = body->toJson();
    if (ref.get() == NULL) {
        if (err != nullptr) *err = DroiError::createDroiError(DROICODE_ERROR, "", "gen data fail.");
    }

    return DroiLoginOutput::fromJson(DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_LOGIN, cocos2d::network::HttpRequest::Type::POST, ref, err));
}

DroiLogoutOutput* DroiRemoteServiceHelper::logout(DroiLogoutBody* body, DroiError* err)
{
    RefPtrAutoReleaser<RefMap> ref = body->toJson();
    if (ref.get() == NULL) {
        if (err != nullptr) *err = DroiError::createDroiError(DROICODE_ERROR, "", "gen data fail.");
    }

    return DroiLogoutOutput::fromJson(DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_LOGOUT, cocos2d::network::HttpRequest::Type::POST, ref, err));
}

DroiRestfulOutput* DroiRemoteServiceHelper::changePassword(const std::string& oldPassword, const std::string newPassword, const std::string& token, DroiError* error)
{
    if (oldPassword.length() == 0 || newPassword.length() == 0) {
        if (error != nullptr)  *error = DroiError::createDroiError(DROICODE_ERROR, "", "Either password is empty");
        return nullptr;
    }
    
    RefPtrAutoReleaser<RefMap> ref = new RefMap;
    RefPtrAutoReleaser<RefValue> refOld = new RefValue(Value(oldPassword));
    RefPtrAutoReleaser<RefValue> refNew = new RefValue(Value(newPassword));
    RefPtrAutoReleaser<RefValue> refToken = new RefValue(Value(token));
    ref->insert("Old", refOld);
    ref->insert("New", refNew);
    ref->insert("Token", refToken);
    
    return DroiRestfulOutput::fromJson( DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_RESET_PWD, cocos2d::network::HttpRequest::Type::PUT, ref, error) );
}

DroiRestfulOutput* DroiRemoteServiceHelper::validateEmailWithToken(const std::string& token, DroiError* error)
{
    RefPtrAutoReleaser<RefMap> ref = new RefMap;
    RefPtrAutoReleaser<RefValue> refToken = new RefValue(Value(token));
    ref->insert("Token", refToken);

    DroiError err;
    std::string outData = DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_VALIDATE_EMAIL, cocos2d::network::HttpRequest::Type::POST, ref, &err);
    if (!err.isOk()) {
        if (error != nullptr) *error = err;
        return nullptr;
    }

    DroiRestfulOutput* output = DroiRestfulOutput::fromJson(outData);
    if (output == nullptr) {
        if (error != nullptr) *error = DroiError::createDroiError(DROICODE_ERROR, "", "generator json fail.");
    }

    return output;
}

DroiRestfulOutput* DroiRemoteServiceHelper::validatePhoneNumWithToken(const std::string& token, DroiError* error)
{
    RefPtrAutoReleaser<RefMap> ref = new RefMap;
    RefPtrAutoReleaser<RefValue> refToken = new RefValue(Value(token));
    ref->insert("Token", refToken);

    DroiError err;
    std::string outData = DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_VALIDATE_SMS, cocos2d::network::HttpRequest::Type::POST, ref, &err);
    if (!err.isOk()) {
        if (error != nullptr) *error = err;
        return nullptr;
    }

    DroiRestfulOutput* output = DroiRestfulOutput::fromJson(outData);
    if (output == nullptr) {
        if (error != nullptr) *error = DroiError::createDroiError(DROICODE_ERROR, "", "generator json fail.");
    }

    return output;
}

DroiRestfulOutput* DroiRemoteServiceHelper::confirmPhoneNumPinCodeWithToken(const std::string& token, const std::string& pinCode, DroiError* error)
{
    RefPtrAutoReleaser<RefMap> ref = new RefMap;
    RefPtrAutoReleaser<RefValue> refToken = new RefValue(Value(token));
    RefPtrAutoReleaser<RefValue> refPinCode = new RefValue(Value(pinCode));

    ref->insert("Token", refToken);
    ref->insert("PinCode", refPinCode);

    DroiError err;
    std::string outData = DroiRemoteServiceHelper::callServer(DroiRESTfulAPIDefinition::USER_VALIDATE_SMS_CHECK, cocos2d::network::HttpRequest::Type::POST, ref, &err);
    if (!err.isOk()) {
        if (error != nullptr) *error = err;
        return nullptr;
    }

    DroiRestfulOutput* output = DroiRestfulOutput::fromJson(outData);
    if (output == nullptr) {
        if (error != nullptr) *error = DroiError::createDroiError(DROICODE_ERROR, "", "generator json fail.");
    }

    return output;
}

