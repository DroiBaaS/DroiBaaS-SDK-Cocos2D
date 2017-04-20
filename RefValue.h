/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __RefValue_H__
#define __RefValue_H__
#include <base/CCValue.h>
#include <base/CCMap.h>
#include <base/CCVector.h>
#include <base/CCRef.h>
#include <base/CCRefPtr.h>
#include <string>
#include <sstream>
#include <thread> 
#include <condition_variable>

#if __ANDROID__
namespace std {
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}
}
#endif

using namespace cocos2d;

typedef cocos2d::Vector< cocos2d::Ref* > DroiRefVector;

class RefValue : public Ref, public Value
{
public:
    RefValue( const Value& value ) : Value(value) {
    }

    RefValue() { };
};

class RefValueMap : public Ref, public ValueMap
{
public:
    RefValueMap( const ValueMap& value ) : ValueMap(value) {
    }
    RefValueMap() { };
    
};


typedef Map< std::string, cocos2d::Ref* > RefMapDataType;
class RefMap : public Ref, public RefMapDataType
{
public:
    RefMap( const RefMapDataType& value ) : RefMapDataType(value) {
    }
    RefMap() { };
    
};

class RefValueVector : public Ref, public ValueVector
{
public:
    RefValueVector( const ValueVector& value ) : ValueVector(value) {
    }
    
    RefValueVector() { };
};

typedef Vector< cocos2d::Ref* > RefVectorDataType;
class RefVector : public Ref, public RefVectorDataType
{
public:
    RefVector( const RefVectorDataType& value ) : RefVectorDataType(value) {
    }
    
    RefVector() { };

};
    
template <typename T> class RefPtrAutoReleaser
{
public:
    
    inline RefPtrAutoReleaser()
    : _ptr(nullptr)
    {
        
    }
    
    inline RefPtrAutoReleaser(T * ptr)
    : _ptr(ptr)
    {
    }
    
    inline ~RefPtrAutoReleaser()
    {
        if ( _ptr != nullptr ) {
            _ptr->release();
            _ptr = nullptr;
        }
    }
    
    inline RefPtrAutoReleaser<T> & operator = (T * other)
    {
        _ptr = other;
        return *this;
    }
    
    inline operator T * () const { return _ptr; }
    
    inline T & operator * () const
    {
        CCASSERT(_ptr, "Attempt to dereference a null pointer!");
        return *_ptr;
    }
    
    inline T * operator->() const
    {
        CCASSERT(_ptr, "Attempt to dereference a null pointer!");
        return _ptr;
    }
    
    inline T * get() const { return _ptr; }
    
    
private:
    T * _ptr;
    
    // NOTE: We can ensure T is derived from cocos2d::Ref at compile time here.
    static_assert(std::is_base_of<Ref, typename std::remove_const<T>::type>::value, "T must be derived from Ref");
};
    
template <typename T> class AutoReleaser
{
public:
    
    inline AutoReleaser()
    : _ptr(nullptr)
    {
        
    }
    
    inline AutoReleaser(T * ptr)
    : _ptr(ptr)
    {
    }
    
    inline ~AutoReleaser()
    {
        CC_SAFE_DELETE(_ptr);
    }
    
    inline operator T * () const { return _ptr; }
    
    inline T & operator * () const
    {
        CCASSERT(_ptr, "Attempt to dereference a null pointer!");
        return *_ptr;
    }
    
    inline T * operator->() const
    {
        CCASSERT(_ptr, "Attempt to dereference a null pointer!");
        return _ptr;
    }
    
    inline T * get() const { return _ptr; }
    
    
private:
    T * _ptr;
};
#endif
