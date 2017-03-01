/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiTaskDispatcher_H__
#define __DroiTaskDispatcher_H__
#include <pthread.h>
#include <thread>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <condition_variable>
#include <string>
#include <set>

typedef uint64_t DROITASKID;
typedef std::function<void()> DROITASK;
extern "C" const char* DroiMainThreadName;
extern "C" const char* DroiBackgroundThreadName;

extern "C" const char* DroiTaskBackgroundThread;

class DroiTaskDispatcher
{
public:
    
public:
    /**
     * Get the specific DroiTaskDispatcher object by dispatcherName
     * \param dispatcherName The dispatcher name
     * \return DroiTaskDispatcher instance
     */
    static DroiTaskDispatcher& getTaskDispatcher( const std::string& dispatcherName );
    
    /**
     * Get DroiTaskDispatcher object of current thread
     * \return DroiTaskDispatcher instance
     */
    static DroiTaskDispatcher* currentTaskDispatcher();
    
    /**
     * Enqueue a task into DroiTaskDispatcher
     * \param task The task that will be executed
     * \return The id of enqueued task
     */
    DROITASKID enqueueTask( const DROITASK& task );

    /**
     * Enqueue a task into DroiTaskDispatcher
     * \param task The task that will be executed
     * \param delayInMillisecond Delay in millisecond
     * \return The id of enqueued task
     */
    DROITASKID enqueueTask( const DROITASK& task, int delayInMillisecond );

    /**
     * Enqueue a task into DroiTaskDispatcher
     * \param taskName The specific task name
     * \param task The task that will be executed
     * \return The id of enqueued task
     */
    DROITASKID enqueueTask( const std::string& taskName, const DROITASK& task );
    
    /**
     * Enqueue a task into DroiTaskDispatcher
     * \param taskName The specific task name
     * \param task The task that will be executed
     * \param delayInMillisecond Delay in millisecond
     * \return The id of enqueued task
     */
    DROITASKID enqueueTask( const std::string& taskName, const DROITASK& task, int delayInMillisecond );

    /**
     * Enqueue a timer task into DroiTaskDispatcher
     * \param task The task that will be executed
     * \param intervalInMillisecond The interval in millisecond
     * \param runOnce Check whether this task is running once
     * \return The id of enqueued task
     */
    DROITASKID enqueueTimerTask( const DROITASK& task, int intervalInMillisecond, bool runOnce );
    
    /**
     * Enqueue a timer task into DroiTaskDispatcher
     * \param taskName The specific task name
     * \param task The task that will be executed
     * \param intervalInMillisecond The interval in millisecond
     * \param runOnce Check whether this task is running once
     * \return The id of enqueued task
     */
    DROITASKID enqueueTimerTask( const std::string& taskName, const DROITASK& task, int intervalInMillisecond, bool runOnce );

    /**
     * Kill the specific task
     * \param taskId The specific task id
     * \return true - succeeded; Otherwise is false
     */
    bool killTask( DROITASKID taskId );
    
    /**
     * Get task id by name
     * \param taskName the name of task
     * \return The id of task by taskName
     */
    static DROITASKID taskIdByName( const std::string& taskName );
    
    /**
     * Destroy DroiTaskDispatcher object
     */
    void destroy();

private:
    typedef std::chrono::steady_clock Clock;
    typedef std::chrono::time_point<Clock> Timestamp;
    typedef std::chrono::milliseconds Interval;
    
    struct DroiTaskData
    {
        template<typename DROITASK>
        DroiTaskData(DROITASKID id, Interval tm, DROITASK&& task, bool ro) noexcept
        : id(id)
        , interval(tm)
        , droiTask  (std::forward<DROITASK>(task))
        , running(false)
        , cancelled(false)
        , runonce(ro)
        {
            nextTriggerTime = tm + Clock::now();
        }
        
        DroiTaskData(DroiTaskData&& r) noexcept
        : id(r.id)
        , nextTriggerTime(r.nextTriggerTime)
        , interval(r.interval)
        , droiTask(std::move(r.droiTask))
        , running(r.running)
        , cancelled(r.cancelled)
        , runonce(r.runonce)
        {
        }
        
        DroiTaskData& operator=(DroiTaskData&& r)
        {
            if (this != &r)
            {
                id = r.id;
                nextTriggerTime = r.nextTriggerTime;
                droiTask = std::move(r.droiTask);
                interval = r.interval;
                running = r.running;
                cancelled = r.cancelled;
                runonce = r.runonce;
            }
            return *this;
        }
        
        DROITASKID id;
        Timestamp nextTriggerTime;
        Interval interval;
        DROITASK droiTask;
        bool running;
        bool cancelled;
        bool runonce;
    };
    
    typedef std::unordered_map<DROITASKID, DroiTaskData> DroiTaskDataMap;
    DroiTaskDataMap tasks;
    
    // Comparison functor to sort the timer "queue" by Instance::next
    struct NextActiveComparator
    {
        bool operator()(const DroiTaskData &a, const DroiTaskData &b) const
        {
            return a.nextTriggerTime < b.nextTriggerTime;
        }
    };
    
    // Queue is a set of references to DroiTaskData objects, sorted by next
    typedef std::reference_wrapper<DroiTaskData> QueueValue;
    typedef std::multiset<QueueValue, NextActiveComparator> Queue;
    Queue queue;
    
    DroiTaskDispatcher( const std::string& dispatcherName );
    static uint64_t hash( const std::string& str );
    std::string generateTaskName();
    
    std::string taskDispatcherName;
    std::mutex csLocker;
    std::condition_variable csCondition;
    std::thread taskThread;
    volatile bool exitThread;
    
    static std::unordered_map< std::string, DroiTaskDispatcher* > taskDispatchers;
    static std::unordered_map< pthread_t, DroiTaskDispatcher* > mmTaskDispatcher;
};

#endif

