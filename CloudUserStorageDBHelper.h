/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef CloudUserStorageDBHelper_h
#define CloudUserStorageDBHelper_h
#include <base/CCRef.h>
#include "RefValue.h"
#include "DroiDataProvider.h"
#include "DroiCondition.h"
using namespace cocos2d;

class DroiObject;
class CloudUserStorageDBHelper : public DroiDataProvider
{
public:
    static CloudUserStorageDBHelper* instance();
    bool createTable( DroiObject* object, const std::string& clazzName );
    DroiError insert( DroiMultimap<Ref*>* commands );
    RefVector* query( DroiMultimap<Ref*>* commands, DroiError* error );
    DroiError update( DroiMultimap<Ref*>* commands );
    DroiError updateData( DroiMultimap<Ref*>* commands );
    DroiError deleteOp( DroiMultimap<Ref*>* commands );

private:
    static CloudUserStorageDBHelper* mInstance;
    DroiError deleteInternal( DroiObject* obj, const std::string& tableName );
    DroiError save( DroiObject* obj, const std::string& tableName, const std::string& op );
};

#endif /* CloudUserStorageDBHelper_h */
