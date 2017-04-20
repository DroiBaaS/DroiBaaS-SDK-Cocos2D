/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include <base/CCUserDefault.h>
#include "DroiUser.h"
#include "DroiCore.h"
#include "DroiError.h"
#include "DroiRemoteServiceHelper.h"
#include "DroiPermission.h"
#include "DroiQuery.h"
#include "DroiDefinition.h"
#include "DroiHelper.h"

using namespace std;
#define KEY_ANONYMOUS "anonymous"
#define KEY_SESSION "session"

RefPtr<DroiUser> DroiUser::g_currentUser = nullptr;

bool DroiUser::mAutoAnonymousUserEnabled = true;

DroiUser::DroiUser()
    : DroiObject( "_User" )
{
    this->setEnabled(true);
}

DroiUser* DroiUser::create()
{
    DroiUser* user = new DroiUser();
    return user;
}

//
DroiUser* DroiUser::getCurrentUser()
{
    if ( g_currentUser != nullptr ) {
        DroiUser*  tmp = g_currentUser.get();
        tmp->retain();
        return tmp;
    }

    // Try to load user from data/data.
    g_currentUser = DroiUser::loadFromFile();
    return g_currentUser;
}

DroiUser* DroiUser::login( const std::string& userId, const std::string& password, DroiError* error )
{
    if (userId.length() == 0 || password.length() == 0) {
        if (error != nullptr)
            *error = DroiError::createDroiError( DROICODE_INVALID_PARAMETER, "", "Empty UserId or Password" );
        return nullptr;
    }

    RefPtrAutoReleaser<DroiUser> user = DroiUser::getCurrentUser();
    if (user != nullptr && user->isLoggedIn() && !user->isAnonymouseUser()) {
        if (error != nullptr)
            *error = DroiError::createDroiError( DROICODE_ERROR,"", "Another user had logged in. Please logout first");
            	
        return nullptr;
    }

    DroiError e = DroiError::createDroiError( DROICODE_OK, "" );
    std::string encryptedPWD = DroiHelper::sha256(password);
    std::string installationId = DroiCore::getInstallationId();
    RefPtrAutoReleaser<DroiLoginBody> loginBody = DroiLoginBody::bodyFromUserId(userId, encryptedPWD, installationId);
    RefPtrAutoReleaser<DroiLoginOutput> loginOut = DroiRemoteServiceHelper::login(loginBody, &e);
    if (!e.isOk()) {
        if (error != nullptr) *error = e;
    }

    if (!loginOut->isSuccessful()) {
        std::string ticket = "";
        if (loginOut->getTicket() != nullptr) {
            ticket = loginOut->getTicket()->asString();
        }
        switch (loginOut->getCode()->asInt() ) {
            case LOGIN_USER_NOT_EXISTS:
                e = DroiError::createDroiError(DROICODE_USER_NOT_EXISTS, ticket);
                break;
            case LOGIN_USER_WRONG_PASSWORD:
                e = DroiError::createDroiError(DROICODE_USER_PASSWORD_INCORRECT, ticket);
                break;
            case LOGIN_USER_DISABLE:
                e = DroiError::createDroiError(DROICODE_USER_DISABLE, ticket);
                break;
            default:
                e = DroiError::createDroiError((DroiErrorCode )loginOut->getCode()->asInt(), ticket);
                break;
        }
        if (error != nullptr) *error = e;

        return nullptr;
    }

    RefPtr<DroiObject> obj = loginOut->getUser();
    user = new DroiUser();
    user->copyFromUser(obj);
    user->mSession.insert(std::make_pair("Token", *(loginOut->getToken())));
    user->mSession.insert(std::make_pair("ExpireAt", *(loginOut->getExpire())));
    DroiUser::storeUser(user);
    if (error != nullptr) *error = e;

    return user;
}

DroiError DroiUser::logout()
{
    DroiError err = DroiError::createDroiError( DROICODE_OK, "" );
    if ( !mSession.empty()) {
        RefPtrAutoReleaser<DroiLogoutBody> logoutBody = DroiLogoutBody::bodyFromToken(getSessionToken(), objectId());
        RefPtrAutoReleaser<DroiLogoutOutput> loginOut = DroiRemoteServiceHelper::logout(logoutBody.get(), &err);
    } else {
        err = DroiError::createDroiError(DroiErrorCode::DROICODE_OK, "");
    }

    DroiUser::cleanUp();
    
    return err;
}

DroiError DroiUser::signup()
{
    DroiError err;
    std::string userId, password;
    this->getValue("UserId", userId);
    this->getValue("Password", password);
    if (userId.length() == 0 || password.length() == 0) {
        return DroiError::createDroiError( DROICODE_INVALID_PARAMETER, "", "Empty UserId or Password" );
    }

    DroiUser* user = DroiUser::getCurrentUser();
    if (user != nullptr && user->isLoggedIn() && !user->isAnonymouseUser())
        return DroiError::createDroiError( DROICODE_ALREADY_LOGIN_A_USER, "Please logout current user first");

    if (this->isLoggedIn() && !this->isAnonymouseUser())
        return DroiError::createDroiError(DROICODE_USER_ALREADY_EXISTS, "This user had been signup.");

    bool needToCHPWD = false;
    RefPtrAutoReleaser<RefMap> authData = new RefMap();
    if (this->getValue("AuthData", *authData) == true) {
        RefMap::iterator it = authData->find(KEY_ANONYMOUS);
        if (it != authData->end()) {
            authData->erase(it);
            needToCHPWD = true;
            this->setValue("AuthData", *authData);
        }

        if (authData->empty())
            this->eraseValue("AuthData");
    }

    if (this->isLoggedIn() == false) {
        std::string hashPassword = DroiHelper::to_lower(DroiHelper::sha256(password));
        //set hashed password for signup
        this->setValue("Password", hashPassword);
        RefPtrAutoReleaser<DroiSignUpBody> signBody = DroiSignUpBody::bodyFromType("general", this);
        RefPtrAutoReleaser<DroiSignUpOutput> signupOut = DroiRemoteServiceHelper::signUp(signBody, &err);

        if (!err.isOk())
            return err;

        if (signupOut->getCode()->asInt() != 0) {
            int code = signupOut->getCode()->asInt();
            if (code == DB_RECORD_CONFLIT || code == SIGNUP_USER_EXISTS) {
                err.setCode(DROICODE_USER_ALREADY_EXISTS);
            } else {
                err.setCode((DroiErrorCode) signupOut->getCode()->asInt());
            }

            err.setMessage("signup fail");
            return err;
        }

        if (signupOut->getCode()->asString() != "") {
            mSession.clear();
            mSession.insert(std::make_pair("Token", *(signupOut->getToken())));
            mSession.insert(std::make_pair("ExpireAt", *(signupOut->getExpire())));
        }
    } else {
        err = this->save();
        if (!err.isOk()) {
            return err;
        }
        
        err = this->changePassword("",  password);
        if (!err.isOk()) {

            return err;
        }
    }

    // Clear password
    this->setValue("Password", "");
    DroiUser::storeUser(this);
    err = DroiError::createDroiError(DROICODE_OK, "");

    return err;
}

DroiUser* DroiUser::createAnonymouseUser()
{
    std::string deviceId = DroiCore::getDeviceId();
    std::string installationId = DroiCore::getInstallationId();
    
    if ( deviceId.empty() || installationId.empty() )
        return nullptr;
    
    DroiUser* user = new DroiUser();
    RefPtrAutoReleaser<RefMap> authData = new RefMap();
    authData->insert( KEY_ANONYMOUS, RefPtrAutoReleaser<RefValue>(new RefValue(Value("1"))));
    user->setValue( "AuthData", *authData );
    user->setValue( "UserId", installationId + deviceId );
    g_currentUser = user;
    return user;
}

DroiUser* DroiUser::loginWithAnonymous( DroiError* err )
{
    DroiUser::cleanUp();
    std::string deviceId = DroiCore::getDeviceId();
    std::string installationId = DroiCore::getInstallationId();
    
    if ( deviceId.empty() || installationId.empty() )
        return nullptr;
    
    DroiUser* user = DroiUser::getCurrentUser();
    if ( user != nullptr && user->isLoggedIn() && !user->isAnonymouseUser() ) {
        if ( err != nullptr )
            *err = DroiError::createDroiError( DROICODE_ERROR, "", "Another user had logged in. Please logout first.");
        if ( user != nullptr )
            user->release();
        return nullptr;
    }
    
    if ( user == nullptr ) {
        user = new DroiUser();
    }

    DroiError e = user->loginAnonymousInternal();
    if ( err != nullptr )
        *err = e;
    
    if (err->isOk() == false)
        return nullptr;

    DroiUser::storeUser( user );
    return user;
}

DroiError DroiUser::loginAnonymousInternal()
{
    DroiError e = DroiError::createDroiError( DROICODE_OK, "" );
    std::string deviceId = DroiCore::getDeviceId();
    std::string installationId = DroiCore::getInstallationId();

    RefPtrAutoReleaser<DroiLoginBody> loginBody = DroiLoginBody::bodyFromAnonymousInstallationId(installationId);
    RefPtrAutoReleaser<DroiLoginOutput> loginOut = DroiRemoteServiceHelper::login(loginBody.get(), &e);
    if (!e.isOk())
        return e;

    if (loginOut != nullptr && loginOut->getCode()->asInt() == DROICODE_OK) {
        RefPtr<DroiObject> user = loginOut->getUser();
        if (user == nullptr)
            return DroiError::createDroiError( DROICODE_ERROR, "", "No user result" );
        this->copyFromUser(user);
        mSession.insert(std::make_pair("Token", *(loginOut->getToken())));
        mSession.insert(std::make_pair("ExpireAt", *(loginOut->getExpire())));
    } else {
        RefPtrAutoReleaser<DroiUser> user = DroiUser::createAnonymouseUser();
        RefPtrAutoReleaser<DroiSignUpBody> signBody = DroiSignUpBody::bodyFromType(KEY_ANONYMOUS, user);
        RefPtrAutoReleaser<DroiSignUpOutput> signOut = DroiRemoteServiceHelper::signUp(signBody, &e);

        if (!e.isOk())
            return e;

        if (signOut->getCode()->asInt() != 0) {
            if (signOut->getCode()->asInt() == DB_RECORD_CONFLIT) {
                e.setCode(DROICODE_USER_ALREADY_EXISTS);
            } else {
                e.setCode((DroiErrorCode) signOut->getCode()->asInt());
            }
            e.setMessage("");

            return e;
        } else {
            this->copyFrom(user);
            if (signOut->getCode()->asString() != "") {
                mSession.insert(std::make_pair("Token", *(signOut->getToken())));
                mSession.insert(std::make_pair("ExpireAt", *(signOut->getExpire())));
            }
        }
    }

    return e;
}

//static
void DroiUser::storeUser(DroiUser *user)
{
    if (user == nullptr) return;

     // fill the permission if empty. To avoid stack overflow about loop below.
     //   getCurrentUser
     //   fromJson
     //   checkDirtyFlags
     //   toJson
     //   getDefaultPermission (permission is nil)
     //   getCurrentUser
    if (user->getPermission() == nullptr) {
        DroiPermission* acl = DroiPermission::getDefaultPermission();
        if (acl == nullptr) {
            acl = new DroiPermission;
            acl->setPublicReadPermission(true);
            acl->setPublicWritePermission(true);
        }
        user->setPermission(acl);
    }

    // Object -> JSON string -> encrypt string -> write to file
    cJSON* jsonObject = cJSONHelper::toJSON( user );
    char* resJson = cJSON_PrintUnformatted( jsonObject );
    std::string rawText = resJson;
    free(resJson);
    cJSON_Delete( jsonObject );

    RefPtrAutoReleaser<RefMap> userDict = new RefMap();
    userDict->insert("user", RefPtrAutoReleaser<RefValue>(new RefValue(Value(rawText))) );
    userDict->insert("session", RefPtrAutoReleaser<RefValue>(new RefValue(Value(user->mSession))));
    jsonObject = cJSONHelper::toJSONMap(userDict);
    resJson = cJSON_PrintUnformatted(jsonObject);
    rawText = resJson;
    free(resJson);
    cJSON_Delete(jsonObject);

    UserDefault::getInstance()->setStringForKey("DROI_CUR_USER", rawText);
    g_currentUser = user;
}


DroiUser* DroiUser::loadFromFile()
{
    DroiUser* user = new DroiUser;
    UserDefault* setting = UserDefault::getInstance();
    std::string rawText = setting->getStringForKey("DROI_CUR_USER");
    if (rawText.size() == 0) {
        user->release();
        return nullptr;
    }

    RefPtrAutoReleaser<RefMap> userDict = cJSONHelper::fromJSON(rawText);
    RefMap::const_iterator iter = userDict->find("user");
    std::string userRawText;
    if (iter != userDict->end())
        userRawText = ((RefValue*)iter->second)->asString();

    RefMap* session = nullptr;
    iter = userDict->find("session");
    if (iter != userDict->end())
        session = (RefMap*)iter->second;

    cJSON* json = cJSON_Parse( userRawText.c_str() );
    if ( json == NULL ) {
        user->release();
        return nullptr;
    }
    RefPtrAutoReleaser<DroiObject> dObject = cJSONHelper::fromJSON( json );
    user->copyFromUser(dObject);
    user->mSession.insert(std::make_pair("Token", *(RefValue*)session->at("Token")));
    user->mSession.insert(std::make_pair("ExpireAt", *(RefValue*)session->at("ExpireAt")));
    cJSON_Delete( json );

    return user;
}

void DroiUser::cleanUp()
{
    UserDefault* userDefalt = UserDefault::getInstance();
    userDefalt->setStringForKey("DROI_CUR_USER", "");
    g_currentUser = nullptr;
}

void DroiUser::copyFromUser(DroiObject* user)
{
    DroiObject::copyFrom(user);
}

// Properties
bool DroiUser::isAnonymouseUser()
{
    RefPtrAutoReleaser<RefMap> map = new RefMap();
    if ( getValue( "AuthData", *map ) == false ) {
        return false;
    }
    
    RefMap::const_iterator iter = map->find( KEY_ANONYMOUS );
    return (iter!=map->end())?true:false;
}

bool DroiUser::isLoggedIn()
{
    if ( mSession.empty() )
        return false;
    
    
    Value tokenValue = mSession["Token"];
    Value expireAtValue = mSession["ExpireAt"];
    if ( tokenValue.isNull() || expireAtValue.isNull() )
        return false;
    
    // TODO: Time UNIT
    std::string token = static_cast<std::string>(tokenValue.asString());
    std::chrono::milliseconds expire(0);
    DroiDateTime::isValidISO8601String(expireAtValue.asString().c_str(), expire);

    return expire > DroiDateTime::now() && !token.empty();
}

void DroiUser::setAutoAnonymouseUser(bool flag)
{
    mAutoAnonymousUserEnabled = flag;
}

bool DroiUser::isAutoAnonymousUserEnabled()
{
    return mAutoAnonymousUserEnabled;
}

bool DroiUser::enabled()
{
    bool enabled = false;
    getValue( "Enabled", enabled );
    return enabled;
}

void DroiUser::setEnabled( bool enabled )
{
    setValue( "Enabled", enabled );
}

// Getter
std::string DroiUser::getUserId()
{
    std::string userId;
    getValue( "UserId", userId );
    return userId;
}

std::string DroiUser::getEmail()
{
    std::string email;
    getValue( "Email", email );
    return email;
}

std::string DroiUser::getPhoneNumber() 
{
    std::string phoneNumber;
    getValue( "PhoneNum", phoneNumber );
    return phoneNumber;
}

std::string DroiUser::getSessionToken()
{
    if ( mSession.empty() )
        return "";
    
    
    Value token = mSession["Token"];
    if ( token.isNull() )
        return "";
    
    // TODO: Time UNIT
    return static_cast<std::string>(token.asString());
}

// Setter
void DroiUser::setUserId( const std::string& userId )
{
    setValue( "UserId", userId );
}

void DroiUser::setPassword( const std::string& password)
{
    setValue( "Password", password);
}

void DroiUser::setEmail( const std::string& email )
{
    setValue( "Email", email );
}

void DroiUser::setPhoneNumber( const std::string& phoneNumber )
{
    setValue( "PhoneNum", phoneNumber );
}

DroiError DroiUser::changePassword( const std::string& oldPassword, const std::string newPassword)
{
    RefPtrAutoReleaser<RefMap> authData = new RefMap();
    this->getValue("AuthData", *authData);
    if (authData->size() > 0) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "Only normal user can change password.");
    }
    
    if (!this->isLoggedIn()) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "User is not login");
    }

    std::string hashOld = DroiHelper::to_lower(DroiHelper::sha256(oldPassword));
    std::string hashNew = DroiHelper::to_lower(DroiHelper::sha256(newPassword));

    DroiError err;
    std::string token = this->getSessionToken();
    RefPtrAutoReleaser<DroiRestfulOutput> output = DroiRemoteServiceHelper::changePassword(hashOld, hashNew, token, &err);

    if (!err.isOk()) {
        return err;
    }

    DroiErrorCode code = DROICODE_OK;
    std::string appendMsg = "";
    if (output.get()->getCode()->asInt() != 0) {
        code = (DroiErrorCode) output.get()->getCode()->asInt();
    }

    std::string ticket;
    if (output->getTicket() != nullptr) {
        ticket = output->getTicket()->asString();
    }
    return DroiError::createDroiError(code, ticket, appendMsg);
}

DroiError DroiUser::validateEmail()
{
    if ( !(this->isLoggedIn()) || this->isAnonymouseUser()) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "User is not authorized or anonymous.");
    } else if (this->_EmailVerified) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "The email had verified.");
    } else if (this->getEmail().length() == 0) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "Email is empty.");
    }

    DroiError err;
    RefPtrAutoReleaser<DroiRestfulOutput> output = DroiRemoteServiceHelper::validateEmailWithToken(this->getSessionToken(), &err);
    if (!err.isOk()) {
        return err;
    }

    DroiErrorCode code = DROICODE_OK;
    std::string appendMsg = "";
    if (output.get()->getCode()->asInt() != 0) {
        code = (DroiErrorCode) output.get()->getCode()->asInt();
        appendMsg.append("validate error. code: ").append(std::to_string(output->getCode()->asInt()));
    }

    std::string ticket;
    if (output->getTicket() != nullptr) {
        ticket = output->getTicket()->asString();
    }

    return DroiError::createDroiError(code, ticket, appendMsg);
}


DroiError DroiUser::validatePhoneNumber()
{
    if ( !(this->isLoggedIn()) || this->isAnonymouseUser()) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "User is not authorized or anonymous.");
    } else if (this->_PhoneNumVerified) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "The phone number had verified.");
    } else if (this->getEmail().length() == 0) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "PhoneNum is empty.");
    }

    DroiError err;
    RefPtrAutoReleaser<DroiRestfulOutput> output = DroiRemoteServiceHelper::validatePhoneNumWithToken(this->getSessionToken(), &err);
    if (!err.isOk()) {
        return err;
    }

    DroiErrorCode code = DROICODE_OK;
    std::string appendMsg = "";
    if (output.get()->getCode()->asInt() != 0) {
        code = (DroiErrorCode) output.get()->getCode()->asInt();
        appendMsg.append("validate error. code: ").append(std::to_string(output->getCode()->asInt()));
    }

    std::string ticket;
    if (output->getTicket() != nullptr) {
        ticket = output->getTicket()->asString();
    }

    return DroiError::createDroiError(code, ticket, appendMsg);
}

DroiError DroiUser::confirmPhoneNumberPinCode( const std::string& pinCode)
{
    if ( !(this->isLoggedIn()) || this->isAnonymouseUser()) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "User is not authorized or anonymous.");
    } else if (this->_PhoneNumVerified) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "The phone number had verified.");
    } else if (this->getEmail().length() == 0) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "PhoneNum is empty.");
    }

    DroiError err;
    RefPtrAutoReleaser<DroiRestfulOutput> output = DroiRemoteServiceHelper::confirmPhoneNumPinCodeWithToken(this->getSessionToken(), pinCode, &err);
    if (!err.isOk()) {
        return err;
    }

    DroiErrorCode code = DROICODE_OK;
    std::string appendMsg = "";
    if (output.get()->getCode()->asInt() != 0) {
        code = (DroiErrorCode) output.get()->getCode()->asInt();
        appendMsg.append("validate error. code: ").append(std::to_string(output->getCode()->asInt()));
    } else {
        this->_PhoneNumVerified = true;
        if (this == g_currentUser) {
            DroiUser::storeUser(this);
        }
    }
    std::string ticket;
    if (output->getTicket() != nullptr) {
        ticket = output->getTicket()->asString();
    }

    return DroiError::createDroiError(code, ticket, appendMsg);
}


bool DroiUser::refreshValidationStatus()
{
    std::string objectid = this->objectId();
    DroiError error;
    auto block = [&]() {
        DroiError err;
        DroiCondition* cond = DroiCondition::eq(DROI_KEY_JSON_OBJECTID
                                                , Value(objectid));
        DroiQuery* query  = DroiQuery::create();
        query->queryData("_User").where(cond);
        RefPtrAutoReleaser<RefVector> res = query->runQuery(&err);
        if (!err.isOk()) {
            error = err;
            csCondition.notify_one();
            return;
        }

        if (res.get()->size() != 1) {
            error = DroiError::createDroiError(DROICODE_ERROR, "", "User error. look up fail.");
            csCondition.notify_one();
            return;
        }

        DroiObject* obj = (DroiObject*)res->at(0);
        bool existed = obj->getValue("EmailVerified", this->_EmailVerified);
        if ( !existed ) this->_EmailVerified = false;

        existed = obj->getValue("PhoneNumVerified", this->_PhoneNumVerified);
        if ( !existed ) this->_PhoneNumVerified = false;

        error = DroiError::createDroiError(DROICODE_OK, "");

        csCondition.notify_one();
    };

    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher(DroiTaskBackgroundThread);
    DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();
    if ( currentTD != nullptr && &td == currentTD ) {
        // Same thread
        block();
    } else {
        std::unique_lock<std::mutex> lock(csLocker);
        td.enqueueTask(block);
        csCondition.wait(lock);
    };

    return error.isOk();
}


bool DroiUser::refreshValidationStatusInBackground(DroiCallback<RefPtr<DroiUser>>::onCallback2 callback)
{
    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher( DroiTaskBackgroundThread );
    DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();

    retain();
    DROITASKID tid = td.enqueueTask([this, callback, currentTD] {
        bool ret = refreshValidationStatus();
        RefPtr<DroiUser> keep(this);
        currentTD->enqueueTask([ret, callback, keep] {
        DroiError err = DroiError::createDroiError(ret ? DROICODE_OK : DROICODE_ERROR, "");
            keep.get()->release();
            callback(keep, err );
        });

    });

    return (tid > 0 )? true : false;
}

bool DroiUser::isEmailVerified()
{
    return this->_EmailVerified;
}

bool DroiUser::isPhoneNumVerified()
{
    return this->_PhoneNumVerified;
}

