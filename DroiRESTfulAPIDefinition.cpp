/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */


#include <stdio.h>
#include "DroiRESTfulAPIDefinition.h"


const std::string DroiRESTfulAPIDefinition::API_VERSION = "/v2";
const std::string DroiRESTfulAPIDefinition::PREFIX = "/droi";
const std::string DroiRESTfulAPIDefinition::DROIOBJECT = PREFIX + "/objects";
const std::string DroiRESTfulAPIDefinition::USERSROOT = PREFIX + "/users";
const std::string DroiRESTfulAPIDefinition::USERSAUTHROOT = "/droiusers";
const std::string DroiRESTfulAPIDefinition::DROICLOUDCACHE = PREFIX + "/cloudcache" +  API_VERSION;
const std::string DroiRESTfulAPIDefinition::GROUPSROOT = PREFIX + "/groups";

const std::string DroiRESTfulAPIDefinition::CRUD = DROIOBJECT + API_VERSION;
const std::string DroiRESTfulAPIDefinition::USER_SIGNUP = USERSAUTHROOT + API_VERSION + "/signup";
const std::string DroiRESTfulAPIDefinition::USER_LOGIN = USERSAUTHROOT + API_VERSION + "/login";
const std::string DroiRESTfulAPIDefinition::USER_LOGOUT = USERSAUTHROOT + API_VERSION + "/logout";
const std::string DroiRESTfulAPIDefinition::USER_VALIDATE_SMS = USERSAUTHROOT + API_VERSION + "/sms";
const std::string DroiRESTfulAPIDefinition::USER_VALIDATE_SMS_CHECK = USERSAUTHROOT + API_VERSION + "/validate/sms";
const std::string DroiRESTfulAPIDefinition::USER_VALIDATE_EMAIL = USERSAUTHROOT + API_VERSION + "/email";
const std::string DroiRESTfulAPIDefinition::USER_RESET_PWD = USERSAUTHROOT + API_VERSION + "/password";
const std::string DroiRESTfulAPIDefinition::USERS = USERSROOT + API_VERSION;
const std::string DroiRESTfulAPIDefinition::GROUPS = GROUPSROOT + API_VERSION;
const std::string DroiRESTfulAPIDefinition::CLOUD_CACHE = DROICLOUDCACHE + API_VERSION + "/ops";

const std::string DroiRESTfulAPIDefinition::REG_DEVICE = "/uregister";
