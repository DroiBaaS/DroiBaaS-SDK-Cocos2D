/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiCloud_H__
#define __DroiCloud_H__
#include "DroiCallback.h"
#include "DroiError.h"
#include "DroiUser.h"
#include "cJSONHelper.h"
#include "DroiTaskDispatcher.h"
#include "RefValue.h"

class DroiCloud
{
public:
    template <typename T, typename V> static RefPtr<V> callCloudService( const std::string& name, RefPtr<T> parameter, DroiError* err = nullptr ) {
        static_assert(std::is_base_of<DroiObject, T>::value, "T must inherit from DroiObject");
        static_assert(std::is_base_of<DroiObject, V>::value, "V must inherit from DroiObject");
        RefPtr<DroiUser> currentUser = DroiUser::getCurrentUser();
        DroiError error;
        if ( currentUser->isAutoAnonymousUserEnabled() && !currentUser->isAuthorizedUser() ) {
            currentUser = DroiUser::loginWithAnonymous(&error);
            if ( error.isOk() == false ) {
                if ( err != nullptr )
                    *err = error;
                return nullptr;
            }
        }
        
        cJSON* jsonObject = cJSONHelper::toJSON( dynamic_cast<DroiObject*>(parameter) );
        char* resJson = cJSON_PrintUnformatted( jsonObject );
        std::string json = resJson;
        free(resJson);
        cJSON_Delete( jsonObject );

        // TODO:
        std::string res;
//        NSData* res = [DroiRemoteServiceHelper callServer:[NSString stringWithFormat:@"%@/%@", DroiRESTfulAPIDefinition_CLOUDCODEROOT, name] withJsonData:inputData error:&err];
        if ( !error.isOk() ) {
            if ( err != nullptr )
                *err = error;
            return nullptr;
        }
        
        //
        RefPtrAutoReleaser<RefMap> jsonDict = cJSONHelper::fromJSON( res );
        int code = (dynamic_cast<RefValue*>(jsonDict->at( "Code" ))->asInt());
        std::string ticket = (dynamic_cast<RefValue*>(jsonDict->at( "Ticket" ))->asString());
        if ( code == 0 ) {
            RefPtr<V> resultDict = dynamic_cast<V*>( jsonDict->at("Result") );
            if ( resultDict == nullptr ) {
                if ( err != nullptr ) {
                    error = DroiError::createDroiError( DROICODE_ERROR, "" );
                    *err = error;
                }
            }
            return resultDict;
        }
        return nullptr;
  
    }
    
    template <typename T, typename V> static bool callCloudServiceInBackground( const std::string& name, RefPtr<T> parameter, DroiCallback<V>::onCallback2 callback ) {
        static_assert(std::is_base_of<DroiObject, T>::value, "T must inherit from DroiObject");
        static_assert(std::is_base_of<DroiObject, V>::value, "V must inherit from DroiObject");

        // DroiTaskBackgroundThread
        DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher( DroiTaskBackgroundThread );
        DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();
        
        td.enqueueTask([name, parameter, callback, currentTD] {
            DroiError err;
            RefPtr<V> res = callCloudService( name, parameter, err );
            
            if ( callback != nullptr ) {
                currentTD->enqueueTask([err, callback] {
                    callback( res, err );
                });
            }
            
        });
        
        return true;
    }
};

#endif
