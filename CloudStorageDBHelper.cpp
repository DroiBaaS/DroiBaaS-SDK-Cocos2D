/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

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

CloudStorageDBHelper* CloudStorageDBHelper::mInstance = nullptr;

CloudStorageDBHelper* CloudStorageDBHelper::instance()
{
    if ( mInstance == nullptr ) {
        mInstance = new CloudStorageDBHelper();
    }
    
    return mInstance;
}

bool CloudStorageDBHelper::createTable( DroiObject* object, const std::string& clazzName )
{
    return false;
}

DroiError CloudStorageDBHelper::insert( DroiMultimap<Ref*>* commands )
{
    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    DroiError err = DroiError::createDroiError(DROICODE_OK, "");
    if ( DroiUser::isAutoAnonymousUserEnabled() && (currentUser == nullptr || !currentUser->isLoggedIn()) ) {
        currentUser = DroiUser::loginWithAnonymous(&err);
        if ( err.isOk() == false ) {
            return err;
        }
    }
    
    RefVector* args = (RefVector*) commands->getElement( DroiQuery::INSERT, 0 );
    if ( args == nullptr || args->size() != 2 ) {
        err = DroiError::createDroiError( DROICODE_INVALID_PARAMETER, "" );
        return err;
    }
    
    DroiObject* droiObject = dynamic_cast<DroiObject*>(args->at(0));
    if ( droiObject->isDirty() ) {
        err = save( droiObject, droiObject->getClassName(), DroiQuery::INSERT );
    }
    
    return err;
}

RefVector* CloudStorageDBHelper::query( DroiMultimap<Ref*>* commands, DroiError* error )
{
    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    DroiError err;
    if ( DroiUser::isAutoAnonymousUserEnabled() && (currentUser == nullptr || !currentUser->isLoggedIn()) ) {
        currentUser = DroiUser::loginWithAnonymous(&err);
        if ( err.isOk() == false ) {
            if ( error != nullptr )
                *error = err;
            return nullptr;
        }
    }

    string className = "";
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
        className = value->asString();
    }

    RefVector* result = new RefVector();
    std::string where = generateWhere(commands);
    std::string order = generateOrder(commands);
    int offset = getOffsetLimit(commands, DroiQuery::OFFSET);
    int limit = getOffsetLimit(commands, DroiQuery::LIMIT);
    string queryString = combineQuery(where, order, offset, limit);
    if (queryString.size() <= 0) {
        err = DroiError::createDroiError( DROICODE_ERROR, "make query fail.");
        if ( error != nullptr )
            *error = err;
        return nullptr;
    }

    std::string url = DroiRESTfulAPIDefinition::CRUD + "/" + className + "?" + queryString;
    std::string res = DroiRemoteServiceHelper::callServer(string(url), cocos2d::network::HttpRequest::Type::GET, nullptr, &err);

    if ( !res.empty() ) {
        RefPtrAutoReleaser<RefMap> resultDict = cJSONHelper::fromJSON( res );
        if ( resultDict != nullptr ) {
            int code = ((RefValue*) resultDict->at("Code"))->asInt();
            if ( code == 0 ) {
                RefPtr<RefVector> resultObj = dynamic_cast<RefVector*>(resultDict->at("Result"));
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

DroiError CloudStorageDBHelper::update( DroiMultimap<Ref*>* commands )
{
    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    DroiError err = DroiError::createDroiError(DROICODE_OK, "");
    if ( DroiUser::isAutoAnonymousUserEnabled() && (currentUser == nullptr || !currentUser->isLoggedIn()) ) {
        currentUser = DroiUser::loginWithAnonymous(&err);
        if ( err.isOk() == false ) {
            return err;
        }
    }

    RefPtr<RefVector> args = commands->getValue( DroiQuery::UPDATE );
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

DroiError CloudStorageDBHelper::updateData( DroiMultimap<Ref*>* commands )
{
    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    DroiError err;
    if ( DroiUser::isAutoAnonymousUserEnabled() && (currentUser == nullptr || !currentUser->isLoggedIn()) ) {
        currentUser = DroiUser::loginWithAnonymous(&err);
        if ( err.isOk() == false ) {
            return err;
        }
    }
    
    RefPtr<RefVector> args = commands->getValue( DroiQuery::UPDATE_DATA );
    if ( args == nullptr || args->size() != 1 )
        return DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "");

    // Backup item data
    RefPtr<RefVector> incItem = commands->getValue( DroiQuery::INC );
    RefPtr<RefVector> decItem = commands->getValue( DroiQuery::DEC );
    RefPtr<RefVector> setItem = commands->getValue( DroiQuery::SET );
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
    
    RefPtrAutoReleaser<RefVector> result = query( commands, &err );
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
            RefPtr<RefVector> setArray = dynamic_cast<RefVector*>(incItem->at(0));
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

DroiError CloudStorageDBHelper::deleteOp( DroiMultimap<Ref*>* commands )
{
    RefPtr<RefVector> args = commands->getValue( DroiQuery::DELETE );
    if ( args == nullptr || args->size() != 1 )
        return DroiError::createDroiError(DROICODE_INVALID_PARAMETER, "");
    
    DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
    RefVector* value = (RefVector* )entry->Value();
    DroiObject* droiObject = dynamic_cast<DroiObject*>(value->at(0));
    RefValue* tRef = dynamic_cast<RefValue *>(value->at(1));
    std::string tableName  = tRef->asString();

    return deleteInternal( droiObject, tableName );
}

// Private methods
DroiError CloudStorageDBHelper::deleteInternal( DroiObject* obj, const std::string& tableName )
{

    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    DroiError err;
    if ( DroiUser::isAutoAnonymousUserEnabled() && (currentUser == nullptr || !currentUser->isLoggedIn()) ) {
        currentUser = DroiUser::loginWithAnonymous(&err);
        if ( err.isOk() == false ) {
            return err;
        }
    }
    std::string url = DroiRESTfulAPIDefinition::CRUD + "/" + tableName + "/" + obj->objectId();
    std::string res = DroiRemoteServiceHelper::callServer(string(url), cocos2d::network::HttpRequest::Type::DELETE, nullptr, &err);

    if ( !err.isOk() ) return err;

    if ( !res.empty() ) {
        RefPtrAutoReleaser<DroiObject> obj = translateResponse(res);
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

DroiError CloudStorageDBHelper::save( DroiObject* obj, const std::string& tableName, const std::string& op )
{
    RefPtrAutoReleaser<DroiUser> currentUser = DroiUser::getCurrentUser();
    if ( currentUser == nullptr || !currentUser->isLoggedIn() ) {
        return DroiError::createDroiError( DROICODE_ERROR, "", "Not active user login" );
    }

    DroiError err;
    std::string objStr;
    if(obj->toJson(objStr) == false) {
        return DroiError::createDroiError(DROICODE_ERROR, "", "Cannot conver target to Json string");
    }
    RefPtrAutoReleaser<RefValue> target = new RefValue(Value(objStr));

    std::string url = DroiRESTfulAPIDefinition::CRUD + "/" + tableName + "/" + obj->objectId();
    std::string res = DroiRemoteServiceHelper::callServer(string(url), cocos2d::network::HttpRequest::Type::PUT, target.get(), &err);
    if ( !res.empty() ) {
        RefPtrAutoReleaser<DroiObject> obj = translateResponse(res);
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

RefMap* CloudStorageDBHelper::travel(DroiCondition* cond)
{
    RefMap* res = new RefMap();
    std::string type = cond->getType();
    RefVector* conditions = cond->getValue();

    if ( type == DroiQuery::COND) {
        RefVector* oriArgs = dynamic_cast<RefVector*>(conditions->at(0));
        RefPtrAutoReleaser<RefVector> args = convertArgumentsFormat(oriArgs);
        res->insert(type, args);
    } else {
        RefPtrAutoReleaser<RefVector> values = new RefVector();
        RefVector::const_iterator iter = conditions->begin();
        for ( ; iter != conditions->end(); ++iter) {
            if ( dynamic_cast<RefVector*>( *iter) != nullptr) {
                values->pushBack(*iter);
            } else if (dynamic_cast<DroiCondition*>(*iter) != nullptr) {
                RefPtrAutoReleaser<RefMap> input = travel(dynamic_cast<DroiCondition*>(*iter) );
                values->pushBack( input );
            }
        }
        res->insert(type, values);
    }

    return res;
}


DroiObject* CloudStorageDBHelper::translateResponse(std::string res)
{
    DroiObject* object = DroiObject::createObject("DROIRESTful");
    RefPtrAutoReleaser<RefMap> jsonMap = dynamic_cast<RefMap*>(cJSONHelper::fromJSON(res));
//    if ( jsonMap == nullptr ) {
//        CCLOG( "%s", res.c_str() );
//        return nullptr;
//    }
    RefValue* refCode = nullptr;
    RefValue* refTicket = nullptr;
    RefMap::const_iterator it = jsonMap->find(DROI_KEY_JSON_CODE);
    if ( it != jsonMap->end() ) {
        if (dynamic_cast<RefValue*>(it->second) != nullptr) {
            refCode = dynamic_cast<RefValue*>(it->second);
        }
    }

    it = jsonMap->find(DROI_KEY_JSON_TICKET);
    if ( it != jsonMap->end() ) {
        if (dynamic_cast<RefValue*>(it->second) != nullptr) {
            refTicket = dynamic_cast<RefValue*>(it->second);
        }
    }

    //TODO
    int code = -1;
    if (refCode != nullptr)
        code = refCode->asInt();
    object->setValue(DROI_KEY_JSON_CODE, code);
    if (refTicket != nullptr)
        object->setValue(DROI_KEY_JSON_TICKET, refTicket->asString());

    return object;
}

std::string CloudStorageDBHelper::generateWhere(DroiMultimap<Ref*>* commands ) {
    std::string data("");
    if (!commands->containsKey(DroiQuery::WHERE))
        return data;

    Ref* element = getFirstElement(commands, DroiQuery::WHERE);
    DroiCondition* cond = getSingleCondition(&element);

    if (cond != nullptr) {
        RefPtrAutoReleaser<RefMap> obj = new RefMap();
        std::string type = cond->getType();
        RefVector* conditions = cond->getValue();

        if (type.compare(DroiQuery::COND) == 0) {
            RefVector* oriArgs = dynamic_cast<RefVector*>(conditions->at(0));
            RefPtrAutoReleaser<RefVector> args = convertArgumentsFormat( oriArgs );
            RefPtrAutoReleaser<RefMap> cond = new RefMap();
            cond->insert(dynamic_cast<RefValue*>(args->at(1))->asString(), args->at(2));
            obj->insert(dynamic_cast<RefValue*>(args->at(0))->asString(), cond);
        } else {
            obj = travel( cond );
        }
        cJSON* jsondata = cJSONHelper::toJSONMap(dynamic_cast<RefMap*>(obj.get()));
        char* res = cJSON_PrintUnformatted(jsondata);
        data = res;
        free( res );
        cJSON_Delete(jsondata);
    }
    return data;
}

std::string CloudStorageDBHelper::generateOrder(DroiMultimap<Ref*>* commands ) {
    std::string data("");
    if (!commands->containsKey(DroiQuery::ORDERBY))
        return data;

    RefVector* list = commands->getValue(DroiQuery::ORDERBY);
    if (list == nullptr || list->size() < 1)
        return data;

    RefVector::iterator it = list->begin();
    for (; it != list->end(); ++it) {
        Ref* current = dynamic_cast<Ref*>(*it);
        if (current == nullptr)
            continue;
        DroiMapEntry<Ref*>* entry = nullptr;

        if (dynamic_cast<DroiMapEntry<Ref*>*>(current) != nullptr)
            entry = dynamic_cast<DroiMapEntry<Ref*>*>(current);

        RefValueVector* target = dynamic_cast<RefValueVector*>(entry->Value());
        if (DroiQuery::DESC.compare(target->at(1).asString()) == 0)
            data.append("-");
        data.append(target->at(0).asString());
        data.append(",");
    }

    data.replace(data.length()-1, 1, "");

    return data;
}

int CloudStorageDBHelper::getOffsetLimit(DroiMultimap<Ref*>* commands, std::string keyName) {
    int res = 0;
    if (!commands->containsKey(keyName))
        return res;

    Ref* element = getFirstElement(commands, keyName);
    RefValue* val = nullptr;
    DroiMapEntry<Ref*>* entry = nullptr;

    if (dynamic_cast<DroiMapEntry<Ref*>*>(element) != nullptr)
        entry = dynamic_cast<DroiMapEntry<Ref*>*>(element);

    if (entry != nullptr && dynamic_cast<RefValue*>(entry->Value()) != nullptr)
        val = dynamic_cast<RefValue*>(entry->Value());

    res = val->asInt();

    return res;
}


Ref* CloudStorageDBHelper::getFirstElement(DroiMultimap<Ref*>* commands, std::string keyName) {
    RefVector* vec = commands->getValue(keyName);
    if (vec->size() != 1)
        return nullptr;

    return vec->at(0);
}

DroiCondition* CloudStorageDBHelper::getSingleCondition(Ref** input) {
    DroiCondition* cond = nullptr;
    DroiMapEntry<Ref*>* entry = nullptr;

    if (dynamic_cast<DroiMapEntry<Ref*>*>(*input) != nullptr)
        entry = dynamic_cast<DroiMapEntry<Ref*>*>(*input);

    if (entry != nullptr && dynamic_cast<DroiCondition*>(entry->Value()) != nullptr)
        cond = dynamic_cast<DroiCondition*>(entry->Value());

    return cond;
}


RefVector* CloudStorageDBHelper::convertArgumentsFormat(RefVector* args ) {
    RefVector* newArgs = new RefVector(*args);
    if (args->size() <= 2)
        return args;

    Ref* arg = args->at(2);
    if ( dynamic_cast<DroiDateTime*>(arg) != nullptr) {
        DroiDateTime* dt = dynamic_cast<DroiDateTime*>( args->at(2) );
        RefValue* tm = new RefValue( Value(dt->toISO8601String()) );
        newArgs->replace(2, tm);
    }

    return newArgs;
}

std::string CloudStorageDBHelper::combineQuery(std::string where, std::string order, int offset, int limit) {
    std::string sb("");
    if (where.size() < 1 && order.size() < 1)
        return sb;

    if (where.size() >= 1) {
        sb.append("where=");
        sb.append(DroiHelper::url_encode(where));
        sb.append(("&"));
    }

    if (order.size() >= 1) {
        sb.append("order=");
        sb.append(DroiHelper::url_encode(order));
        sb.append("&");
    }

    sb.append("offset=");
    sb.append(std::to_string(offset));
    sb.append("&");

    sb.append("limit=");
    sb.append(std::to_string(limit));

    return sb;

}
