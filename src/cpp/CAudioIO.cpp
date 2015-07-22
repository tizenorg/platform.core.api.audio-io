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


#include <mm.h>
#include <pthread.h>
#include <assert.h>
#include <audio-session-manager.h>
#include "CAudioIODef.h"

#define AUDIO_IO_DEBUG

using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioIO
 */
CAudioIO::CAudioIO() : mIsInit(false), mForceIgnore(false), mpAudioSessionHandler(NULL), mpPulseAudioClient(NULL) {
    mState = CAudioInfo::AUDIO_IO_STATE_NONE;
    mStatePrev = CAudioInfo::AUDIO_IO_STATE_NONE;
    mByPolicy = false;
}

CAudioIO::CAudioIO(CAudioInfo& audioInfo) : mIsInit(false), mForceIgnore(false), mpAudioSessionHandler(NULL), mpPulseAudioClient(NULL) {
    mAudioInfo = audioInfo;
    mState = CAudioInfo::AUDIO_IO_STATE_NONE;
    mStatePrev = CAudioInfo::AUDIO_IO_STATE_NONE;
    mByPolicy = false;
}

CAudioIO::~CAudioIO() {
}

void CAudioIO::setInit(bool flag) {
    mIsInit = flag;
}

bool CAudioIO::isInit() {
    return mIsInit;
}

bool CAudioIO::IsReady() {
    return ((mState == CAudioInfo::AUDIO_IO_STATE_RUNNING || mState == CAudioInfo::AUDIO_IO_STATE_PAUSED)? true : false);
}

void CAudioIO::internalLock() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    if (pthread_mutex_lock(&mMutex) != 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_INTERNAL_OPERATION, "Failed pthread_mutex_lock()");
    }
#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD(COLOR_RED "LOCK" COLOR_END);
#endif
}

void CAudioIO::internalUnlock() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    if (pthread_mutex_unlock(&mMutex) != 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_INTERNAL_OPERATION, "Failed pthread_mutex_lock()");
    }
#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD(COLOR_GREEN "UNLOCK" COLOR_END);
#endif
}

void CAudioIO::internalWait() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD(COLOR_RED "WAIT" COLOR_END);
#endif

    pthread_cond_wait(&mCond, &mMutex);
}

void CAudioIO::internalSignal() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD(COLOR_GREEN "SIGNAL" COLOR_END);
#endif

    pthread_cond_signal(&mCond);
}

bool CAudioIO::isForceIgnore() {
    return mForceIgnore;
}

void CAudioIO::initialize() throw (CAudioError) {
    if (mIsInit == true) {
        return;
    }

    AUDIO_IO_LOGD("initialize");

    int ret = pthread_mutex_init(&mMutex, NULL);
    if (ret != 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pthread_mutex_init()");
    }

    ret = pthread_cond_init(&mCond, NULL);
    if (ret != 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pthread_cond_init()");
    }

    mIsInit = true;
}

void CAudioIO::finalize() {
    if (mIsInit == false) {
        return;
    }

    AUDIO_IO_LOGD("finalize");

    int ret = pthread_mutex_destroy(&mMutex);
    if (ret != 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pthread_mutex_destroy() ret:%d", ret);
    }

    ret = pthread_cond_destroy(&mCond);
    if (ret != 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pthread_cond_destroy() ret:%d", ret);
    }

    mIsInit = false;
}

void CAudioIO::onStream(CPulseAudioClient* pClient, size_t length) {
    assert(mIsInit == true);
    assert(pClient != NULL);
    assert(length > 0);

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("mStreamCallback.onStream(%p), pClient(%p), length(%zu)", mStreamCallback.onStream, pClient, length);
#endif

    if (mStreamCallback.onStream != NULL) {
        mStreamCallback.onStream(length, mStreamCallback.mUserData);
    }
}

void CAudioIO::onStateChanged(CAudioInfo::EAudioIOState state, bool byPolicy) {
    assert(mIsInit == true);
    assert(state > 0);

    mStatePrev = mState;
    mState     = state;
    mByPolicy  = byPolicy;

    AUDIO_IO_LOGD("current(%d), previous(%d), by_policy(%d)", mState, mStatePrev, mByPolicy);

    if (mStateChangedCallback.onStateChanged != NULL) {
        mStateChangedCallback.onStateChanged(mState, mStatePrev, mByPolicy, mStateChangedCallback.mUserData);
    }
}

void CAudioIO::onStateChanged(CAudioInfo::EAudioIOState state) {
    onStateChanged(state, false);
}

void CAudioIO::onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info) {
    assert(pHandler);

    int session_option = pHandler->getOptions();

    if (id == -1) {
        ///////////////////////////////////////
        // Triggered by 'focus watch callback'
        ///////////////////////////////////////

        if (session_option & (ASM_SESSION_OPTION_PAUSE_OTHERS | ASM_SESSION_OPTION_UNINTERRUPTIBLE)) {
            AUDIO_IO_LOGD("Session option is pausing others or uninterruptible, skip...");
            return;
        }

        if (state == FOCUS_IS_RELEASED) {
            // Focus handle(id) of the other application was released, do resume if possible
            internalLock();

            mpPulseAudioClient->cork(false);
            onStateChanged(CAudioInfo::AUDIO_IO_STATE_RUNNING);

            internalUnlock();

            // Focus watch callback doesn't have focus handle, but it need to convert & report to application for convenience
            state = FOCUS_IS_ACQUIRED;
        } else if (state == FOCUS_IS_ACQUIRED) {
            // Focus handle(id) of the other application was acquired, do pause if possible
            internalLock();

            if (mpPulseAudioClient->getStreamDirection() == CPulseAudioClient::STREAM_DIRECTION_PLAYBACK) {
                if (mpPulseAudioClient->drain() == false) {
                    AUDIO_IO_LOGE("Failed CPulseAudioClient::drain()");
                }
            }

            mpPulseAudioClient->cork(true);
            onStateChanged(CAudioInfo::AUDIO_IO_STATE_PAUSED);

            internalUnlock();

            // Focus watch callback doesn't have focus handle, but it need to convert & report to application for convenience
            state = FOCUS_IS_RELEASED;
        }
    } else {
        ///////////////////////////////////////
        // Triggered by 'focus callback'
        ///////////////////////////////////////

        if (pHandler->getId() != id) {
            AUDIO_IO_LOGW("Id is different, why? [mId : %d]", pHandler->getId());
        }

        if (session_option & ASM_SESSION_OPTION_UNINTERRUPTIBLE) {
            AUDIO_IO_LOGD("Session option is uninterruptible, skip...");
            return;
        }

        if (state == FOCUS_IS_RELEASED) {
            // Focus handle(id) was released, do pause here
            internalLock();

            if (mpPulseAudioClient->getStreamDirection() == CPulseAudioClient::STREAM_DIRECTION_PLAYBACK) {
                if (mpPulseAudioClient->drain() == false) {
                    AUDIO_IO_LOGE("Failed CPulseAudioClient::drain()");
                }
            }

            mpPulseAudioClient->cork(true);
            onStateChanged(CAudioInfo::AUDIO_IO_STATE_PAUSED);

            internalUnlock();
        } else if (state == FOCUS_IS_ACQUIRED) {
            // Focus handle(id) was acquired again,
            // check reason_for_change ("call-voice","call-video","voip","alarm","notification", ...)
            // do resume here and call interrupt completed callback to application.
            internalLock();

            mpPulseAudioClient->cork(false);
            onStateChanged(CAudioInfo::AUDIO_IO_STATE_RUNNING);

            internalUnlock();
        }
    }

    if (mInterruptCallback.onInterrupt != NULL) {
        IAudioSessionEventListener::EInterruptCode e = IAudioSessionEventListener::INTERRUPT_COMPLETED;
        e = IAudioSessionEventListener::convertInterruptedCode(state, reason_for_change);
        mInterruptCallback.onInterrupt(e, mInterruptCallback.mUserData);
    }
}

void CAudioIO::onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value) {
    assert(pHandler);

    if (signal == MM_SOUND_SIGNAL_RELEASE_INTERNAL_FOCUS) {
        if (value == 1 && pHandler->getSubscribeId() >= 0) {
            // Unregister focus watch callback & disable session handler
            pHandler->disableSessionHandler();
            AUDIO_IO_LOGD("Session handler disabled by signal");
        } else if (value == 0) {
            // Currently do nothing...
        }
    }
}

void CAudioIO::prepare() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    try {
        AUDIO_IO_LOGD("prepare");
        /* Do nothing */
    } catch (CAudioError e) {
        throw e;
    }
}

void CAudioIO::unprepare() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    try {
        AUDIO_IO_LOGD("unprepare");
        /* Do nothing */
    } catch (CAudioError e) {
        throw e;
    }
}

void CAudioIO::pause() throw (CAudioError) {
    if (mIsInit == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioIO");
    }

    try {
        internalLock();
        AUDIO_IO_LOGD("pause");
        mpPulseAudioClient->cork(true);
        internalUnlock();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioIO::resume() throw (CAudioError) {
    if (mIsInit == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioIO");
    }

    try {
        internalLock();
        AUDIO_IO_LOGD("resume");
        mpPulseAudioClient->cork(false);
        internalUnlock();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioIO::drain() throw (CAudioError) {
    if (mIsInit == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioIO");
    }

    try {
        internalLock();
        AUDIO_IO_LOGD("drain");
        mpPulseAudioClient->drain();
        internalUnlock();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioIO::flush() throw (CAudioError) {
    if (mIsInit == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioIO");
    }

    try {
        internalLock();
        AUDIO_IO_LOGD("flush");
        mpPulseAudioClient->flush();
        internalUnlock();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

CAudioInfo CAudioIO::getAudioInfo() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    return mAudioInfo;
}

void CAudioIO::setStreamCallback(SStreamCallback callback) throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    mStreamCallback = callback;
}

CAudioIO::SStreamCallback CAudioIO::getStreamCallback() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    return mStreamCallback;
}

void CAudioIO::setStateChangedCallback(SStateChangedCallback callback) throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    mStateChangedCallback = callback;
}

CAudioIO::SStateChangedCallback CAudioIO::getStateChangedCallback() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    return mStateChangedCallback;
}

void CAudioIO::setInterruptCallback(SInterruptCallback callback) throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    mInterruptCallback = callback;
}

CAudioIO::SInterruptCallback CAudioIO::getInterruptCallback() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    return mInterruptCallback;
}


void CAudioIO::ignoreSession() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioIO");
    }

    try {
        internalLock();

        if (mpPulseAudioClient != NULL && mpPulseAudioClient->isCorked() == false) {
            THROW_ERROR_MSG(CAudioError::ERROR_INVALID_OPERATION, "An Operation is not permitted while started");
        }

        bool isSkip = mpAudioSessionHandler->isSkipSessionEvent();
        if (isSkip == false && mpAudioSessionHandler->getId() >= 0) {
            mpAudioSessionHandler->unregisterSound();
            mForceIgnore = true;
        }

        internalUnlock();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}
