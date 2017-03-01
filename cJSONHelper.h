/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __cJSONHelper_H__
#define __cJSONHelper_H__
#include "RefValue.h"
#include <string>
#include "DroiObject.h"
#include "DroiUser.h"

struct cJSON;

class cJSONHelper
{
public:
    static bool getString( cJSON* json, const std::string& keyName, std::string& result );
    static bool getBool( cJSON* json, const std::string& keyName, bool& result );
    static bool getInt( cJSON* json, const std::string& keyName, int32_t& result );
    static bool getDouble( cJSON* json, const std::string& keyName, double& result );
    
    // Deserialization
    static DroiObject* fromJSON( cJSON* json );
    static DroiObject* fromJSON( RefMap* dict );
    static RefMap* fromJSON( const std::string& msg );
    static RefVector* fromJSONToVector( const std::string& msg );
    
    // Serialization
    static cJSON* toJSON( Value& obj );
    static cJSON* toJSON( DroiObject* obj );
    static cJSON* toJSONMap( RefMap* obj );
    static cJSON* toJSONArray( RefVector* obj );
    static cJSON* toJSONMap( ValueMap* obj );
    static cJSON* toJSONArray( ValueVector* obj );

private:
    
    static cocos2d::Ref* fromJSONToValue( cJSON* node );
    static void addToJSONMap( cJSON* node, const std::string& keyName, cocos2d::Ref* pValue );
    static void addToJSONMap( cJSON* node, const std::string& keyName, Value value );
    
    static bool processDataMember( cJSON* node, DroiObject* dObject );
};
    
#endif
