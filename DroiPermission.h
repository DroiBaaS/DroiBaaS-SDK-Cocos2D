/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiPermission_H__
#define __DroiPermission_H__
#include "DroiObject.h"

#define DROI_PERMISSION_READ    2
#define DROI_PERMISSION_WRITE   1


class DroiPermission
{
public:
    /**
     * Constructor
     */
    DroiPermission();

    /**
     * Destructor
     */
    ~DroiPermission();
    
    /**
     * Set user write permission.
     * \param userId User ID
     * \param grantPermission true to enable read, false to disable.
     */
    void setUserWritePermission( const char* userId, bool grantPermission );

    /**
     * Set user read permission.
     * \param userId User ID
     * \param grantPermission true to enable read, false to disable.
     */
    void setUserReadPermission( const char* userId, bool grantPermission );
    
    /**
     * Set public write permission.
     * \param grantPermission true to enable read, false to disable.
     */
    void setPublicWritePermission( bool grantPermission );
    /**
     * Set public read permission.
     * \param grantPermission true to enable read, false to disable.
     */
    void setPublicReadPermission( bool grantPermission );
    
//    /**
//     * Set group write permission.
//     * \param groupId Group ID
//     * \param grantPermission true to enable read, false to disable.
//     */
//    void setGroupWritePermission( const char* groupId, bool grantPermission );
//
//    /**
//     * Set group read permission.
//     * \param groupId Group ID
//     * \param grantPermission true to enable read, false to disable.
//     */
//    void setGroupReadPermission( const char* groupId, bool grantPermission );

    /**
     * Deserialize from cJSON object.
     * @param json JSON object
     */
    void applyJSONObject( cJSON* json );

    /**
     * Serialize to JSON format.
     * \return cJSON object
     */
    cJSON* toJSONObject();

    /**
     * Clone current object
     * \return The clone object
     */
    DroiPermission* clone();
    DroiPermission& operator=( DroiPermission& o );

    /**
     * Get default permission. Will assign to default DroiObject data if not assigned in DroiObject.
     * \return Permission
     */
    static DroiPermission* getDefaultPermission();

    /**
     * Set default permission. Assign to DroiObject data if not assigned in DroiObject.
     * \param permission
     */
    static void setDefaultPermission(DroiPermission*  permission);

private:
    void permissionMapToJSONObject( cJSON* node, const cocos2d::ValueMap& perm, const char* readKey, const char* writeKey );
    void applyPermission( cJSON* items, int mask, cocos2d::ValueMap& perm );
    void updatePermission( cocos2d::ValueMap& perm, const char* identify, int permissionMask, bool grantPermission );
    int publicPermission;
    cocos2d::ValueMap userPermission;
    cocos2d::ValueMap groupPermission;

    static DroiPermission* _gDefaultPermission;
};
#endif


