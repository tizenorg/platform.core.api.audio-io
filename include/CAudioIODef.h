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

#ifndef __TIZEN_MEDIA_AUDIO_IO_CAUDIO_DEF_H__
#define __TIZEN_MEDIA_AUDIO_IO_CAUDIO_DEF_H__


#ifdef __cplusplus


#include <stdio.h>
#include <dlog.h>
#include <vconf.h>

#include "CAudioError.h"
#include "CAudioInfo.h"
#include "IAudioSessionEventListener.h"
#include "CAudioSessionHandler.h"
#include "IPulseStreamListener.h"
#include "CPulseAudioVolume.h"
#include "CPulseAudioPolicy.h"
#include "CPulseStreamSpec.h"
#include "CPulseAudioClient.h"
#include "CAudioIO.h"
#include "CAudioInput.h"
#include "CAudioOutput.h"

//#define _AUDIO_IO_DEBUG_TIMING_

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TIZEN_N_AUDIO_IO"

#define AUDIO_IO_LOGD(_fmt_, arg...) { \
        LOGD(_fmt_, ##arg);      \
}

#define AUDIO_IO_LOGW(_fmt_, arg...) { \
        LOGW(_fmt_, ##arg);      \
}

#define AUDIO_IO_LOGE(_fmt_, arg...) { \
        LOGE(_fmt_, ##arg);      \
}

#define _AUDIO_IO_SHELL_COLOR_
#ifdef _AUDIO_IO_SHELL_COLOR_
#define COLOR_BLACK     "\033[0;30m"
#define COLOR_RED       "\033[0;31m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_YELLOW    "\033[0;33m"
#define COLOR_BLUE      "\033[0;34m"
#define COLOR_MAGENTA   "\033[0;35m"
#define COLOR_CYAN      "\033[0;36m"
#define COLOR_GRAY      "\033[0;37m"
#define COLOR_WHITE     "\033[1;37m"
#define COLOR_END       "\033[0m"
#else
#define COLOR_BLACK
#define COLOR_RED
#define COLOR_GREEN
#define COLOR_BLUE
#define COLOR_YELLOW
#define COLOR_MAGENTA
#define COLOR_CYAN
#define COLOR_WHITE
#define COLOR_END
#endif


#define RET_ERROR(_x_)              {return CAudioError((_x_), __FILE__, __func__, __LINE__);};
#define RET_ERROR_MSG(_x_, _msg_)   {return CAudioError((_x_), (_msg_), __FILE__, __func__, __LINE__);};

#define RET_ERROR_MSG_FORMAT(_x_, _format_, ...) {                     \
    char _msg_[CAudioError::MSG_LENGTH] = {0, };                       \
    snprintf(_msg_, CAudioError::MSG_LENGTH, _format_, ##__VA_ARGS__); \
    return CAudioError((_x_), (_msg_), __FILE__, __func__, __LINE__);  \
};


#define THROW_ERROR(_x_)            {throw  CAudioError((_x_), __FILE__, __func__, __LINE__);};
#define THROW_ERROR_MSG(_x_, _msg_) {throw  CAudioError((_x_), (_msg_), __FILE__, __func__, __LINE__);};

#define THROW_ERROR_MSG_FORMAT(_x_, _format_, ...) {                   \
    char _msg_[CAudioError::MSG_LENGTH] = {0, };                       \
    snprintf(_msg_, CAudioError::MSG_LENGTH, _format_, ##__VA_ARGS__); \
    throw CAudioError((_x_), (_msg_), __FILE__,  __func__, __LINE__);  \
};

#define VALID_POINTER_START(_x_) { \
    if ((_x_) != NULL) {

#define VALID_POINTER_END } \
}

#define SAFE_DELETE(_x_)   {if ((_x_)) {delete (_x_); (_x_) = NULL;}};
#define SAFE_FINALIZE(_x_) {if ((_x_)) {(_x_)->finalize();}};
#define SAFE_REMOVE(_x_)   {if ((_x_)) {(_x_)->finalize(); delete (_x_); (_x_) = NULL;}};


#endif
#endif /* __TIZEN_MEDIA_CPP_OBJECTS_IO_H__ */
