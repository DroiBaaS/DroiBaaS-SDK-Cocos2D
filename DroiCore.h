/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiCore_H__
#define __DroiCore_H__
#include <string>
#include "DroiHttp.h"

#define CACHE_KEY_DEVICE_ID "DROI_DEVICE_ID"

/**
 * DoireCore provides methods to get Droi AppId, Platform Id and Installation Id information. Developer should use initializeCore to setup Droi Core SDK environment.
 */
class DroiCore
{
public:
    /**
     * Get device identifier
     * \return The device identifier
     */
    static const std::string getDeviceId();
    
    /**
     * Get installation identifier
     * \return The installation identifier
     */
    static const std::string getInstallationId();
    
    /**
     * Get application identifier
     * \return The application identifier
     */
    static const std::string getAppId();

    /**
     * Initiate Droi Core SDK
     * \param applicationId The application identifie
     */
    static void initializeCore(const std::string& applicationId);
    
    friend class DroiHttp;
    friend class DroiRemoteServiceHelper;
private:
    static std::string InstallationId;
    static std::string ApplicationId;
    static std::string PlatformId;
    static std::string DroiEntry;
};

#endif 

