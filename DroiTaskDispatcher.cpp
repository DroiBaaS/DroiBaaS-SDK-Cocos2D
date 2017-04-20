/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiTaskDispatcher.h"
#include "DroiHelper.h"
#include <functional>
#include <algorithm>
#include <base/CCDirector.h>
#include <base/CCScheduler.h>
const char* DroiMainThreadName = "DroiMainThreadName";
const char* DroiBackgroundThreadName = "DroiBackgroundThreadName";
const char* DroiTaskBackgroundThread = "DroiTaskBackgroundThread";

std::unordered_map< std::string, DroiTaskDispatcher* > DroiTaskDispatcher::taskDispatchers;
std::unordered_map< pthread_t, DroiTaskDispatcher* > DroiTaskDispatcher::mmTaskDispatcher;
pthread_key_t _gCurrentThreadTaskDispatcher;

DroiTaskDispatcher& DroiTaskDispatcher::getTaskDispatcher( const std::string& dispatcherName )
{
    DroiTaskDispatcher* taskDispatcher = taskDispatchers[ dispatcherName ];
    if ( taskDispatcher == NULL ) {
        taskDispatcher = new DroiTaskDispatcher( dispatcherName );
        taskDispatchers[ dispatcherName ] = taskDispatcher;
    }
    
    return *taskDispatcher;
}

DroiTaskDispatcher* DroiTaskDispatcher::currentTaskDispatcher()
{
    pthread_t current = pthread_self();
    DroiTaskDispatcher* currentTD = mmTaskDispatcher[ current ];
    if ( currentTD == NULL ) {
        DroiTaskDispatcher& mainTD = getTaskDispatcher( DroiMainThreadName );
        return &mainTD;
    }
    return currentTD;
}


DROITASKID DroiTaskDispatcher::enqueueTask( const DROITASK& task )
{
    return enqueueTask( task, 0 );
}

DROITASKID DroiTaskDispatcher::enqueueTask( const DROITASK& task, int delayInMillisecond )
{
    return enqueueTask( generateTaskName(), task, delayInMillisecond );
}

DROITASKID DroiTaskDispatcher::enqueueTask( const std::string& taskName, const DROITASK& task )
{
    return enqueueTask( taskName, task, 0 );
}

DROITASKID DroiTaskDispatcher::enqueueTask( const std::string& taskName, const DROITASK& task, int delayInMillisecond )
{
    return enqueueTimerTask( taskName, task, delayInMillisecond, true );
}

DROITASKID DroiTaskDispatcher::enqueueTimerTask( const DROITASK& task, int intervalInMillisecond, bool runOnce )
{
    return enqueueTimerTask( generateTaskName(), task, intervalInMillisecond, runOnce );
}

DROITASKID DroiTaskDispatcher::enqueueTimerTask( const std::string& taskName, const DROITASK& task, int intervalInMillisecond, bool runOnce )
{
    DROITASKID taskId = hash(taskName);
    
    // Check whether this task is in tasks
    if ( tasks.find( taskId ) != tasks.end() )
        return 0;
    
    // Create new task data
    DroiTaskData taskData(taskId, std::chrono::milliseconds(intervalInMillisecond), task, runOnce );
    std::unique_lock<std::mutex> lock(csLocker);
    auto iter = tasks.emplace( taskData.id, std::move(taskData) );
    queue.insert( iter.first->second );
    csCondition.notify_all();
    return taskId;
    
}

bool DroiTaskDispatcher::killTask( DROITASKID taskId )
{
    std::unique_lock<std::mutex> lock(csLocker);
    // Remove task from tasks list
    DroiTaskDataMap::iterator iter = tasks.find( taskId );
    if ( iter != tasks.end() ) {
        DroiTaskData& taskData = iter->second;
        tasks.erase( iter );
        taskData.cancelled = true;
    }
    csCondition.notify_all();
    return false;
}

DROITASKID DroiTaskDispatcher::taskIdByName( const std::string& taskName )
{
    return hash( taskName );
}


void DroiTaskDispatcher::destroy()
{
    
    std::unique_lock<std::mutex> lock(csLocker);
    exitThread = true;
    csCondition.notify_all();
    lock.unlock();
    
    taskThread.join();
}

DroiTaskDispatcher::DroiTaskDispatcher( const std::string& dispatcherName )
{
    taskDispatcherName = dispatcherName;
    exitThread = false;
    
    taskThread = std::thread( [this] {
        
        // std::mutex
        std::mutex* pFuncLocker = NULL;
        if ( taskDispatcherName == DroiMainThreadName ) {
            pFuncLocker = new std::mutex();
        }
        
        //
        pthread_t current = pthread_self();
        mmTaskDispatcher[current] = this;
        
        // Infinite loop
        for (; !exitThread; ) {
            //
            {
                std::unique_lock<std::mutex> lock(csLocker);
                // Wait until new task
                if ( tasks.empty() ) {
                    csCondition.wait( lock );
                }
                
                if ( exitThread )
                    break;
            }
            
            auto firstTask = queue.begin();
            DroiTaskData& task = *firstTask;
            if (Clock::now() >= task.nextTriggerTime)
            {
                {
                    std::unique_lock<std::mutex> lock(csLocker);
                    queue.erase(firstTask);
                }
                
                // Mark it as running to handle racing destroy
                task.running = true;
                
                // Call the handler
                if ( !task.cancelled ) {
                    if ( taskDispatcherName == DroiMainThreadName ) {
                        //
                        std::unique_lock<std::mutex> lock(*pFuncLocker);
                        std::condition_variable cond;
                        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&,this]  {
                            task.droiTask();
                            std::unique_lock<std::mutex> lock(*pFuncLocker);
                            cond.notify_all();
                          
                        });
                        cond.wait( lock );
                    } else {
                        task.droiTask();
                    }
                }
                
                if ( task.cancelled )
                {
                    std::unique_lock<std::mutex> lock(csLocker);
                    tasks.erase( task.id );
                }
                else
                {
                    task.running = false;
                    
                    // If it is periodic, schedule a new one
                    std::unique_lock<std::mutex> lock(csLocker);
                    if ( task.runonce ) {
                        tasks.erase( task.id );
                    } else {
                        task.nextTriggerTime = task.interval + Clock::now();
                        queue.insert( task );
                    }
                }
            } else {
                std::unique_lock<std::mutex> lock(csLocker);
                // Wait until the timer is ready or a timer creation notifies
                csCondition.wait_until( lock, task.nextTriggerTime );
            }
        }
        
        //
        if ( pFuncLocker != NULL )
            delete pFuncLocker;
    } );
}

// djb2 from Daniel J. Bernstein
uint64_t DroiTaskDispatcher::hash( const std::string& str )
{
    uint64_t hash = 5381;
    uint64_t c;
    for ( int i=0; i<str.length(); i++ ) {
        c = str[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    
    return hash;
}

std::string DroiTaskDispatcher::generateTaskName()
{
    return DroiHelper::generateUUID();
}

