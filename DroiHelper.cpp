/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiHelper.h"
#include "crypto/picosha2.h"
#include <sys/time.h>
#include <cstdlib>
#include <cfloat>

std::string DroiHelper::generateUUID()
{
    struct uuid {
        uint32_t	a;
        uint16_t	b;
        uint16_t	c;
        uint8_t	d[8];
    };
    
    uint16_t seq;
    
    struct timeval tv;
    static struct timeval		last = {0, 0};
    unsigned long long clock_reg;
    static int			adjustment = 0;
    
try_again:
    gettimeofday(&tv, 0);
    
    if ((last.tv_sec == 0) && (last.tv_usec == 0)) {
        seq = (uint16_t) rand() & 0x3FFF;
        last = tv;
        last.tv_sec--;
    }
    
    if ((tv.tv_sec < last.tv_sec) ||
        ((tv.tv_sec == last.tv_sec) &&
         (tv.tv_usec < last.tv_usec))) {
            seq = (seq+1) & 0x3FFF;
            adjustment = 0;
            last = tv;
        } else if ((tv.tv_sec == last.tv_sec) &&
                   (tv.tv_usec == last.tv_usec)) {
            if (adjustment >= 0)
                goto try_again;
            adjustment++;
        } else {
            adjustment = 0;
            last = tv;
        }
    
    clock_reg = tv.tv_usec * 10 + adjustment;
    clock_reg += ((unsigned long long) tv.tv_sec) * 10000000;
    clock_reg += (((unsigned long long) 0x01B21DD2) << 32) + 0x13814000;
    uuid uu;
    
    srand( clock_reg*time(NULL) );
    for ( int i=0; i<8; i++ ) {
        uu.d[i] = rand()&0xFF;
    }
    memcpy( &uu, &clock_reg, sizeof(clock_reg) );
    
    char str[37] = {};
    sprintf(str,
            "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uu.a, uu.b, uu.c,
            uu.d[0], uu.d[1],
            uu.d[2], uu.d[3], uu.d[4], uu.d[5], uu.d[6], uu.d[7] );
    return std::string( str );
}


/* Converts an integer value to its hex character*/
char to_hex(char code)
{
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

std::string DroiHelper::sha256(const std::string& data)
{
    return picosha2::hash256_hex_string(data);
}

std::string DroiHelper::url_encode(const std::string& data)
{
    std::string escaped;
    std::string::const_iterator it = data.begin();
    for ( ; it != data.end() ; ++it) {
        std::string::value_type c = (*it);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped += c;
        } else if (c == ' ') {
            escaped += '+';
        } else {
            escaped += '%';
            escaped += to_hex(c >> 4);
            escaped += to_hex(c & 15);
        }
    }
    
    return escaped;
}

std::string DroiHelper::to_lower(const std::string &input)
{
    std::string res = input;
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}
