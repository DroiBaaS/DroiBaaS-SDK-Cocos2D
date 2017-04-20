/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiDateTime.h"

#ifdef WINDOWS
#define timegm _mkgmtime
#elif __ANDROID__
#include <time64.h>
#define timegm timegm64
#endif


std::chrono::milliseconds DroiDateTime::now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

DroiDateTime::DroiDateTime()
{
    mTimeValue = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

DroiDateTime::DroiDateTime( DroiDateTime& val )
{
    mTimeValue = val.mTimeValue;
}


DroiDateTime::DroiDateTime(const char* iso8601String)
{
    std::chrono::milliseconds ms;
    if ( DroiDateTime::isValidISO8601String(iso8601String, ms ) == false ) {
        // TODO: error handle
        mTimeValue = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    } else {
        mTimeValue = ms;
    }
}


std::string DroiDateTime::toISO8601String( time_t tm )
{
    struct tm * ptm;
    char buf[sizeof "2016-02-27T15:08:00Z"];

    // Get localtime
    ptm = gmtime ( &tm );  // localtime to UTC
    strftime( buf, sizeof buf, "%FT%TZ", ptm);
    // this will work too, if your compiler doesn't support %F or %T:
    //strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    return std::string(buf);
}

const std::string DroiDateTime::toISO8601String( std::chrono::milliseconds ms)
{
    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
    time_t tt = s.count();
    
    char buf[sizeof "2016-02-27T15:08:00"];
    struct tm* ptm = gmtime ( &tt );  // localtime to UTC
    strftime( buf, sizeof buf, "%FT%T", ptm);
    // this will work too, if your compiler doesn't support %F or %T:
    //strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    size_t fractional_seconds = ms.count() % 1000;
    char buf2[(sizeof buf) + 4];
    sprintf(buf2, "%s.%ldZ", buf, fractional_seconds);
    
    return std::string(buf2);
}

bool DroiDateTime::isValidISO8601String( const char* iso8601String, struct tm* ptm )
{
    int year, month, day, hour, minute, second;
    
    if ( sscanf( iso8601String, "%d-%02d-%02dT%d:%d:%dZ", &year, &month, &day, &hour, &minute, &second ) != 6 ) {
        return false;
    }

    if ( ptm != NULL ) {
        ptm->tm_year = year - 1900;
        ptm->tm_mon = month - 1;
        ptm->tm_mday = day;
        ptm->tm_hour = hour;
        ptm->tm_min = minute;
        ptm->tm_sec = second;
    }

    return true;
}

bool DroiDateTime::isValidISO8601String( const char* iso8601String, std::chrono::milliseconds& pdur )
{
    int year, month, day, hour, minute, second, milisec;
    if ( sscanf( iso8601String, "%d-%02d-%02dT%d:%d:%d.%03dZ", &year, &month, &day, &hour, &minute, &second, &milisec ) != 7 ) {
        return false;
    }

    struct tm ptm = {0};
    strptime(iso8601String, "%FT%T", &ptm);
    time_t tt = timegm(&ptm);
    std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::from_time_t(tt);
    pdur = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) + std::chrono::milliseconds(milisec);

    return true;
}

const std::string DroiDateTime::toISO8601String() const
{
    return toISO8601String(mTimeValue);
}

