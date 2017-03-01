/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */
#include "DroiCondition.h"
#include "DroiQuery.h"

std::string DroiCondition::LT = "$lt";
std::string DroiCondition::LT_OR_EQ = "$lte";
std::string DroiCondition::EQ = "$eq";
std::string DroiCondition::NEQ = "$ne";
std::string DroiCondition::GT_OR_EQ = "$gte";
std::string DroiCondition::GT = "$gt";
std::string DroiCondition::ISNULL = "ISNULL";
std::string DroiCondition::ISNOTNULL = "ISNOTNULL";
std::string DroiCondition::CONTAINS = "$contains";
std::string DroiCondition::NOTCONTAINS = "$notContains";
std::string DroiCondition::STARTSWITH = "$starts";
std::string DroiCondition::NOTSTARTSWITH = "$notStarts";
std::string DroiCondition::ENDSWITH = "$ends";
std::string DroiCondition::NOTENDSWITH = "$notEnds";
std::string DroiCondition::IN = "$in";
std::string DroiCondition::NOTIN = "$nin";


DroiCondition* DroiCondition::orOperator( DroiCondition* left, DroiCondition* right )
{
    return merge( left, DroiQuery::OR, right );
}

DroiCondition* DroiCondition::andOperator( DroiCondition* left, DroiCondition* right )
{
    return merge( left, DroiQuery::AND, right );
}

DroiCondition* DroiCondition::cond( const std::string& arg1, const std::string& type )
{
    return cond( arg1, type, Value("") );
}

DroiCondition* DroiCondition::cond( const std::string& arg1, const std::string& type, const cocos2d::Value& arg2 )
{
    RefVector* rvv = new RefVector();
    RefValue* arg = new RefValue( Value(arg1) );
    rvv->pushBack( arg );
    arg->release();
    
    RefValue* t = new RefValue( Value(type) );
    rvv->pushBack( t );
    t->release();
    
    if ( !arg2.isNull() ) {
        arg = new RefValue( arg2 );
        rvv->pushBack( arg );
        arg->release();
    }
    
    DroiCondition* cond = new DroiCondition();
    cond->mpConditions->pushBack( rvv );
    rvv->release();
    return cond;
}

DroiCondition* DroiCondition::cond( const std::string& arg1, const std::string& type, DroiDateTime& arg2 )
{
    RefVector* rvv = new RefVector();
    RefValue* arg = new RefValue( Value(arg1) );
    rvv->pushBack( arg );
    arg->release();
    
    RefValue* t = new RefValue( Value(type) );
    rvv->pushBack( t );
    t->release();
    
    arg = new RefValue( Value(arg2.toISO8601String()) );
    rvv->pushBack( arg );
    arg->release();

    DroiCondition* cond = new DroiCondition();
    cond->mpConditions->pushBack( rvv );
    rvv->release();
    return cond;
}

DroiCondition* DroiCondition::cond( const std::string& arg1, const std::string& type, Ref* arg2 )
{
    RefVector* rvv = new RefVector();
    
    RefValue* arg = new RefValue( Value(arg1) );
    rvv->pushBack( arg );
    arg->release();
    
    RefValue* t = new RefValue( Value(type) );
    rvv->pushBack( t );
    t->release();
    
    arg = new RefValue( Value(arg2) );
    rvv->pushBack( arg );
    arg->release();
    
    DroiCondition* cond = new DroiCondition();
    cond->mpConditions->pushBack( rvv );
    rvv->release();
    return cond;
}

DroiCondition* DroiCondition::selectIn( const std::string& arg1, RefValueVector* items )
{
    return cond( arg1, DroiCondition::IN, items );
}

DroiCondition* DroiCondition::notSelectIn( const std::string& arg1, RefValueVector* items )
{
    return cond( arg1, DroiCondition::NOTIN, items );
}

DroiCondition* DroiCondition::lt( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::LT, arg2 );
}

DroiCondition* DroiCondition::lt( const std::string& arg1, DroiDateTime& arg2 )
{
    return cond( arg1, DroiCondition::LT, arg2);
}

DroiCondition* DroiCondition::ltOrEq( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::LT_OR_EQ, arg2 );
}

DroiCondition* DroiCondition::ltOrEq( const std::string& arg1, DroiDateTime& arg2 )
{
    return cond( arg1, DroiCondition::LT_OR_EQ, arg2 );
}

DroiCondition* DroiCondition::eq( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::EQ, arg2 );
}

DroiCondition* DroiCondition::eq( const std::string& arg1, DroiDateTime& arg2 )
{
    return cond( arg1, DroiCondition::EQ, arg2 );
}

DroiCondition* DroiCondition::neq( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::NEQ, arg2 );
}

DroiCondition* DroiCondition::neq( const std::string& arg1, DroiDateTime& arg2 )
{
    return cond( arg1, DroiCondition::NEQ, arg2 );
}

DroiCondition* DroiCondition::gtOrEq( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::GT_OR_EQ, arg2 );
}

DroiCondition* DroiCondition::gtOrEq( const std::string& arg1, DroiDateTime& arg2 )
{
    return cond( arg1, DroiCondition::GT_OR_EQ, arg2 );
}

DroiCondition* DroiCondition::gt( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::GT, arg2 );
}

DroiCondition* DroiCondition::gt( const std::string& arg1, DroiDateTime& arg2 )
{
    return cond( arg1, DroiCondition::GT, arg2 );
}

DroiCondition* DroiCondition::isNull( const std::string& arg1 )
{
    return cond( arg1, DroiCondition::ISNULL );
}

DroiCondition* DroiCondition::isNotNull( const std::string& arg1 )
{
    return cond( arg1, DroiCondition::ISNOTNULL );
}

DroiCondition* DroiCondition::contains( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::CONTAINS, arg2 );
}

DroiCondition* DroiCondition::notContains( const std::string& arg1, const cocos2d::Value& arg2 )
{
    return cond( arg1, DroiCondition::NOTCONTAINS, arg2 );
}

DroiCondition* DroiCondition::startsWith( const std::string& arg1, const std::string& arg2 )
{
    return cond( arg1, DroiCondition::STARTSWITH, Value(arg2) );
}

DroiCondition* DroiCondition::notStartsWith( const std::string& arg1, const std::string& arg2 )
{
    return cond( arg1, DroiCondition::NOTSTARTSWITH, Value(arg2) );
    
}

DroiCondition* DroiCondition::endsWith( const std::string& arg1, const std::string& arg2 )
{
    return cond( arg1, DroiCondition::ENDSWITH, Value(arg2) );
}

DroiCondition* DroiCondition::notEndsWith( const std::string& arg1, const std::string& arg2 )
{
    return cond( arg1, DroiCondition::NOTENDSWITH, Value(arg2) );
}


DroiCondition::DroiCondition()
{
    mType = DroiQuery::COND;
    mpConditions = new RefVector();
    mpConditions->release();
}

DroiCondition* DroiCondition::merge( DroiCondition* left, const std::string& type, DroiCondition* right )
{
    DroiCondition* res = new DroiCondition();
    res->mType = type;
    
    if ( left->mType == type ) {
        res->mpConditions->pushBack( *left->mpConditions );
    } else {
        res->mpConditions->pushBack( left );
    }
    
    if ( right->mType == type ) {
        res->mpConditions->pushBack( *right->mpConditions );
    } else {
        res->mpConditions->pushBack( right );
    }
    
    return res;
}

DroiCondition::~DroiCondition()
{
}

std::string DroiCondition::getType()
{
    return mType;
}

RefVector* DroiCondition::getValue()
{
    return mpConditions;
}

DroiCondition* DroiCondition::autorelease()
{
    Ref::autorelease();
    return this;
}
