/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiError_H__
#define __DroiError_H__
#include <string>

typedef enum tagDroiErrorCode {
    DROICODE_OK = 0,

    //10700xx
    DROICODE_UNKNOWN_ERROR = 1070000,
    DROICODE_ERROR = 1070001,
    DROICODE_USER_NOT_EXISTS = 1070002,
    DROICODE_USER_PASSWORD_INCORRECT = 1070003,
    DROICODE_USER_ALREADY_EXISTS = 1070004,
    DROICODE_NETWORK_NOT_AVAILABLE = 1070005,
    DROICODE_USER_NOT_AUTHORIZED = 1070006,
    DROICODE_SERVER_NOT_REACHABLE = 1070007,
    DROICODE_HTTP_SERVER_ERROR = 1070008,
    DROICODE_SERVICE_NOT_ALLOWED = 1070000,
    DROICODE_SERVICE_NOT_FOUND = 1070010,
    DROICODE_INTERNAL_SERVER_ERROR = 1070011,
    DROICODE_INVALID_PARAMETER = 1070012,
    DROICODE_USER_DISABLE = 1070013,
    DROICODE_CORE_NOT_INITIALIZED = 1070017,
    DROICODE_USER_CANCELED = 1070018,
    DROICODE_BANDWIDTH_LIMIT_EXCEED = 1070019,
    
    // General: 10701**
    DROICODE_APPLICATION_ID_UNCORRECTED = 1070101,
    DROICODE_OPERATION_NOT_ALLOWED_IN_MAINTHREAD = 1070102,

    //Network: 10702**
    DROICODE_TIME_UNCORRECTED = 1070201,
    DROICODE_TIMEOUT = 1070202,

    //DroiUser: 10703**
    DROICODE_ALREADY_LOGIN_A_USER = 1070301,
    DROICODE_USER_CONTACT_HAD_VERIFIED = 1070302,
    DROICODE_USER_CONTACT_IS_EMPTY = 1070303,
    DROICODE_USER_FUNC_NOT_ALLOWED = 1070304

} DroiErrorCode;

class DroiError
{
public:
    DroiError();
    DroiError( const DroiError& err );
    DroiErrorCode getCode() const;
    void setCode(DroiErrorCode code);
    std::string getTicket() const;
    bool isOk() const;
    std::string getMessage() const;
    void setMessage(const std::string& msg);
    std::string toString();
    
    static DroiError createDroiError( DroiErrorCode code, const std::string& ticket, const std::string& message="" );
    
private:
    DroiErrorCode mCode;
    std::string mTicket;
    std::string mMessage;
};

#endif
