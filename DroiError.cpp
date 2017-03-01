/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#include "DroiError.h"

DroiErrorCode DroiError::getCode() const
{
    return mCode;
}

void DroiError::setCode(DroiErrorCode c)
{
    mCode = c;
}

std::string DroiError::getTicket()  const
{
    return mTicket;
}

bool DroiError::isOk()  const
{
    return (mCode==DROICODE_OK)?true:false;
}

std::string DroiError::getMessage() const
{
    return mMessage;
}

void DroiError::setMessage(const std::string& msg)
{
    mMessage = msg;
}

std::string DroiError::toString() {
    std::string data;
    data.append("ErrorCode: ").append(std::to_string(mCode));
    data.append(" Ticket: ").append(mTicket);
    data.append(" ").append(mMessage);

    return data;
}

DroiError DroiError::createDroiError( DroiErrorCode code, const std::string& ticket, const std::string& message )
{
    DroiError err;
    err.mCode = code;
    err.mTicket = ticket;
    err.mMessage = message;
    return err;
}

DroiError::DroiError( const DroiError& err )
{
    mCode = err.mCode;
    mTicket = err.mTicket;
    mMessage = err.mMessage;
}


DroiError::DroiError()
{
    mCode = DROICODE_OK;
    mTicket = "";
    mMessage = "";
}
