/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */


#ifndef __CloudStorageDBHelper_H__
#define __CloudStorageDBHelper_H__
#include <base/CCRef.h>
#include "RefValue.h"
#include "DroiDataProvider.h"
#include "DroiCondition.h"
using namespace cocos2d;

class DroiObject;
class CloudStorageDBHelper : public DroiDataProvider
{
public:
    static CloudStorageDBHelper* instance();
    virtual bool createTable( DroiObject* object, const std::string& clazzName );
    virtual DroiError insert( DroiMultimap<Ref*>* commands );
    virtual RefVector* query( DroiMultimap<Ref*>* commands, DroiError* error );
    virtual DroiError update( DroiMultimap<Ref*>* commands );
    virtual DroiError updateData( DroiMultimap<Ref*>* commands );
    virtual DroiError deleteOp( DroiMultimap<Ref*>* commands );

private:
    friend class CloudUserStorageDBHelper;
    static CloudStorageDBHelper* mInstance;
    static Ref* getFirstElement(DroiMultimap<Ref*>* commands, std::string keyName);
    static DroiCondition* getSingleCondition(Ref** input);
    static RefVector* convertArgumentsFormat(RefVector* args);
    static std::string generateWhere(DroiMultimap<Ref*>* commands);
    static std::string generateOrder(DroiMultimap<Ref*>* commands);
    static int getOffsetLimit(DroiMultimap<Ref*>* commands, std::string keyName);
    static std::string combineQuery(std::string where, std::string order, int offset, int limit);
    static RefMap* travel(DroiCondition* cond);
    static DroiObject* translateResponse(std::string res);

    DroiError deleteInternal( DroiObject* obj, const std::string& tableName );
    DroiError save( DroiObject* obj, const std::string& tableName, const std::string& op );

};
#endif
