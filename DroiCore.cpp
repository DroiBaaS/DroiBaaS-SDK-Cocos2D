/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiCore.h"
#include "DroiHelper.h"

#define CACHE_KEY_INSTALLATION_OD "DROI_INSTALLATION_ID"

std::string DroiCore::InstallationId = "UnKnown";
std::string DroiCore::ApplicationId = "UnKnown";

std::string DroiCore::DroiEntry = "https://api.droibaas.com";

using namespace cocos2d;

const std::string DroiCore::getDeviceId()
{
    DeviceId current = UserDefault::getInstance()->getStringForKey(CACHE_KEY_DEVICE_ID);
    if (current.empty()) {
        current = DroiHttp::instance()->getDeviceId();
        UserDefault::getInstance()->setStringForKey(CACHE_KEY_DEVICE_ID, current);
    }

    return current;
}

const std::string DroiCore::getInstallationId()
{
    std::string current = UserDefault::getInstance()->getStringForKey(CACHE_KEY_INSTALLATION_OD);
    if (current.empty()) {
        current = DroiHelper::generateUUID();
        UserDefault::getInstance()->setStringForKey(CACHE_KEY_INSTALLATION_OD, current);
    }
    
    return current;
}

const std::string DroiCore::getAppId()
{
    return ApplicationId;
}

void DroiCore::initializeCore( const std::string& applicationId)
{
    ApplicationId = applicationId;
}

