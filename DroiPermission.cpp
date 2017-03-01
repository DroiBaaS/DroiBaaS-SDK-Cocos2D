/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiPermission.h"
#include "cJSON.h"
#include "cJSONHelper.h"
using namespace cocos2d;

DroiPermission* DroiPermission::_gDefaultPermission = nullptr;

DroiPermission::DroiPermission()
{
    publicPermission = 0;
}

DroiPermission::~DroiPermission()
{
}

DroiPermission*  DroiPermission::getDefaultPermission(){
    return _gDefaultPermission;
}

void DroiPermission::setDefaultPermission(DroiPermission*  permission)
{
    _gDefaultPermission = permission;
}

// Set user r/w permission
void DroiPermission::setUserWritePermission( const char* userId, bool grantPermission )
{
    updatePermission( userPermission, userId, DROI_PERMISSION_WRITE, grantPermission );
}

void DroiPermission::setUserReadPermission( const char* userId, bool grantPermission )
{
    updatePermission( userPermission, userId, DROI_PERMISSION_READ, grantPermission );
}


// Set public r/w permission
void DroiPermission::setPublicWritePermission( bool grantPermission )
{
    if ( grantPermission )
        publicPermission |= DROI_PERMISSION_WRITE;
    else
        publicPermission &= ~DROI_PERMISSION_WRITE;
}

void DroiPermission::setPublicReadPermission( bool grantPermission )
{
    if ( grantPermission )
        publicPermission |= DROI_PERMISSION_READ;
    else
        publicPermission &= ~DROI_PERMISSION_READ;
}

//// Set group r/w permission
//void DroiPermission::setGroupWritePermission( const char* groupId, bool grantPermission )
//{
//    updatePermission( groupPermission, groupId, DROI_PERMISSION_WRITE, grantPermission );
//}
//
//void DroiPermission::setGroupReadPermission( const char* groupId, bool grantPermission )
//{
//    updatePermission( groupPermission, groupId, DROI_PERMISSION_READ, grantPermission );
//}

void DroiPermission::applyJSONObject( cJSON* json )
{
    // Release old memory
    publicPermission = 0;
    userPermission.clear();
    groupPermission.clear();

    // Extract data from json
    bool boolValue = false;
    if ( cJSONHelper::getBool( json, "pr", boolValue ) && boolValue )
        publicPermission |= DROI_PERMISSION_READ;
    boolValue = false;
    if ( cJSONHelper::getBool( json, "pw", boolValue ) && boolValue )
        publicPermission |= DROI_PERMISSION_WRITE;
    
    // USER
    cJSON* array = cJSON_GetObjectItem( json, "ur" );
    if ( array != NULL && array->type == cJSON_Array ) {
        applyPermission( array->child, DROI_PERMISSION_READ, userPermission );
    }
    array = cJSON_GetObjectItem( json, "uw" );
    if ( array != NULL && array->type == cJSON_Array ) {
        applyPermission( array->child, DROI_PERMISSION_WRITE, userPermission );
    }
    
    // GROUP
    array = cJSON_GetObjectItem( json, "gr" );
    if ( array != NULL && array->type == cJSON_Array ) {
        applyPermission( array->child, DROI_PERMISSION_READ, groupPermission );
    }
    array = cJSON_GetObjectItem( json, "gw" );
    if ( array != NULL && array->type == cJSON_Array ) {
        applyPermission( array->child, DROI_PERMISSION_WRITE, groupPermission );
    }
}

void DroiPermission::applyPermission( cJSON* items, int mask, cocos2d::ValueMap& perm )
{
    for ( ; items != NULL; items = items->next ) {
        cocos2d::ValueMap::iterator dIter = perm.find( items->valuestring );
        if ( dIter != userPermission.end() ) {
            cocos2d::Value& value = dIter->second;
            int v = value.asInt() | mask;
            value = v;
        } else {
            cocos2d::Value v(mask);
            perm.insert( cocos2d::ValueMap::value_type( items->valuestring, v ) );
        }
    }
}


cJSON* DroiPermission::toJSONObject()
{
    cJSON* node = cJSON_CreateObject();
    
    // public read/write
    if ( publicPermission&DROI_PERMISSION_READ )
        cJSON_AddTrueToObject( node, "pr" );
    if ( publicPermission&DROI_PERMISSION_WRITE )
        cJSON_AddTrueToObject( node, "pw" );
    
    permissionMapToJSONObject( node, userPermission, "ur", "uw" );
    permissionMapToJSONObject( node, groupPermission, "gr", "gw" );
    return node;
}

DroiPermission* DroiPermission::clone()
{
    DroiPermission* perm = new DroiPermission();
    
    //
    perm->publicPermission = publicPermission;
    perm->userPermission = userPermission;
    perm->userPermission = userPermission;
    perm->groupPermission = groupPermission;
    return perm;
}

DroiPermission& DroiPermission::operator=( DroiPermission& o )
{
    publicPermission = o.publicPermission;
    userPermission = o.userPermission;
    groupPermission = o.groupPermission;
    return *this;
}


void DroiPermission::permissionMapToJSONObject( cJSON* node, const cocos2d::ValueMap& perm, const char* readKey, const char* writeKey )
{
    if ( perm.empty() )
        return;
    
    cJSON* readArray = NULL;
    cJSON* writeArray = NULL;
    
    cocos2d::ValueMap::const_iterator iter = perm.begin();
    for ( ; iter != perm.end(); ++iter ) {
        const Value& value = iter->second;
        if ( value.getType() != cocos2d::Value::Type::INTEGER || value.asInt() == 0 )
            continue;
        
        int permValue = value.asInt();
        if ( permValue&DROI_PERMISSION_READ ) {
            if ( readArray == NULL )
                readArray = cJSON_CreateArray();
            cJSON_AddItemToArray( readArray, cJSON_CreateString(iter->first.c_str()) );
        }
            
        if ( permValue&DROI_PERMISSION_WRITE ) {
            if ( writeArray == NULL )
                writeArray = cJSON_CreateArray();
           cJSON_AddItemToArray( writeArray, cJSON_CreateString(iter->first.c_str()) );
        }
    }
    
    //
    if ( readArray != NULL )
        cJSON_AddItemToObject( node, readKey, readArray );
    if ( writeArray != NULL )
        cJSON_AddItemToObject( node, readKey, writeArray );
}

void DroiPermission::updatePermission( cocos2d::ValueMap& perms, const char* identify, int permissionMask, bool grantPermission )
{
    ValueMap::iterator iter = perms.find( identify );
    Value value;
    if ( iter == perms.end() ) {
        value = 0;
        perms.insert( ValueMap::value_type( identify, value ) );
    } else {
        
        value = iter->second;
    }
    
    if ( grantPermission )
        value = value.asInt() | permissionMask;
    else {
        int d = value.asInt() & ~(permissionMask);
        value = d;
    }
    
    //
    perms[ identify ] = value;
}

