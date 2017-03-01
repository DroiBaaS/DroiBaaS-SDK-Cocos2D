/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiDateTime_H__
#define __DroiDateTime_H__
#include <base/CCRef.h>
#include <string>
#include <chrono>

class DroiDateTime : public cocos2d::Ref
{
public:
    static std::chrono::milliseconds now();

    /**
     *  Constructor
     */
    DroiDateTime();

    /**
     *  Constructor
     * \param val DroiDateTime to be copied
     */
    DroiDateTime( DroiDateTime& val );

    /**
     *  Constructor
     * \param iso8601String iso8601String to be copied
     */
    DroiDateTime(const char* iso8601String);

    /**
     *  Get time in ISO8601 string
     * \return string ISO8601 format
     */
    const std::string toISO8601String() const;

    /**
     *  Get time in std::chrono::milliseconds
     * \return std::chrono::milliseconds
     */
    std::chrono::milliseconds getTime() const { return mTimeValue; }

    /**
     *  Convert tm to ISO8601 string
     * \param tm time_t to be converted
     * \return string ISO8601 format
     */
    static std::string toISO8601String( time_t tm );

    /**
     *  Convert milisecond to ISO8601 string
     * \param milisecond std::chrono::milliseconds to be converted
     * \return string ISO8601 format
     */
    static const std::string toISO8601String( std::chrono::milliseconds milisecond);

    /**
     *  Check iso8601String is valid ISO8601 format or not.
     * \param iso8601String string to be checked.
     * \param ptm tm from converting iso8601String
     * \return true iso8601String is valid ISO8601 format; Otherwise is false
     */
    static bool isValidISO8601String( const char* iso8601String, struct tm* ptm );

    /**
     *  Check iso8601String is valid ISO8601 format or not.
     * \param iso8601String string to be checked.
     * \param pdur std::chrono::milliseconds from converting iso8601String
     * \return true iso8601String is valid ISO8601 format; Otherwise is false
     */
    static bool isValidISO8601String( const char* iso8601String, std::chrono::milliseconds& pdur );
    
private:
    std::chrono::milliseconds mTimeValue;
};

#endif
