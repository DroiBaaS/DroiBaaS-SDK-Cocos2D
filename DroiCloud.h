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
#include "DroiHttp.h"
#include "DroiRemoteServiceHelper.h"

class DroiCloud
{
public:
    template static string callRestfulApi( const std::string& apiKey, const std::string& apiPath, cocos2d::network::HttpRequest::Type apiMethod, const std::string& parameter, DroiError* err = nullptr ) {
        RefPtr<DroiUser> currentUser = DroiUser::getCurrentUser();
        DroiError error;
        if ( currentUser->isAutoAnonymousUserEnabled() && currentUser->isLoggedIn() ) {
            currentUser = DroiUser::loginWithAnonymous(&error);
            if ( error.isOk() == false ) {
                if ( err != nullptr )
                    *err = error;
                return "";
            }
        }
        
        RefPtrAutoReleaser<RefMap> headers = new RefMap();
        headers->insert(DROI_KEY_API_KEY, new RefValue(Value(apiKey)));
        RefPtrAutoReleaser<RefValue> payload = new RefValue(Value(parameter));
        RefPtrAutoReleaser<DroiRestfulOutput> output  = DroiRestfulOutput::fromJson(DroiRemoteServiceHelper::callServer(apiPath, apiMethod, payload, headers, &error));

        if (!error.isOk()) {
            if ( err != nullptr )
                *err = error;
            return "";
        }

        DroiErrorCode code = DROICODE_OK;
        std::string appendMsg = "";
        if (output.get()->getCode()->asInt() != 0) {
            code = (DroiErrorCode) output.get()->getCode()->asInt();
        }

        std::string ticket;
        if (output->getTicket() != nullptr) {
            ticket = output->getTicket()->asString();
        }

        if (err != nullptr) {
            *err = DroiError::createDroiError(code, ticket, appendMsg);
        }

        RefMap* result = output->getResult();
        std::string data;
        if (result != nullptr) {
            cJSON* jsondata = cJSONHelper::toJSONMap(result);
            if (jsondata != nullptr) {
                char* res = cJSON_PrintUnformatted(jsondata);
                data = res;
                free(res);
                cJSON_Delete(jsondata);
            }
        }
        
        return data;
    }
    
    template static bool callCloudServiceInBackground( const std::string& apiKey, const std::string& apiPath, cocos2d::network::HttpRequest::Type apiMethod, const std::string& parameter, DroiCallback<std::string>::onCallback2 callback ) {
        // DroiTaskBackgroundThread
        DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher( DroiTaskBackgroundThread );
        DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();
        
        td.enqueueTask([apiKey, apiPath, apiMethod, parameter, callback, currentTD] {
            DroiError err;
            std::string res = callRestfulApi( apiKey, apiPath, apiMethod, parameter, &err);
            
            if ( callback != nullptr ) {
                currentTD->enqueueTask([res, err, callback] {
                    callback( res, err );
                });
            }
            
        });
        
        return true;
    }
};

#endif
