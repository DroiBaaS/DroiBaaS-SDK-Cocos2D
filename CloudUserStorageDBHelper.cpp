/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "CloudUserStorageDBHelper.h"
#include "CloudStorageDBHelper.h"
#include "DroiObject.h"
#include "DroiError.h"
#include "DroiUser.h"
#include "DroiQuery.h"
#include "DroiDefinition.h"
#include "DroiRemoteServiceHelper.h"
#include "DroiRESTfulAPIDefinition.h"
#include "DroiHelper.h"
#include "cJSONHelper.h"
#include "cJSON.h"

#define DROITOKENINVALID 1040006

extern const char* gszJSON_CLASSNAME;
extern const char* gszJSON_OBJECTID;
extern const char* gszJSON_CREATION_TIME;
extern const char* gszJSON_MODIFIED_TIME;
extern const char* gszJSON_PERMISSION;

CloudUserStorageDBHelper* CloudUserStorageDBHelper::mInstance = nullptr;

CloudUserStorageDBHelper* CloudUserStorageDBHelper::instance()
{
    if ( mInstance == nullptr ) {
        mInstance = new CloudUserStorageDBHelper();
    }
    return mInstance;
}

bool CloudUserStorageDBHelper::createTable( DroiObject* object, const std::string& clazzName )
{
    return false;
}

DroiError CloudUserStorageDBHelper::insert( DroiMultimap<Ref*>* commands )
{
    return DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "", "Not allowed to call insert");
}

RefVector* CloudUserStorageDBHelper::query( DroiMultimap<Ref*>* commands, DroiError* error )
{
    string tableName = "";
    DroiError err = DroiError::createDroiError(DROICODE_OK, "");
    if ( commands->containsKey( DroiQuery::SELECT ) ) {
        RefPtr<RefVector> args = commands->getValue( DroiQuery::SELECT );
        if ( args->size() != 1 ) {
            err = DroiError::createDroiError( DROICODE_INVALID_PARAMETER, "" );
            if ( error != nullptr )
                *error = err;
            return nullptr;
        }
        DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
        RefValue* value = (RefValue*)entry->Value();
        tableName = value->asString();
    }

    RefVector* result = new RefVector();
    std::string where = CloudStorageDBHelper::generateWhere(commands);
    std::string order = CloudStorageDBHelper::generateOrder(commands);
    int offset = CloudStorageDBHelper::getOffsetLimit(commands, DroiQuery::OFFSET);
    int limit = CloudStorageDBHelper::getOffsetLimit(commands, DroiQuery::LIMIT);
    string queryString = CloudStorageDBHelper::combineQuery(where, order, offset, limit);
    if (queryString.size() <= 0) {
        err = DroiError::createDroiError( DROICODE_ERROR, "make query fail.");
        if ( error != nullptr )
            *error = err;
        return nullptr;
    }

    std::string url = ((tableName.compare("_User") == 0) ?  DroiRESTfulAPIDefinition::USERS : DroiRESTfulAPIDefinition::GROUPS) + "?" + queryString;
    std::string res = DroiRemoteServiceHelper::callServer(string(url), cocos2d::network::HttpRequest::Type::GET, nullptr, &err);

    if ( !res.empty() ) {
        RefPtrAutoReleaser<RefMap> resultDict = cJSONHelper::fromJSON( res );
        if ( resultDict != nullptr ) {
            int code = ((RefValue*) resultDict->at("Code"))->asInt();
            if ( code == 0 ) {
                RefVector* resultObj = dynamic_cast<RefVector*>(resultDict->at("Result"));
                if ( resultObj == nullptr )
                    return nullptr;

                RefVector::iterator iter = resultObj->begin();
                for (; iter != resultObj->end(); ++iter ) {
                    DroiObject* child = dynamic_cast<DroiObject*>(*iter);
                    if ( child == nullptr)
                        continue;
                    result->pushBack(child);
                }
            } else {
                if ( error != nullptr )
                    *error = DroiError::createDroiError((DroiErrorCode) code, "");
                if ( code == DROITOKENINVALID)
                    DroiUser::cleanUp();
            }
        }
    } else {
        if ( error != nullptr )
            *error = DroiError::createDroiError( DROICODE_OK, "");
    }
    return result;
}

DroiError CloudUserStorageDBHelper::update( DroiMultimap<Ref*>* commands )
{
    DroiError err = DroiError::createDroiError(DROICODE_OK, "");

    RefVector* args = commands->getValue( DroiQuery::UPDATE );
    if ( args == nullptr || args->size() != 1 )
        return DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "");

    DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
    RefVector* value = (RefVector* )entry->Value();
    DroiObject* droiObject = dynamic_cast<DroiObject*>(value->at(0));
    RefValue* tRef = dynamic_cast<RefValue *>(value->at(1));
    std::string tableName  = tRef->asString();
    if ( droiObject->isDirty() == false )
        return err;

    return save( droiObject, tableName, DroiQuery::UPDATE );
}

DroiError CloudUserStorageDBHelper::updateData( DroiMultimap<Ref*>* commands )
{
    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    DroiError err;
    if ( DroiUser::isAutoAnonymousUserEnabled() && (currentUser == nullptr || !currentUser->isLoggedIn()) ) {
        currentUser = DroiUser::loginWithAnonymous(&err);
        if ( err.isOk() == false ) {
            return err;
        }
    }

    RefVector* args = commands->getValue( DroiQuery::UPDATE_DATA );
    if ( args == nullptr || args->size() != 1 )
        return DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "");

    // Backup item data
    RefVector* incItem = commands->getValue( DroiQuery::INC );
    RefVector* decItem = commands->getValue( DroiQuery::DEC );
    RefVector* setItem = commands->getValue( DroiQuery::SET );
    commands->remove( DroiQuery::INC );
    commands->remove( DroiQuery::DEC );
    commands->remove( DroiQuery::SET );

    // Convert type from UpdateData to Select
    string className = "";
    if ( commands->containsKey( DroiQuery::SELECT ) ) {
        RefVector* args = commands->getValue( DroiQuery::SELECT );
        if ( args->size() != 1 ) {
            err = DroiError::createDroiError( DROICODE_INVALID_PARAMETER, "" );
            return err;
        }

        RefValue* value = (RefValue*) args->at(0);
        className = value->asString();
    }
    commands->remove( DroiQuery::UPDATE_DATA );
    RefPtrAutoReleaser<RefValue> op = new RefValue( Value(className) );
    commands->put( DroiQuery::SELECT, op );

    RefPtr<RefVector> result = query( commands, &err );
    RefPtrAutoReleaser<RefVector>updateItems = new RefVector();

    if ( err.isOk() ) {
        if ( incItem != nullptr ) {
            RefValue* firstObject = dynamic_cast<RefValue*>(incItem->at(0));
            std::string keyName = firstObject->asString();
            for ( auto autoObj : *result ) {
                DroiObject* obj = dynamic_cast<DroiObject*>(autoObj);
                if ( obj->hasValue( keyName ) ) {
                    Value val;
                    if ( obj->getValue( keyName, val ) && (val.getType() == Value::Type::INTEGER || val.getType() == Value::Type::DOUBLE) ) {
                        if ( val.getType() == Value::Type::INTEGER ) {
                            int incValue = val.asInt() + 1;
                            obj->setValue( keyName, incValue );
                        } else {
                            double incValue = val.asDouble() + 1.0;
                            obj->setValue( keyName, incValue );
                        }

                        updateItems->pushBack( obj );
                    }
                }
            }
        } else if ( decItem != nullptr ) {
            RefValue* firstObject = dynamic_cast<RefValue*>(incItem->at(0));
            std::string keyName = firstObject->asString();
            for ( auto autoObj : *result ) {
                DroiObject* obj = dynamic_cast<DroiObject*>(autoObj);
                if ( obj->hasValue( keyName ) ) {
                    Value val;
                    if ( obj->getValue( keyName, val ) && (val.getType() == Value::Type::INTEGER || val.getType() == Value::Type::DOUBLE) ) {
                        if ( val.getType() == Value::Type::INTEGER ) {
                            int incValue = val.asInt() - 1;
                            obj->setValue( keyName, incValue );
                        } else {
                            double incValue = val.asDouble() - 1.0;
                            obj->setValue( keyName, incValue );
                        }

                        updateItems->pushBack( obj );
                    }
                }
            }

        } else if ( setItem != nullptr ) {
            RefVector* setArray = dynamic_cast<RefVector*>(incItem->at(0));
            RefValue* firstObject = dynamic_cast<RefValue*>(setArray->at(0));
            std::string keyName = firstObject->asString();
            RefValue* secondObject = dynamic_cast<RefValue*>(setArray->at(1));
            for ( auto autoObj : *result ) {
                DroiObject* obj = dynamic_cast<DroiObject*>(autoObj);
                if ( obj->hasValue( keyName ) ) {
                    Value val;
                    if ( obj->getValue( keyName, val ) ) {
                        obj->setValue( keyName, secondObject );
                        updateItems->pushBack( obj );
                    }
                }
            }
        }

        // Update data
        if ( updateItems->size() > 0 ) {
            err = DroiObject::saveAll( updateItems );
        }
    }
    return err;
}

DroiError CloudUserStorageDBHelper::deleteOp( DroiMultimap<Ref*>* commands )
{
    RefVector* args = commands->getValue( DroiQuery::DELETE );
    if ( args == nullptr || args->size() != 1 )
        return DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "");

    DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
    RefVector* value = (RefVector* )entry->Value();
    DroiObject* droiObject = dynamic_cast<DroiObject*>(value->at(0));
    RefValue* tRef = dynamic_cast<RefValue *>(value->at(1));
    std::string tableName  = tRef->asString();

    return deleteInternal( droiObject, tableName );
}

DroiError CloudUserStorageDBHelper::deleteInternal( DroiObject* obj, const std::string& tableName )
{
    DroiError err;
    std::string url = ((tableName.compare("_User") == 0) ?  DroiRESTfulAPIDefinition::USERS : DroiRESTfulAPIDefinition::GROUPS) + "/" + obj->objectId();
    std::string res = DroiRemoteServiceHelper::callServer(string(url), cocos2d::network::HttpRequest::Type::DELETE, nullptr, &err);

    if ( !err.isOk() ) return err;

    if ( !res.empty() ) {
        RefPtrAutoReleaser<DroiObject> obj = CloudStorageDBHelper::translateResponse(res);
        int code = -1;
        obj->getValue(DROI_KEY_JSON_CODE, code);
        if (code != 0) {
            if (code == DROITOKENINVALID)
                DroiUser::cleanUp();
            std::string ticket;
            bool existed = obj->getValue(DROI_KEY_JSON_TICKET, ticket);
            if (!existed) ticket = "";
            err = DroiError::createDroiError( (DroiErrorCode) code, ticket, std::string("Delete fails, code: ").append(std::to_string(code)));
        } else {
            err = DroiError::createDroiError(DROICODE_OK, "", "");
        }
    }

    return err;
}

DroiError CloudUserStorageDBHelper::save( DroiObject* obj, const std::string& tableName, const std::string& op )
{
    DroiError err;
    std::string objStr;
    if(obj->toJson(objStr) == false) {
        return DroiError::createDroiError(DROICODE_ERROR, "", "Cannot conver target to Json string");
    }
    
    RefPtrAutoReleaser<RefValue> jsonObject = new RefValue(Value(objStr));

    std::string url = ((tableName.compare("_User") == 0) ?  DroiRESTfulAPIDefinition::USERS : DroiRESTfulAPIDefinition::GROUPS) + "/" + obj->objectId();
    std::string res = DroiRemoteServiceHelper::callServer(string(url), cocos2d::network::HttpRequest::Type::PUT, jsonObject.get(), &err);

    if ( !res.empty() ) {
        RefPtrAutoReleaser<DroiObject> obj = CloudStorageDBHelper::translateResponse(res);
        int code = -1;
        obj->getValue(DROI_KEY_JSON_CODE, code);
        if (code != 0) {
            if (code == DROITOKENINVALID)
                DroiUser::cleanUp();
            std::string ticket;
            bool existed = obj->getValue(DROI_KEY_JSON_TICKET, ticket);
            if (!existed) ticket = "";
            err = DroiError::createDroiError( (DroiErrorCode) code, ticket, std::string("Save fails, code: ").append(std::to_string(code)));
        } else {
            err = DroiError::createDroiError(DROICODE_OK, "", "");
        }
    }

    return err;
}
