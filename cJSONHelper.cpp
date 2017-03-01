/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "cJSONHelper.h"
#include "cJSON.h"
#include "DroiDateTime.h"
#include "DroiPermission.h"
#include "RefValue.h"
#include <cmath>
#include <cfloat>
#include <utility>
#include <cassert>

using namespace cocos2d;

extern "C" const char* gszJSON_CLASSNAME;
extern "C" const char* gszJSON_OBJECTID;
extern "C" const char* gszJSON_CREATION_TIME;
extern "C" const char* gszJSON_MODIFIED_TIME;
extern "C" const char* gszJSON_PERMISSION;

bool cJSONHelper::getString( cJSON* json, const std::string& keyName, std::string& result )
{
    bool res = false;
    cJSON* node = ::cJSON_GetObjectItem( json, keyName.c_str() );
    if ( node == NULL )
        return res;
    
    if ( node->type == cJSON_String ) {
        result = node->valuestring;
        res = true;
    } else {
        // TODO:
    }
    return res;
}

bool cJSONHelper::getBool( cJSON* json, const std::string& keyName, bool& result )
{
    bool res = false;
    cJSON* node = cJSON_GetObjectItem( json, keyName.c_str() );
    if ( node == NULL )
        return res;

    if ( node->type == cJSON_False && node->type == cJSON_True ) {
        result = (node->type==cJSON_True)?true:false;
        res = true;
    } else {
        // TODO:
    }
    return res;
}

bool cJSONHelper::getInt( cJSON* json, const std::string& keyName, int32_t& result )
{
    bool res = false;
    cJSON* node = cJSON_GetObjectItem( json, keyName.c_str() );
    if ( node == NULL )
        return res;
    
    if ( node->type == cJSON_Number ) {
        result = node->valueint;
        res = true;
    } else {
        // TODO:
    }
    return res;
}

bool cJSONHelper::getDouble( cJSON* json, const std::string& keyName, double& result )
{
    bool res = false;
    cJSON* node = cJSON_GetObjectItem( json, keyName.c_str() );
    if ( node == NULL )
        return res;
    
    if ( node->type == cJSON_Number ) {
        result = node->valuedouble;
        res = true;
    } else {
        // TODO:
    }
    return res;
}

// Deserializations
cocos2d::Ref* cJSONHelper::fromJSONToValue( cJSON* node )
{
    cocos2d::Ref* pValue = NULL;
    switch( node->type ) {
        case cJSON_Number:
        {
            // Check whether this value is double
            int v = node->valueint;
            if ( fabs(v - node->valuedouble) <= DBL_EPSILON )   { // Integer
                pValue = new RefValue( Value(node->valueint) );
            } else {
                pValue = new RefValue( Value(node->valuedouble) );
            }
        }
            break;
        case cJSON_String:
            pValue = new RefValue( Value(node->valuestring) );
            break;
        case cJSON_True:
        case cJSON_False:
            pValue = new RefValue( Value((node->type==cJSON_True)?true:false) );
            break;
        case cJSON_Array:
        {
            RefVector* vec = new RefVector();
            pValue = vec;
            cJSON* child = node->child;
            for (; child != NULL; child = child->next ) {
                Ref* res = fromJSONToValue(child);
                vec->pushBack( res );
                res->release();
            }
        }
            break;
        case cJSON_Object:
        {
            DroiObject* cObject = dynamic_cast<DroiObject*>(fromJSON( node ));
            if ( cObject != NULL ) {
                pValue = cObject;
                
            } else {
                
                // This is map object ??
                RefMap* map = new RefMap();
                pValue = map;
                cJSON* child = node->child;
                for (; child != NULL; child = child->next ) {
                    Ref* res = fromJSONToValue(child);
                    map->insert( child->string, res );
                    res->release();
                }
            }
        }
            break;
    }
    return pValue;
}

bool cJSONHelper::processDataMember( cJSON* node, DroiObject* dObject )
{
    bool res = false;
    if ( !strcmp( gszJSON_CLASSNAME, node->string) ) {
        res = true; // We don't process this property
    } else if ( !strcmp( gszJSON_OBJECTID, node->string) ) {
        if ( node->type == cJSON_String )
            dObject->mObjectId = node->valuestring;
        res = true;
    } else if ( !strcmp( gszJSON_CREATION_TIME, node->string) ) {
        if ( node->type == cJSON_String ) {
            DroiDateTime dt( node->valuestring );
            dObject->mCreationTime = dt.getTime();
        }
        res = true;
    } else if ( !strcmp( gszJSON_MODIFIED_TIME, node->string) ) {
        if ( node->type == cJSON_String ) {
            DroiDateTime dt( node->valuestring );
            dObject->mModifyTime = dt.getTime();
        }
        res = true;
    } else if ( !strcmp( gszJSON_PERMISSION, node->string) ) {
        if ( node->type == cJSON_Object ) {
            cJSON* perm = node->child;
            dObject->mpPermission = new DroiPermission();
            dObject->mpPermission->applyJSONObject( perm );
        }
    }
    
    return res;
}


DroiObject* cJSONHelper::fromJSON( cJSON* json )
{
    // cJSON_Get
    std::string className;
    if ( !cJSONHelper::getString( json, gszJSON_CLASSNAME, className ) || className.length() <= 0 ) {
        // This payload isn't for DroiObject
        return NULL;
    }
    
    DroiObject* dObject = DroiObject::createObject( className );
    
    cJSON* node = json->child;
    for ( ; node != NULL; node = node->next) {
        if ( processDataMember(node, dObject) == false ) {
            // Put into keyValue
            Ref* refObject = fromJSONToValue( node );
            dObject->set( node->string, refObject );
            refObject->release();
        }
    }
    return dObject;
}

DroiObject* cJSONHelper::fromJSON( RefMap* dict )
{
    std::string className;
    RefMap::const_iterator iter = dict->find("_ClassName");
    if ( iter != dict->end())
        className = ((RefValue*)iter->second)->asString();
    else {
        assert(false);
        className = "DroiObject";
    }

    // Get registered class creator.
    createObject cFunc = nullptr;
    if (className.size() > 0) {
        classFactory::const_iterator iter2 = DroiObject::registerdClasses.find(className);
        if ( iter2 != DroiObject::registerdClasses.end())
            cFunc = iter2->second;
    }

    DroiObject* obj = nullptr;
    if ( cFunc == nullptr) {
        obj = DroiObject::createObject(className);
    } else {
        obj = cFunc();
    }

    iter = dict->begin();
    for ( ; iter != dict->end(); ++iter) {
        obj->set(iter->first, iter->second);
    }

    return obj;
}

RefMap* cJSONHelper::fromJSON(const std::string& msg  )
{
    cJSON* json = cJSON_Parse( msg.c_str() );
    if ( json == NULL )
        return nullptr;
    
    //
    RefMap* res = new RefMap();
    cJSON* node = json->child;
    for ( ; node != NULL; node = node->next) {
        Ref* refObject = fromJSONToValue( node );
        res->insert( node->string, refObject );
        refObject->release();
    }
    
    cJSON_Delete( json );
    return res;
}

RefVector* cJSONHelper::fromJSONToVector( const std::string& msg )
{
    cJSON* json = cJSON_Parse( msg.c_str() );
    if ( json == NULL )
        return nullptr;

    RefVector* res = new RefVector();
    cJSON* child = json->child;
    for ( ; child != NULL; child = child->next) {
        Ref* refObject = fromJSONToValue( child );
        res->pushBack(refObject);
        refObject->release();
    }

    cJSON_Delete( json );
    return res;
}

// Serailizations
cJSON* cJSONHelper::toJSON( Value& obj )
{
    cJSON* res = nullptr;
    switch( obj.getType() ) {
        case RefValue::Type::BOOLEAN:
            res = cJSON_CreateBool(obj.asBool());
            break;
        case RefValue::Type::INTEGER:
        case RefValue::Type::UNSIGNED:
        case RefValue::Type::BYTE:
            res = cJSON_CreateNumber(obj.asInt());
            break;
        case RefValue::Type::DOUBLE:
        case RefValue::Type::FLOAT:
            res = cJSON_CreateNumber(obj.asDouble());
            break;
        case RefValue::Type::STRING:
            res = cJSON_CreateString(obj.asString().c_str());
            break;
        case RefValue::Type::VECTOR:
            res = toJSONArray( &obj.asValueVector());
            break;
        case RefValue::Type::MAP:
            res = toJSONMap( &obj.asValueMap() );
            break;
        case RefValue::Type::NONE:
        case RefValue::Type::INT_KEY_MAP:
        default:
            break;
    }
    
    return res;
}

cJSON* cJSONHelper::toJSONArray( RefVector* obj )
{
    cJSON* array = cJSON_CreateArray();
    RefVector::const_iterator iter = obj->begin();
    for ( ; iter != obj->end(); ++iter ) {
        Ref* pValue = *iter;

        if ( dynamic_cast<RefValue*>(pValue) != NULL ) {
            RefValue* val = dynamic_cast<RefValue*>(pValue);
            cJSON* res = toJSON( *val );
            cJSON_AddItemToArray( array, res);
        } else if ( dynamic_cast<RefValueVector*>(pValue) != NULL ) {
            RefValueVector* val = dynamic_cast<RefValueVector*>(pValue);
            cJSON_AddItemToArray( array, toJSONArray(val) );
        } else if ( dynamic_cast<RefValueMap*>(pValue) != NULL ) {
            RefValueMap* val = dynamic_cast<RefValueMap*>(pValue);
            cJSON_AddItemToArray( array, toJSONMap(val) );
        } else if ( dynamic_cast<RefMap*>(pValue) != NULL ) {
            RefMap* val = dynamic_cast<RefMap*>(pValue);
            cJSON_AddItemToArray( array, toJSONMap(val) );
        } else if ( dynamic_cast<RefVector*>(pValue) != NULL ) {
            RefVector* val = dynamic_cast<RefVector*>(pValue);
            cJSON_AddItemToArray( array, toJSONArray(val) );
        }
    }

    return array;
}

cJSON* cJSONHelper::toJSONArray( ValueVector* obj )
{
    cJSON* array = cJSON_CreateArray();
    ValueVector::const_iterator iter = obj->begin();
    for ( ; iter != obj->end(); ++iter ) {
        Value val = *iter;
        cJSON* res = toJSON( val );
        if ( res != nullptr )
            cJSON_AddItemToArray( array, res );
    }
    
    return array;
}


cJSON* cJSONHelper::toJSONMap( RefMap* obj )
{
    cJSON* map = cJSON_CreateObject();
    RefMap::const_iterator iter = obj->begin();
    for ( ; iter != obj->end(); ++iter ) {
        Ref* pValue = iter->second;
        addToJSONMap( map, iter->first, pValue );
    }
    
    return map;
}

cJSON* cJSONHelper::toJSONMap( ValueMap* obj )
{
    cJSON* map = cJSON_CreateObject();
    ValueMap::const_iterator iter = obj->begin();
    for ( ; iter != obj->end(); ++iter ) {
        Value value = iter->second;
        addToJSONMap( map, iter->first, value );
    }
    
    return map;
}

void cJSONHelper::addToJSONMap( cJSON* node, const std::string& keyName, Value value )
{
    cJSON* res = toJSON( value );
    if ( res != nullptr )
        cJSON_AddItemToObject( node, keyName.c_str(), res );
}

void cJSONHelper::addToJSONMap( cJSON* node, const std::string& keyName, Ref* pValue )
{
    
    if ( dynamic_cast<RefValue*>(pValue) != NULL ) {
        RefValue* val = dynamic_cast<RefValue*>(pValue);
        
        cJSON* res = toJSON( *val );
        if ( res != nullptr )
            cJSON_AddItemToObject( node, keyName.c_str(), res );
    } else if ( dynamic_cast<RefValueVector*>(pValue) != NULL ) {
        RefValueVector* val = dynamic_cast<RefValueVector*>(pValue);
        cJSON_AddItemToObject( node, keyName.c_str(), toJSONArray( val ) );
    } else if ( dynamic_cast<RefValueMap*>(pValue) != NULL ) {
        RefValueMap* val = dynamic_cast<RefValueMap*>(pValue);
        cJSON_AddItemToObject( node, keyName.c_str(), toJSONMap( val ) );
    } else if ( dynamic_cast<RefMap*>(pValue) != NULL ) {
        RefMap* val = dynamic_cast<RefMap*>(pValue);
        cJSON_AddItemToObject( node, keyName.c_str(), toJSONMap( val ) );
    } else if ( dynamic_cast<RefVector*>(pValue) != NULL ) {
        RefVector* val = dynamic_cast<RefVector*>(pValue);
        cJSON_AddItemToObject( node, keyName.c_str(), toJSONArray( val ) );
    } else if ( dynamic_cast<DroiObject*>(pValue) ) {
        DroiObject* droiObject = dynamic_cast<DroiObject*>(pValue);
        cJSON_AddItemToObject( node, keyName.c_str(), cJSONHelper::toJSON(droiObject) );
    }
}
//
cJSON* cJSONHelper::toJSON( DroiObject* obj )
{
    cJSON *root = cJSON_CreateObject();
    
    // Put DroiObject tag
    cJSON_AddStringToObject( root, gszJSON_CLASSNAME, obj->mClassName.c_str() );
    cJSON_AddStringToObject( root, gszJSON_CREATION_TIME, DroiDateTime::toISO8601String( obj->mCreationTime ).c_str() );
    cJSON_AddStringToObject( root, gszJSON_MODIFIED_TIME, DroiDateTime::toISO8601String( obj->mModifyTime ).c_str() );
    cJSON_AddStringToObject( root, gszJSON_OBJECTID, obj->mObjectId.c_str() );
    if ( obj->getPermissionWithDefault() != NULL ) {
        cJSON* perm = obj->getPermissionWithDefault()->toJSONObject();
        if ( perm )
            cJSON_AddItemToObject( root, gszJSON_PERMISSION, perm );
    }
    
    RefMap::const_iterator iter = obj->mProperties->begin();
    for (; iter != obj->mProperties->end(); ++iter ) {
        Ref* pValue = iter->second;
        addToJSONMap( root, iter->first, pValue );
    }
    return root;
}
