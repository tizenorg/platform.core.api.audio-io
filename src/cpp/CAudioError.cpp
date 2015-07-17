/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <dlog.h>
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioError
 */
CAudioError::EError CAudioError::mLastError = CAudioError::ERROR_NONE;
char CAudioError::mLastErrorMsg[CAudioError::MSG_LENGTH];

CAudioError::CAudioError(EError err) : mError(err) {
    mLastError = mError;
}

CAudioError::CAudioError(EError err, const char* fileName, const char* parentFunc, int lineNum) : mError(err) {
    mLastError = mError;

    const char* findFileName = strrchr(fileName, '/');
    findFileName++;
    const char* errStr = _convertErrorToString(mError);

    snprintf(mErrorMsg, CAudioError::MSG_LENGTH, "["
            COLOR_RED    "THROW" COLOR_END ":%s|"
            COLOR_YELLOW "ERR"    COLOR_END ":%s|"
            COLOR_YELLOW "FUNC"   COLOR_END ":%s(%d)]", findFileName, errStr, parentFunc, lineNum);

    snprintf(mLastErrorMsg, CAudioError::MSG_LENGTH, "LastError:%s", mErrorMsg);
}

CAudioError::CAudioError(EError err, const char* msg, const char* fileName, const char* parentFunc, int lineNum) : mError(err) {
    mLastError = mError;

    const char* findFileName = strrchr(fileName, '/');
    findFileName++;
    const char* errStr = _convertErrorToString(mError);

    snprintf(mErrorMsg, CAudioError::MSG_LENGTH, "["
            COLOR_RED    "THROW" COLOR_END ":%s|"
            COLOR_YELLOW "ERR"    COLOR_END ":%s|"
            COLOR_YELLOW "FUNC"   COLOR_END ":%s(%d)]"
            COLOR_YELLOW "MSG"    COLOR_END ":"
            COLOR_CYAN   "%s"     COLOR_END, findFileName, errStr, parentFunc, lineNum, msg);

    snprintf(mLastErrorMsg, CAudioError::MSG_LENGTH, "LastError:%s", mErrorMsg);
}

//CAudioError::CAudioError(CAudioError& err) {
//    mError = err.mError;
//    strncpy(mErrorMsg, err.mErrorMsg, MSG_LENGTH);
//}

CAudioError::~CAudioError() {
}

const char* CAudioError::_convertErrorToString(EError err) {
    switch (err) {
    default:
    case ERROR_NONE:                return COLOR_GREEN "ERROR_NONE"               COLOR_END;
    case ERROR_INVALID_ARGUMENT:    return COLOR_RED   "ERROR_INVALID_ARGUMENT"   COLOR_END;
    case ERROR_INVALID_HANDLE:      return COLOR_RED   "ERROR_INVALID_HANDLE"     COLOR_END;
    case ERROR_INVALID_SAMPLERATE:  return COLOR_RED   "ERROR_INVALID_SAMPLERATE" COLOR_END;
    case ERROR_INVALID_CHANNEL:     return COLOR_RED   "ERROR_INVALID_CHANNEL"    COLOR_END;
    case ERROR_INVALID_FORMAT:      return COLOR_RED   "ERROR_INVALID_FORMAT"     COLOR_END;
    case ERROR_INVALID_POINTER:     return COLOR_RED   "ERROR_INVALID_POINTER"    COLOR_END;
    case ERROR_INVALID_OPERATION:   return COLOR_RED   "ERROR_INVALID_OPERATION"  COLOR_END;
    case ERROR_NOT_INITIALIZED:     return COLOR_RED   "ERROR_NOT_INITIALIZED"    COLOR_END;
    case ERROR_NOT_SUPPORTED:       return COLOR_RED   "ERROR_NOT_SUPPORTED"      COLOR_END;
    case ERROR_NOT_SUPPORTED_TYPE:  return COLOR_RED   "ERROR_NOT_SUPPORTED_TYPE" COLOR_END;
    case ERROR_PERMISSION_DENIED:   return COLOR_RED   "ERROR_PERMISSION_DENIED"  COLOR_END;
    case ERROR_DEVICE_NOT_OPENED:   return COLOR_RED   "ERROR_DEVICE_NOT_OPENED"  COLOR_END;
    case ERROR_DEVICE_NOT_CLOSED:   return COLOR_RED   "ERROR_DEVICE_NOT_CLOSED"  COLOR_END;
    case ERROR_OUT_OF_MEMORY:       return COLOR_RED   "ERROR_OUT_OF_MEMORY"      COLOR_END;
    case ERROR_INTERNAL_OPERATION:  return COLOR_RED   "ERROR_INTERNAL_OPERATION" COLOR_END;
    case ERROR_FAILED_OPERATION:    return COLOR_RED   "ERROR_FAILED_OPERATION"   COLOR_END;
    case ERROR_POLICY_BLOCKED:      return COLOR_RED   "ERROR_POLICY_BLOCKED"     COLOR_END;
    case ERROR_POLICY_INTERRUPTED:  return COLOR_RED   "ERROR_POLICY_INTERRUPTED" COLOR_END;
    case ERROR_POLICY_DUPLICATED:   return COLOR_RED   "ERROR_POLICY_DUPLICATED"  COLOR_END;
    }
}

CAudioError::EError CAudioError::getLastError() {
    return mLastError;
}

const char* CAudioError::getLastErrorMsg() {
    return mLastErrorMsg;
}

CAudioError::EError CAudioError::getError() {
    return mError;
}

const char* CAudioError::getErrorMsg() {
    return mErrorMsg;
}

CAudioError& CAudioError::operator = (const EError err) {
    mError = err;
    mLastError = mError;
    return *this;
}

CAudioError& CAudioError::operator = (const CAudioError& err) {
    mError = err.mError;
    mLastError = mError;
    memcpy(mErrorMsg, err.mErrorMsg, MSG_LENGTH);
    memcpy(mLastErrorMsg, mErrorMsg, MSG_LENGTH);
    return *this;
}

bool CAudioError::operator != (const EError err) {
    return (mError != err);
}

bool CAudioError::operator == (const EError err) {
    return (mError == err);
}

//bool operator == (const CAudioError& src, const CAudioError::EError& err) {
//    //return (src.mLastError == err);
//    return true;
//}
