/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DROI_DATA_PROVIDER_H__
#define __DROI_DATA_PROVIDER_H__
#include <string>
#include <base/CCRef.h>
#include "DroiMultimap.h"
using namespace cocos2d;
class DroiError;
class DroiObject;
class RefVector;

class DroiDataProvider : public Ref
{
public:
    virtual bool createTable( DroiObject* object, const std::string& clazzName ) = 0;
    virtual DroiError insert( DroiMultimap<Ref*>* commands ) = 0;
    virtual RefVector* query( DroiMultimap<Ref*>* commands, DroiError* error ) = 0;
    virtual DroiError update( DroiMultimap<Ref*>* commands ) = 0;
    virtual DroiError updateData( DroiMultimap<Ref*>* commands ) = 0;
    virtual DroiError deleteOp( DroiMultimap<Ref*>* commands ) = 0;
};
#endif
