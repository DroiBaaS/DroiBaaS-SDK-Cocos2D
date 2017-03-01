/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DROI_OBJECT_H__
#define __DROI_OBJECT_H__
#include "RefValue.h"
#include "DroiError.h"
#include "DroiCallback.h"

using namespace std;
using namespace cocos2d;

class DroiDateTime;
class DroiPermission;
class cJSONHelper;
class DroiObject;
struct cJSON;

typedef DroiObject* (*createObject)();
typedef std::map<std::string, createObject> classFactory;

/**
 * The DroiObject is the basic data type that can be saved and retrieved from the droi cloud service.
 */
class DroiObject : public cocos2d::Ref {
public:
    virtual ~DroiObject();
    
    /**
     * Register custom droiobject
     * \param className The class name of created object.
     * \param factory The create function of custom class.
     * \return DroiObject instance
     */
    static void registerCustomClass(std::string classname, createObject factory);

    /**
     * Create DroiObject object with className
     * \param className The class name of created object.
     * \return DroiObject instance
     */
	static DroiObject* createObject( std::string className );
    
    /**
     * Get class name of object
     * \return The class name of object.
     */
    std::string getClassName() const { return mClassName; }
    
    //==========================================================================
    // Getter/Setter
    // For normal data type
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The boolean value
     */
    void setValue( const std::string& keyName, bool value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The integer value
     */
    void setValue( const std::string& keyName, int32_t value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The integer value
     */
    void setValue( const std::string& keyName, const std::string& value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The double value
     */
    void setValue( const std::string& keyName, double value );
    
    // For normal cocos2d::Value
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link cocos2d::Value \endlink value
     */
    void setValue( const string& keyName, const cocos2d::Value& value );

    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link cocos2d::ValueVector \endlink value
     */
    void setValue( const string& keyName, const cocos2d::ValueVector& value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link cocos2d::ValueMap \endlink value
     */
    void setValue( const string& keyName, const cocos2d::ValueMap& value );
    
    // For Droi data type
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link RefVector \endlink value
     */
    void setValue( const string& keyName, const RefVector& value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link RefMap \endlink value
     */
    void setValue( const string& keyName, const RefMap& value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link DroiObject \endlink value
     */
    void setValue( const string& keyName, DroiObject* value );
    
    /**
     *  Set a key-value pair into DroiObject
     * \param keyName The key name
     * \param value The \link DroiDateTime \endlink value
     */
    void setValue( const string& keyName, DroiDateTime value );
    
    /**
     *  Remove a value by key from DroiObject
     * \param keyName The key name
     */
    void eraseValue( const string& keyName );

    //=============================================================================
    // For normal data type
    /**
     * Check whether this keyName exists
     * \return true This keyName is in key-value pair; Otherwise is false
     */
    bool hasValue( const std::string& keyName );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of boolean 
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, bool& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of integer
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const std::string& keyName, int32_t& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of std::string
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const std::string& keyName, std::string& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of double
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const std::string& keyName, double& value );
    
    // For normal cocos2d::Value
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of cocos2d::Value
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, cocos2d::Value& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of cocos2d::ValueVector
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, cocos2d::ValueVector& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of cocos2d::ValueMap
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, cocos2d::ValueMap& value );
    
    // For Droi data type
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of RefVector
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, RefVector& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of RefMap
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, RefMap& value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of DroiObject
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, DroiObject* & value );
    
    /**
     * Get value by specific keyName
     * \param keyName The key name
     * \param value The reference value of DroiDateTime
     * \return true Get value succeeded; Otherwise is false
     */
    bool getValue( const string& keyName, DroiDateTime& value );
    
    /**
     * Check whether the object is dirty
     * \return true This object is dirty; Otherwise is false
     */
    bool isDirty();

    //
    
    // Member value
    /**
     * Get object identifier
     * \return The string of object identifier
     */
    std::string objectId() const { return mObjectId; }
    
    /**
     * Get default permission object
     * \return The default permission object
     */
    DroiPermission* getPermissionWithDefault() const;
    
    /**
     * Set permission object
     * \param permission The permission object
     */
    void setPermission( DroiPermission* permission );
    DroiPermission* getPermission();

    //
    /**
     * Clone current object
     * \return The clone object
     */
    DroiObject* clone();
    
    /**
     * Save current object to cloud storage
     * \return DroiErr that indicate the error code
     */
    virtual DroiError save();

    /**
     * Delete current object from cloud storage
     * \return DroiErr that indicate the error code
     */
    virtual DroiError deleteFromStoage();
    
    /**
     * Save current object to cloud storage in bacgkround thread
     * \param callback The DroiCallback object 
     */
    void saveInBackground( DroiCallback<DroiError>::onCallback callback );
    
    /**
     * Create DroiObject from specific json string
     * \param jsonString The json string of DroiObject
     * \return The DroiObject instance by json string
     */
    static DroiObject* fromJSON( const std::string& jsonString );
    
    /**
     * Save all DroiObject objects
     */
    static DroiError saveAll( RefVector* items );

    /**
     * Convert this to Json string
     */
    bool toJson( std::string& result );
    
protected:
    DroiObject( const std::string& className );


    void set( const std::string& keyName, Ref* refValue );
    Ref* get( const std::string& keyName );
    void copyFrom(const DroiObject* obj);

private:
	DroiObject();
    static classFactory registerdClasses;

    //
    bool mDirty;
    DroiPermission* mpPermission;
    friend class cJSONHelper;

    std::string mClassName;
    RefMap* mProperties;
    std::string mObjectId;
    std::chrono::milliseconds mCreationTime;
    std::chrono::milliseconds mModifyTime;
    int32_t mDirtyFlags;
};

#endif
