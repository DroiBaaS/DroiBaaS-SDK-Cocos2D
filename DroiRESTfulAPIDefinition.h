/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */


#ifndef DroiRESTfulAPIDefinition_h
#define DroiRESTfulAPIDefinition_h

class DroiRESTfulAPIDefinition
{
public:
    static const std::string CLOUDCODEROOT;
    static const std::string CRUD;
    static const std::string USER_SIGNUP;
    static const std::string USER_LOGIN;
    static const std::string USER_LOGOUT;
    static const std::string USER_OAUTH_BIND;
    static const std::string USER_VALIDATE_SMS;
    static const std::string USER_VALIDATE_SMS_CHECK;
    static const std::string USER_VALIDATE_EMAIL;
    static const std::string USER_RESET_PWD;
    static const std::string USERS;
    static const std::string GROUPS;
    static const std::string APP_GET_OAUTH;
    static const std::string APP_GET_PREFERNCE;
    static const std::string CLOUD_CACHE;
    static const std::string REG_DEVICE;

private:
    static const std::string API_VERSION;
    static const std::string PREFIX;
    static const std::string DROIOBJECT;
    static const std::string USERSAUTHROOT;
    static const std::string USERSROOT;
//    static const std::string DROIAPPS;
    static const std::string DROICLOUDCACHE;
    static const std::string GROUPSROOT;
};

#endif /* DroiRESTfulAPIDefinition_h */
