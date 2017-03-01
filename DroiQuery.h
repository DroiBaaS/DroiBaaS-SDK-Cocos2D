/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiQuery_H__
#define __DroiQuery_H__
#include <string>
#include <mutex>
#include "RefValue.h"
#include "DroiError.h"
#include "DroiMultimap.h"
#include "DroiDataProvider.h"
#include "DroiCondition.h"
#include "DroiCallback.h"

/**
 *  The DroiQuery class defines a query that is used to query DroiObject.
 */
class DroiQuery
{
public:
    static DroiQuery* create();

    // Statement Definitions
    static std::string SELECT;  ///<
    static std::string INSERT;
    static std::string DELETE;
    static std::string UPDATE;
    static std::string COUNT;
    static std::string UPDATE_DATA;
    
    static std::string WHERE;
    static std::string COND;
    static std::string VALUES;
    static std::string OR;
    static std::string AND;
    
    static std::string INC;
    static std::string DEC;
    static std::string SET;
    
    static std::string ORDERBY;
    static std::string ASC;
    static std::string DESC;
    
    static std::string LIMIT;
    static std::string OFFSET;
    
    //
    /**
     * Run DroiQuery instruction
     * \return DroiError object.
     */
    DroiError run();
    
    /**
     * Run DroiQuery instruction in background thread
     * \param callback DroiQuery would execute callback method after finishing
     */
    void runInBackground( DroiCallback<DroiError>::onCallback callback );
    
    
    /**
     * Run DroiQuery to query data
     * \param error A pointer to DroiError object. You may specify nullptr for this parameter if you do not want the error information.
     * \return Query results.
     */
    RefVector* runQuery(DroiError* error);

    /**
     * Run DroiQuery to query data in background thread
     * \param callback DroiQuery would execute callback method after finishing
     */

    void runQueryInBackground( DroiCallback<RefVector*>::onCallback2 callback );
    
    
    //
    /**
     * Query data from specific table 
     * \param tableName Specify table name
     * \return DroiQuery instance
     */
    DroiQuery& queryData( const std::string& tableName );
    
    /**
     * Delete data from specific table
     * \param tableName Specify table name
     * \return DroiQuery instance
     */
    DroiQuery& deleteData( const std::string& tableName, DroiObject& obj );
    
    /**
     * Update data from specific table
     * \param tableName Specify table name
     * \return DroiQuery instance
     */
    DroiQuery& updateData( const std::string& tableName );

    DroiQuery& updateObject( const std::string& tableName, DroiObject& obj );
    
    /**
     * Count query data from specific table
     * \param tableName Specify table name
     * \return DroiQuery instance
     */
    DroiQuery& countData( const std::string& tableName );
    
    /**
     * Increase data by specific key
     * \param propertyName The specific key
     * \return DroiQuery instance
     */
    DroiQuery& inc( const std::string& propertyName );
    
    /**
     * Decrease data by specific key
     * \param propertyName The specific key
     * \return DroiQuery instance
     */
    DroiQuery& dec( const std::string& propertyName );
    
    /**
     * Set specific key data to value
     * \param propertyName The specific key
     * \param value The update data
     * \return DroiQuery instance
     */
    DroiQuery& set( const std::string& propertyName, const cocos2d::Value& value );

    /**
     * Add condition for query
     * \param propertyName The key of condition to check
     * \param opType The condition type. 
     * \param arg2 The value is for checking
     * \return DroiQuery instance
     */
    DroiQuery& where( const std::string& propertyName, const std::string& opType, const cocos2d::Value& arg2 );

    /**
     * Add condition for query
     * \param cond A DroiCondtion.
     * \return DroiQuery instance
     */
    DroiQuery& where( DroiCondition* cond );
    
    /**
     * Sort the query result by given value
     * \param propertyName The key name to sort
     * \param ascending Determine whether the ascending sort
     * \return DroiQuery instance
     */
    DroiQuery& orderBy( const std::string& propertyName, bool ascending );
    
    /**
     * Set the maximum number of query result
     * \param limitSize The maximum number of query result. Default number is 200
     * \return DroiQuery instance
     */
    DroiQuery& limit( int limitSize );
    
    /**
     * The query result starting at specific offset
     * \param position The specific offset
     * \return DroiQuery instance
     */
    DroiQuery& offset( int position );
    
private:
    static std::string getTableName(DroiMultimap<Ref*>* commands);

    DroiQuery();
    RefVector* runQueryStatement( DroiError* err );
    bool checkIfTheCommandInvalid();
    RefVector* runQueryInDataProvider( DroiDataProvider* dp, DroiError* err );
    
    // Member variables
    static std::mutex mLocker;
    DroiMultimap<Ref*> mQueryCommand;
    std::string mQueryAction;
};
#endif
