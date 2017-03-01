/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiQuery.h"
#include "DroiTaskDispatcher.h"
#include "CloudStorageDBHelper.h"
#include "CloudUserStorageDBHelper.h"
#include "DroiHelper.h"
#include "DroiCondition.h"
#include <condition_variable>

std::string DroiQuery::SELECT = "select";
std::string DroiQuery::INSERT = "insert";
std::string DroiQuery::DELETE = "delete";
std::string DroiQuery::UPDATE = "update";
std::string DroiQuery::COUNT = "count";
std::string DroiQuery::UPDATE_DATA = "updateData";
            
std::string DroiQuery::WHERE = "where";
std::string DroiQuery::COND = "cond";
std::string DroiQuery::VALUES = "values";
std::string DroiQuery::OR = "or";
std::string DroiQuery::AND = "and";
            
std::string DroiQuery::INC = "inc";
std::string DroiQuery::DEC = "dec";
std::string DroiQuery::SET = "set";
            
std::string DroiQuery::ORDERBY = "orderby";
std::string DroiQuery::ASC = "ASC";
std::string DroiQuery::DESC = "DESC";
            
std::string DroiQuery::LIMIT = "limit";
std::string DroiQuery::OFFSET = "offset";

std::string OPQueueThread = "DroiQueryOPQueue";

std::mutex DroiQuery::mLocker;

DroiQuery* DroiQuery::create()
{
    DroiQuery* query = new DroiQuery;
    return query;
}

DroiQuery::DroiQuery()
{
}

DroiError DroiQuery::run()
{
    DroiError err;
    runQuery(&err);
    return err;
}

void DroiQuery::runInBackground( DroiCallback<DroiError>::onCallback callback )
{
    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher( DroiTaskBackgroundThread );
    DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();

    td.enqueueTask([this, callback, currentTD] {
        DroiError err = run();
        
        if ( callback != nullptr ) {
            currentTD->enqueueTask([err, callback] {
                callback( err );
            });
        }
        
    });
}

RefVector* DroiQuery::runQuery(DroiError* error)
{
    return runQueryStatement( error );
}

void DroiQuery::runQueryInBackground( DroiCallback<RefVector*>::onCallback2 callback )
{
    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher( DroiTaskBackgroundThread );
    DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();

    td.enqueueTask([this, callback, currentTD] {
        DroiError err;
        
        RefVector* vec = runQuery( &err );
        
        if ( callback != nullptr ) {
            currentTD->enqueueTask([err, callback, vec] {
                callback( vec, err );
            });
        }
    });
}

//
DroiQuery& DroiQuery::queryData( const std::string& tableName )
{
    RefValue* tbName = new RefValue( Value(tableName) );
    mQueryCommand.put( SELECT, tbName );
    tbName->release();
    return *this;
}

DroiQuery& DroiQuery::deleteData( const std::string& tableName, DroiObject& obj )
{
    RefPtr<RefVector> rv = new RefVector();
    rv->release();
    rv->pushBack( (Ref*) &obj );
    RefValue* tbName = new RefValue( Value(tableName) );
    rv->pushBack( tbName );
    tbName->release();
    mQueryCommand.put( DELETE, rv.get());
    return *this;
}

DroiQuery& DroiQuery::updateData( const std::string& tableName )
{
    RefValue* tbName = new RefValue( Value(tableName) );
    mQueryCommand.put( UPDATE_DATA, tbName );
    tbName->release();
    return *this;
}

DroiQuery& DroiQuery::updateObject( const std::string& tableName, DroiObject& obj )
{
    RefPtr<RefVector> rv = new RefVector();
    rv->release();
    
    rv->pushBack( (Ref*) &obj );
    RefValue* tbName = new RefValue( Value(tableName) );
    rv->pushBack( tbName );
    tbName->release();
    mQueryCommand.put( UPDATE, rv);
    return *this;
}

DroiQuery& DroiQuery::countData( const std::string& tableName )
{
    RefPtrAutoReleaser<RefValue> tbName = new RefValue( Value(tableName) );
    mQueryCommand.put( COUNT, tbName.get() );
    return *this;
}

DroiQuery& DroiQuery::inc( const std::string& propertyName )
{
    RefPtrAutoReleaser<RefValue> value = new RefValue( Value(propertyName) );
    mQueryCommand.put( INC, value.get() );
    return *this;
}

DroiQuery& DroiQuery::dec( const std::string& propertyName )
{
    RefPtrAutoReleaser<RefValue> value = new RefValue( Value(propertyName) );
    mQueryCommand.put( COUNT, value.get() );
    return *this;
}

DroiQuery& DroiQuery::set( const std::string& propertyName, const cocos2d::Value& value )
{
    RefPtrAutoReleaser<RefValueVector> vec = new RefValueVector();
    vec->push_back( Value(propertyName) );
    vec->push_back( value );
    mQueryCommand.put( SET, vec.get() );
    return *this;
}

DroiQuery& DroiQuery::where( const std::string& propertyName, const std::string& opType, const cocos2d::Value& arg2 )
{
    if ( mQueryCommand.containsKey(WHERE) ) {
        log("There is one WHERE statement in commnad queue. Skip this WHERE statement");
        return *this;
    }

    DroiCondition* cond = DroiCondition::cond(propertyName, opType, arg2);
    mQueryCommand.put( DroiQuery::WHERE, cond );
    


    return *this;
}

DroiQuery& DroiQuery::where( DroiCondition* cond )
{
    if ( mQueryCommand.containsKey(WHERE) ) {
        log("There is one WHERE statement in commnad queue. Skip this WHERE statement");
        return *this;
    }

    mQueryCommand.put( DroiQuery::WHERE, cond );

    return *this;
}

DroiQuery& DroiQuery::orderBy( const std::string& propertyName, bool ascending )
{
    RefPtrAutoReleaser<RefValueVector> values = new RefValueVector();
    values->push_back( Value(propertyName) );
    values->push_back( ascending?Value(ASC):Value(DESC) );
    mQueryCommand.put( DroiQuery::ORDERBY, values );
    return *this;
}

DroiQuery& DroiQuery::limit( int limitSize )
{
    if ( mQueryCommand.containsKey( DroiQuery::LIMIT ) ) {
        log( "There is LIMIT object in command queue." );
    } else {
        limitSize = MIN( limitSize, 1000 );
        RefPtrAutoReleaser<RefValue> val = new RefValue( Value(limitSize) );
        mQueryCommand.put( DroiQuery::LIMIT, val.get() );
    }
    return *this;
}

DroiQuery& DroiQuery::offset( int position )
{
    if ( mQueryCommand.containsKey( DroiQuery::OFFSET ) ) {
        log( "There is OFFSET object in command queue." );
    } else {
        position = MIN( position, 1000 );
        RefPtrAutoReleaser<RefValue> val = new RefValue( Value(position) );
        mQueryCommand.put( DroiQuery::OFFSET, val.get() );
    }
    return *this;
}


bool DroiQuery::checkIfTheCommandInvalid()
{
    int fs_counter = (mQueryCommand.containsKey( DroiQuery::SELECT )?1:0) +
        (mQueryCommand.containsKey( DroiQuery::INSERT )?1:0) +
        (mQueryCommand.containsKey( DroiQuery::DELETE )?1:0) +
        (mQueryCommand.containsKey( DroiQuery::UPDATE )?1:0) +
        (mQueryCommand.containsKey( DroiQuery::UPDATE_DATA )?1:0);
    
    if ( fs_counter > 1 )
        return false;
    
    if ( fs_counter == 0 ) {
        RefPtrAutoReleaser<RefValue> val = new RefValue( Value("*") );
        mQueryCommand.put( DroiQuery::SELECT, val.get() );
    }

    static std::string queryActionList[] = { SELECT, INSERT, DELETE, UPDATE, UPDATE_DATA };
    
    for ( std::string action : queryActionList ) {
        if ( mQueryCommand.containsKey(action) ) {
            mQueryAction = action;
            break;
        }
    }

    if ( mQueryCommand.containsKey(INSERT) && ( mQueryCommand.containsKey(OR) || mQueryCommand.containsKey(AND) || mQueryCommand.containsKey(WHERE) ||
                                               mQueryCommand.containsKey(INC) || mQueryCommand.containsKey(DEC) || mQueryCommand.containsKey(SET) ) ) {
        log( "IllegalArgument. Insert command cannot combine with OR/AND/WHERE command" );
        return false;
    }

    if ( (mQueryCommand.containsKey(OR) || mQueryCommand.containsKey(AND)) && !mQueryCommand.containsKey(WHERE) ) {
        log( "IllegalArgument. OR/AND statement is only for WHERE statement" );
        return false;
    }
    
    int update_counter = (mQueryCommand.containsKey(INC)?1:0) +
        (mQueryCommand.containsKey(DEC)?1:0) +
        (mQueryCommand.containsKey(SET)?1:0);
    if ( mQueryCommand.containsKey(UPDATE_DATA) && update_counter != 1 ) {
        return false;
    }
    
    return true;
}

RefVector* DroiQuery::runQueryStatement( DroiError* err )
{
    if ( checkIfTheCommandInvalid() == false )
        return nullptr;

    DroiTaskDispatcher& td = DroiTaskDispatcher::getTaskDispatcher(OPQueueThread);
    DroiTaskDispatcher* currentTD = DroiTaskDispatcher::currentTaskDispatcher();
    RefVector* res = nullptr;
    DroiDataProvider* dp = nullptr;

    std::string table = getTableName(&mQueryCommand);
    if (table.compare("_User") == 0 || table.compare("_Group") == 0)
        dp = CloudUserStorageDBHelper::instance();
    else
        dp = CloudStorageDBHelper::instance();
    
    if ( currentTD != nullptr && &td == currentTD ) {
        // Same thread
        res = runQueryInDataProvider( dp, err );
    } else {
        std::unique_lock<std::mutex> lock(mLocker);
        std::condition_variable csCondition;
        td.enqueueTask([this, &err, &res, &csCondition, dp] {
            res = runQueryInDataProvider( dp, err );
            
            std::unique_lock<std::mutex> lock(mLocker);
            csCondition.notify_all();
        });

        csCondition.wait( lock );
    }
    
    return res;
}

RefVector* DroiQuery::runQueryInDataProvider( DroiDataProvider* dp, DroiError* err )
{
    RefVector* res = nullptr;
    
    DroiError error;
    
    if ( mQueryAction == UPDATE_DATA )
        error = dp->updateData(&mQueryCommand);
    else {
        // Remove unused command
        mQueryCommand.remove( INC );
        mQueryCommand.remove( DEC );
        mQueryCommand.remove( SET );
        mQueryCommand.remove( UPDATE_DATA );
        
        if ( mQueryAction == SELECT )
            res = dp->query( &mQueryCommand, &error );
        else if ( mQueryAction == INSERT )
            error = dp->insert( &mQueryCommand );
        else if ( mQueryAction == DELETE )
            error = dp->deleteOp(&mQueryCommand);
        else if ( mQueryAction == UPDATE )
            error = dp->update(&mQueryCommand);
    }
    
    if ( err != nullptr ) *err = error;
    return res;
}


std::string DroiQuery::getTableName(DroiMultimap<Ref*>* commands)
{
    std::string tableName("");
    if (commands->containsKey(DroiQuery::SELECT)) {
        RefPtr<RefVector> args = commands->getValue( DroiQuery::SELECT );
        if ( args->size() == 1 ) {
            DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
            RefValue* value = (RefValue*)entry->Value();
            tableName = value->asString();
        }
    } else if (commands->containsKey(DroiQuery::UPDATE)) {
        RefPtr<RefVector> args = commands->getValue( DroiQuery::UPDATE );
        DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
        RefVector* value = (RefVector* )entry->Value();
        RefValue* tRef = dynamic_cast<RefValue *>(value->at(1));
        tableName = tRef->asString();

    } else if (commands->containsKey(DroiQuery::DELETE)) {
        RefPtr<RefVector> args = commands->getValue( DroiQuery::DELETE );
        DroiMapEntry<Ref*>* entry = (DroiMapEntry<Ref*>*)args->at(0);
        RefVector* value = (RefVector* )entry->Value();
        RefValue* tRef = dynamic_cast<RefValue *>(value->at(1));
        tableName = tRef->asString();
    }

    return tableName;
}
