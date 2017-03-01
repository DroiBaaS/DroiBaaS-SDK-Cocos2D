/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */


#ifndef DroiRemoteServiceHelper_h
#define DroiRemoteServiceHelper_h

#include "RefValue.h"
#include "cJSON.h"
#include "cJSONHelper.h"
#include "DroiCore.h"
#include "DroiHttp.h"
#include "DroiObject.h"
#include "DroiError.h"
#include "DroiDateTime.h"

class DroiSignUpBody : public Ref
{
public:
    static DroiSignUpBody* bodyFromType(std::string const& type, DroiObject* obj);
    RefMap* toJson();

private:
    RefPtrAutoReleaser<RefValue> type;
    RefPtr<DroiObject> obj;
    DroiSignUpBody() {};
    ~DroiSignUpBody(){};
};

class DroiSignUpOutput : public Ref
{
public:
    static DroiSignUpOutput* fromJson(const std::string& jsonString);
    RefValue* getCode();
    DroiObject* getUser();
    RefValue* getToken();
    RefValue* getTicket();
    RefValue* getExpire();

private:
    RefPtr<RefValue> code;
    RefPtr<DroiObject> user;
    RefPtr<RefValue> token;
    RefPtr<RefValue> ticket;
    RefPtr<RefValue> expireIn;
};

class DroiLoginBody : public Ref
{
public:
    static DroiLoginBody* bodyFromUserId(std::string const& userId, std::string const& password, std::string const& installationId) noexcept;
    static DroiLoginBody* bodyFromAnonymousInstallationId(std::string const& installationId) noexcept;
    RefMap* toJson();

private:
    RefPtrAutoReleaser<RefValue>type;
    RefPtrAutoReleaser<RefValue> installationId;
    RefPtrAutoReleaser<RefValue> userId;
    RefPtrAutoReleaser<RefValue> password;
    DroiLoginBody() {};
};

class DroiLoginOutput : public Ref
{
public:
    static DroiLoginOutput* fromJson(const std::string& jsonString);
    //getter
    RefValue* getCode();
    DroiObject* getUser();
    RefValue* getToken();
    RefValue* getTicket();
    RefValue* getExpire();
    bool isSuccessful();

private:
    DroiLoginOutput();
    RefPtr<RefValue> code;
    RefPtr<DroiObject> user;
    RefPtr<RefValue> token;
    RefPtr<RefValue> ticket;
    RefPtr<RefValue> expireIn;
};

class DroiLogoutBody : public Ref
{
public:
    static DroiLogoutBody* bodyFromToken(std::string const& token, std::string const& _Id) noexcept;
    RefMap* toJson();

private:
    RefPtrAutoReleaser<RefValue> token;
    RefPtrAutoReleaser<RefValue> _Id;
    DroiLogoutBody() {};
};

class DroiLogoutOutput : public Ref
{
public:
    static DroiLogoutOutput* fromJson(const std::string& jsonString);
    RefValue* getCode();
    RefValue* getTicket();

private:
    RefPtr<RefValue> code;
    RefPtr<RefValue> ticket;
};


class DroiRestfulOutput : public Ref
{
public:
    static DroiRestfulOutput* fromJson(const std::string& jsonString);
    RefValue* getCode();
    RefValue* getResult();
    RefValue* getTicket();
    
private:
    RefPtr<RefValue> code;
    RefPtr<RefValue> ticket;
    RefPtr<RefValue> result;
    
};

class DroiRestfulArrayOutput : public Ref
{
public:
    static DroiRestfulArrayOutput* fromJson(const std::string& jsonString);
    RefValue* getCode();
    RefValue* getResult();
    RefValue* getTicket();
    RefValue* getCount();
    
private:
    RefPtr<RefValue> code;
    RefPtr<RefValue> ticket;
    RefPtr<RefValue> result;
    RefPtr<RefValue> count;
    
};

class DroiRemoteServiceHelper
{

public:
    static std::string callServer(const std::string& service, cocos2d::network::HttpRequest::Type method, Ref* payload, DroiError* err);
    static DroiError translateResponseError(DroiHttpResponse* resp);
    static void appendDefaultHeaderToRequest(DroiHttpRequest* request);
    static DroiSignUpOutput* signUp(DroiSignUpBody* body, DroiError* err);
    static DroiLoginOutput* login(DroiLoginBody* body, DroiError* err);
    static DroiLogoutOutput* logout(DroiLogoutBody* body, DroiError* err);
    static DroiRestfulOutput* changePassword(const std::string& oldPassword, const std::string newPassword, const std::string& token, DroiError* err);
    static DroiRestfulOutput* validateEmailWithToken(const std::string& token, DroiError* err);
    static DroiRestfulOutput* validatePhoneNumWithToken(const std::string& token, DroiError* err);
    static DroiRestfulOutput* confirmPhoneNumPinCodeWithToken(const std::string& token, const std::string& pinCode, DroiError* err);
};

#endif /* DroiRemoteServiceHelper_h */
