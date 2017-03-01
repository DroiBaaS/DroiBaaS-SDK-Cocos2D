/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */
#ifndef __DroiCondition_H__
#define __DroiCondition_H__
#include <string>
#include "RefValue.h"
#include "DroiDateTime.h"

class DroiCondition : public Ref
{
public:
    ~DroiCondition();
    
    static std::string LT;
    static std::string LT_OR_EQ;
    static std::string EQ;
    static std::string NEQ;
    static std::string GT_OR_EQ;
    static std::string GT;
    static std::string ISNULL;
    static std::string ISNOTNULL;
    static std::string CONTAINS;
    static std::string NOTCONTAINS;
    static std::string STARTSWITH;
    static std::string NOTSTARTSWITH;
    static std::string ENDSWITH;
    static std::string NOTENDSWITH;
    static std::string IN;
    static std::string NOTIN;
    
    static DroiCondition* orOperator( DroiCondition* left, DroiCondition* right );
    static DroiCondition* andOperator( DroiCondition* left, DroiCondition* right );

    static DroiCondition* cond( const std::string& arg1, const std::string& type );
    static DroiCondition* cond( const std::string& arg1, const std::string& type, const cocos2d::Value& arg2 );
    static DroiCondition* cond( const std::string& arg1, const std::string& type, DroiDateTime& arg2 );
    static DroiCondition* cond( const std::string& arg1, const std::string& type, Ref* arg2 );
    static DroiCondition* selectIn( const std::string& arg1, RefValueVector* items );
    static DroiCondition* notSelectIn( const std::string& arg1, RefValueVector* items );
    static DroiCondition* lt( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* lt( const std::string& arg1, DroiDateTime& arg2 );
    static DroiCondition* ltOrEq( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* ltOrEq( const std::string& arg1, DroiDateTime& arg2 );
    static DroiCondition* eq( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* eq( const std::string& arg1, DroiDateTime& arg2  );
    static DroiCondition* neq( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* neq( const std::string& arg1, DroiDateTime& arg2 );
    static DroiCondition* gtOrEq( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* gtOrEq( const std::string& arg1, DroiDateTime& arg2 );
    static DroiCondition* gt( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* gt( const std::string& arg1, DroiDateTime& arg2 );
    static DroiCondition* isNull( const std::string& arg1 );
    static DroiCondition* isNotNull( const std::string& arg1 );
    static DroiCondition* contains( const std::string& arg1, const cocos2d::Value& arg2 );
    static DroiCondition* notContains( const std::string& arg1, const cocos2d::Value& arg2 );
    
    static DroiCondition* startsWith( const std::string& arg1, const std::string& arg2 );
    static DroiCondition* notStartsWith( const std::string& arg1, const std::string& arg2 );
    static DroiCondition* endsWith( const std::string& arg1, const std::string& arg2 );
    static DroiCondition* notEndsWith( const std::string& arg1, const std::string& arg2 );
    
    std::string getType();
    RefVector* getValue();

    // For
    DroiCondition* autorelease();
private:
    DroiCondition();
    static DroiCondition* merge( DroiCondition* left, const std::string& type, DroiCondition* right );

    RefPtr<RefVector> mpConditions;
    std::string mType;
};

#endif
