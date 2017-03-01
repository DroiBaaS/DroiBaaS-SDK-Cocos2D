/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiHelper_H__
#define __DroiHelper_H__
#include <string>

class DroiHelper
{
public:
    static std::string generateUUID();
    static std::string sha256(const std::string& data);
    static std::string url_encode(const std::string& data);
    static std::string to_lower(const std::string &data);
};

#endif /* DroiHelper_hpp */
