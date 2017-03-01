/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiObject.h"
#include "DroiDateTime.h"
#include "cJSON.h"
#include "cJSONHelper.h"
#include "DroiPermission.h"
#include "DroiHelper.h"
#include "DroiTaskDispatcher.h"
#include "DroiQuery.h"

using namespace cocos2d;

const char* gszJSON_CLASSNAME = "_ClassName";
const char* gszJSON_OBJECTID = "_Id";
const char* gszJSON_CREATION_TIME = "_CreationTime";
const char* gszJSON_MODIFIED_TIME = "_ModifiedTime";
const char* gszJSON_PERMISSION = "_ACL";

classFactory DroiObject::registerdClasses;

//===========================================================================================================
// DroiValue
//
class DroiValue : public Ref, Value
{
public:
    DroiValue() { }
};

DroiObject* DroiObject::createObject( std::string className )
{
    DroiObject* obj = new DroiObject();
    obj->mClassName = className;
    return obj;
}

//==========================================================================
// Getter/Setter
//
void DroiObject::setValue( const string& keyName, const cocos2d::Value& value )
{
    eraseValue(keyName);
    RefValue* res = new RefValue(value);
    mProperties->insert( keyName, res );
    res->release();
}

void DroiObject::setValue( const string& keyName, const cocos2d::ValueVector& value )
{
    eraseValue(keyName);
    RefValueVector* vec = new RefValueVector(value);
    mProperties->insert( keyName, vec );
    vec->release();
}

void DroiObject::setValue( const string& keyName, const cocos2d::ValueMap& value )
{
    eraseValue(keyName);
    RefValueMap* map = new RefValueMap(value);
    mProperties->insert( keyName, map );
    map->release();
}

void DroiObject::setValue( const string& keyName, const RefVector& value )
{
    eraseValue(keyName);
    RefVector* res = new RefVector(value);
    mProperties->insert( keyName, res );
    res->release();
}

void DroiObject::setValue( const string& keyName, const RefMap& value )
{
    eraseValue(keyName);
    RefMap* res = new RefMap(value);
    mProperties->insert( keyName, res );
    res->release();
}

void DroiObject::setValue( const string& keyName, DroiObject* value )
{
    eraseValue(keyName);
    mProperties->insert( keyName, value );
}

void DroiObject::setValue( const string& keyName, DroiDateTime value )
{
    eraseValue(keyName);
    DroiDateTime* res = new DroiDateTime(value);
    mProperties->insert( keyName, res );
    res->release();
}

void DroiObject::eraseValue( const string& keyName )
{
    mProperties->erase(keyName);
}

bool DroiObject::hasValue( const std::string& keyName )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    return true;
}

bool DroiObject::getValue( const string& keyName, cocos2d::Value& value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    RefValue* refValue = dynamic_cast<RefValue*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = *refValue;
    return true;
}

bool DroiObject::getValue( const string& keyName, cocos2d::ValueVector& value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    RefValueVector* refValue = dynamic_cast<RefValueVector*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = *refValue;
    return true;
}

bool DroiObject::getValue( const string& keyName, cocos2d::ValueMap& value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    RefValueMap* refValue = dynamic_cast<RefValueMap*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = *refValue;
    return true;
}

bool DroiObject::getValue( const string& keyName, RefVector& value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    RefVector* refValue = dynamic_cast<RefVector*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = *refValue;
    return true;
}

bool DroiObject::getValue( const string& keyName, RefMap& value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    RefMap* refValue = dynamic_cast<RefMap*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = *refValue;
    return true;
}

bool DroiObject::getValue( const string& keyName, DroiObject* & value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    DroiObject* refValue = dynamic_cast<DroiObject*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = refValue;
    value->retain();
    return true;
}

bool DroiObject::getValue( const string& keyName, DroiDateTime& value )
{
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return false;
    }
    
    Ref* val = iter->second;
    DroiDateTime* refValue = dynamic_cast<DroiDateTime*>(val);
    if ( refValue == nullptr )
        return false;
    
    value = *refValue;
    return true;
}


void DroiObject::setValue( const std::string& keyName, bool value )
{
    // Create new RefValue object
    RefPtrAutoReleaser<RefValue> val = new RefValue( Value(value) );
    
    // Insert
    mProperties->insert( keyName, val.get() );
}

void DroiObject::setValue( const std::string& keyName, int32_t value )
{
    // Create new RefValue object
    RefPtrAutoReleaser<RefValue> val = new RefValue( Value(value) );
    
    // Insert
    mProperties->insert( keyName, val.get() );

}

void DroiObject::setValue( const std::string& keyName, const std::string& value )
{
    // Create new RefValue object
    RefPtrAutoReleaser<RefValue> val = new RefValue( Value(value) );
    
    // Insert
    mProperties->insert( keyName, val.get() );

}

void DroiObject::setValue( const std::string& keyName, double value )
{
    // Create new RefValue object
    RefPtrAutoReleaser<RefValue> val = new RefValue( Value(value) );
    
    // Insert
    mProperties->insert( keyName, val.get() );

}

bool DroiObject::getValue( const std::string& keyName, bool& value )
{
    Value val;
    if ( getValue( keyName, val ) == false )
        return false;
    
    value = val.asBool();
    return true;
}

bool DroiObject::getValue( const std::string& keyName, int32_t& value )
{
    Value val;
    if ( getValue( keyName, val ) == false )
        return false;
    
    value = val.asInt();
    return true;
}

bool DroiObject::getValue( const std::string& keyName, double& value )
{
    Value val;
    if ( getValue( keyName, val ) == false )
        return false;
    
    value = val.asDouble();
    return true;
}

bool DroiObject::getValue( const std::string& keyName, std::string& value )
{
    Value val;
    if ( getValue( keyName, val ) == false )
        return false;

    value = val.asString();
    return true;
}

bool DroiObject::isDirty()
{
    return mDirty;
}

DroiError DroiObject::save()
{
    std::string res;
    DroiError err;
    if ( toJson( res ) ) {
        // TODO: Local or Cloud
        mDirty = true;

        this->mModifyTime = DroiDateTime::now();
        AutoReleaser<DroiQuery> query  = DroiQuery::create();
        query->updateObject(this->getClassName(), *this);
        query->runQuery(&err);

        // Reset dirty flag if the operation is finished
        if ( err.isOk() )
            mDirty = false;
    }

    return err;
}

/**
 * Save current object to cloud storage in bacgkround thread
 * \param callback The DroiCallback object
 */
void DroiObject::saveInBackground( DroiCallback<DroiError>::onCallback callback )
{
    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher( DroiTaskBackgroundThread );
    DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();
    
    td.enqueueTask([this, callback, currentTD] {
        DroiError err =  save();
        
        currentTD->enqueueTask([err, callback] {
            callback( err );
        });        
    });

}

DroiError DroiObject::deleteFromStoage()
{
    std::string res;
    DroiError err;
    if ( toJson( res ) ) {
        // TODO: Local or Cloud

        this->mModifyTime = DroiDateTime::now();
        AutoReleaser<DroiQuery> query  = DroiQuery::create();
        query->deleteData(this->getClassName(), *this);
        query->runQuery(&err);
    }

    return err;
}


void DroiObject::set( const std::string& keyName, Ref* refValue )
{
    mProperties->insert( keyName, refValue );
    mDirty = true;
}

Ref* DroiObject::get( const std::string& keyName )
{
    // Get value
    RefMap::iterator iter = mProperties->find( keyName );
    if ( iter == mProperties->end() ) {
        return nullptr;
    }
    
    // Return
    Ref* val = iter->second;
    return val;
}


bool DroiObject::toJson( std::string& result )
{
    cJSON* json = cJSONHelper::toJSON( this );
    // TODO:Local or Cloud
    char* res = cJSON_PrintUnformatted( json );
    result = res;
    free(res);
    cJSON_Delete( json );
    return true;
}

DroiObject* DroiObject::fromJSON( const std::string& jsonString )
{
    cJSON* json = cJSON_Parse( jsonString.c_str() );
    if ( json == NULL )
        return nullptr;
    
    //
    DroiObject* dObject = cJSONHelper::fromJSON( json );
    cJSON_Delete( json );
    return dObject;
}

DroiError DroiObject::saveAll( RefVector* items )
{
    if ( items == nullptr || items->size() == 0 )
        return DroiError::createDroiError( DROICODE_INVALID_PARAMETER, "" );
    
    DroiError err = DroiError::createDroiError( DROICODE_OK, "" );
    for ( auto autoObj : *items ) {
        DroiObject* obj = dynamic_cast<DroiObject*>(autoObj);
        if ( !(err = obj->save()).isOk())
            return err;
    }
    return err;
}

DroiObject::DroiObject( const std::string& className )
: DroiObject()
{
    mClassName = className;
}

void find_and_replace(string& source, string const& find, string const& replace)
{
    for(string::size_type i = 0; (i = source.find(find, i)) != string::npos;)
    {
        source.replace(i, find.length(), replace);
        i += replace.length();
    }
}

DroiObject::DroiObject()
{
    mCreationTime = mModifyTime = DroiDateTime::now();
    mpPermission = nullptr;
    mDirty = true;
    
    // UUID
    mObjectId = DroiHelper::generateUUID().substr(8);
    find_and_replace(mObjectId, "-", "");
    mProperties = new RefMap();
}

DroiPermission* DroiObject::getPermissionWithDefault() const
{
    if (mpPermission)
        return mpPermission;

    //Return default permission
    DroiPermission* permission = DroiPermission::getDefaultPermission();
    if (permission != nullptr)
        return permission;
    else
        return new DroiPermission;
}

void DroiObject::setPermission( DroiPermission* permission )
{
    if ( mpPermission )
        delete mpPermission;
    mpPermission = permission->clone();
}

DroiPermission* DroiObject::getPermission()
{
    return mpPermission;
}

DroiObject* DroiObject::clone()
{
    DroiObject* clone = DroiObject::createObject( mClassName );
    *clone = *this;
    
    // Clone all items in keyValue lists
    clone->mProperties->clear();
    
    //
    *(clone->mProperties) = *mProperties;
    
    //
    if ( mpPermission != NULL ) {
        clone->mpPermission = new DroiPermission();
        *(clone->mpPermission) = *mpPermission;
    }
    return clone;
}

void DroiObject::copyFrom(const DroiObject* obj)
{

    mObjectId = obj->mObjectId;
    mCreationTime = obj->mCreationTime;
    mModifyTime = obj->mModifyTime;
    *mProperties = *(obj->mProperties);
    if ( mpPermission != NULL ) {
        mpPermission = new DroiPermission();
        *mpPermission = *(obj->mpPermission);
    }
    mDirtyFlags = obj->mDirtyFlags;
}

DroiObject::~DroiObject()
{
    // Release all allocated memory
//    mProperties.clear();
    CC_SAFE_RELEASE_NULL(mProperties);

    //
    if ( mpPermission != NULL ) {
        delete mpPermission;
    }
}

