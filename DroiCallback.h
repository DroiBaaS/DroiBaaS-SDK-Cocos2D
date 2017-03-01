/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef DroiCallback_hpp
#define DroiCallback_hpp
#include "base/CCRef.h"
#include <functional>

template <typename T> struct DroiCallback {
    typedef std::function<void(T)> onCallback;
    typedef std::function<void(T, DroiError)> onCallback2;
};

#endif /* DroiCallback_hpp */
