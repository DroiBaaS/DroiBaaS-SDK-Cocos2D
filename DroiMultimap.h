/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiMultimap_H__
#define __DroiMultimap_H__
#include <base/CCRef.h>
#include <base/CCVector.h>
#include <base/CCMap.h>
#include "RefValue.h"
using namespace cocos2d;

template <class V>
class DroiMapEntry : public Ref
{
public:
    static DroiMapEntry* create( const std::string& key, V value ) {
        DroiMapEntry<V>* item = new DroiMapEntry<V>();
        item->mKey = key;
        item->mValue = value;
        item->mValue->retain();
        return item;
    }
    
    ~DroiMapEntry() {
        CC_SAFE_RELEASE( mValue );
    }
    
    std::string Key() { return mKey; }
    V Value() { return mValue; }
    
protected:
    std::string mKey;
    V mValue;
};



template <class V>
class DroiMultimap : public Ref
{

public:
    typedef Map< std::string, RefVector* > MapData;

    void put( const std::string& key, V value ) {
        DroiMapEntry<V>* entry = DroiMapEntry<V>::create( key, value );
        
        typename MapData::iterator iter =  mMap.find( key );
        
        RefPtr<RefVector> refVector;
        if ( iter == mMap.end() ) {
            refVector = new RefVector();
            refVector->release();
            mMap.insert( key, refVector );
        } else {
            refVector = iter->second;
        }
        
        mSequenceList.pushBack( entry );
        refVector->pushBack( entry );
        entry->release();
    }
    
    void remove( const std::string& key ) {
        if (getValue(key) == nullptr)
            return;

        mMap.erase( key );
        RefVector::iterator iter = mSequenceList.begin();
        for (; iter != mSequenceList.end(); ) {
            DroiMapEntry<V>* entry = dynamic_cast<DroiMapEntry<V>*>(*iter);
            if ( entry->Key() == key ) {
                iter = mSequenceList.erase( iter );
            } else {
                ++iter;
            }
        }
    }
    
    void clear() {
        mSequenceList.clear();
        mMap.clear();
    }
    
    bool containsKey( const std::string& key ) {
        return (mMap.find(key)==mMap.end())?false:true;
    }
    
    RefVector* mapEntries() {
        RefVector* vec = new RefVector( mSequenceList );
        return vec;
    }
    
    RefVector* getValue( const std::string& key ) {
        typename MapData::iterator iter =  mMap.find( key );
        
        if ( iter == mMap.end() )
            return nullptr;
        
        RefVector* refVector = iter->second;
        return refVector;
    }
    
    Ref* getElement( const std::string& key, int index ) {
        RefVector* value = getValue( key );
        if ( value == nullptr || value->size() <= index )
            return nullptr;
        
        return value->at(index);
    }
    
private:
    RefVector mSequenceList;
    MapData  mMap;
};

#endif

