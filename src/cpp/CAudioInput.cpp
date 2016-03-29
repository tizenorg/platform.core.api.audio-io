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


#include <pulse/pulseaudio.h>
#include "CAudioIODef.h"

#define RECORDER_PRIVILEGE "http://tizen.org/privilege/recorder"
#define CLIENT_NAME "AUDIO_IO_PA_CLIENT"

using namespace std;
using namespace tizen_media_audio;

struct PrivilegeData {
    bool isPrivilegeAllowed;
    pa_threaded_mainloop *paMainloop;
};

/**
 * class CAudioInput inherited by CAudioIO
 */
CAudioInput::CAudioInput(CAudioInfo& info) :
    CAudioIO(info),
    __mIsUsedSyncRead(true),
    __mIsInit(false) {
}

CAudioInput::CAudioInput(
        unsigned int            sampleRate,
        CAudioInfo::EChannel    channel,
        CAudioInfo::ESampleType type,
        CAudioInfo::EAudioType  audioType) :
    __mIsUsedSyncRead(true),
    __mIsInit(false) {
    mAudioInfo = CAudioInfo(sampleRate, channel, type, audioType, -1);
}

CAudioInput::~CAudioInput() {
}

void CAudioInput::onStream(CPulseAudioClient* pClient, size_t length) {
    assert(pClient);

    /*
     * Does not call CAudioIO::onStream() for synchronization
     * if a user is using read()
     */
    if (__mIsUsedSyncRead == true) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
        AUDIO_IO_LOGD("Sync Read Mode! - pClient:[%p], length:[%d]", pClient, length);
#endif
        return;
    }

    /*
     * Accrues callback function
     */
#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("pClient:[%p], length:[%d]", pClient, length);
#endif
    CAudioIO::onStream(pClient, length);
}

//LCOV_EXCL_START
void CAudioInput::onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info) {
    assert(pHandler);
    AUDIO_IO_LOGD("[pHandler:0x%x], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s]", pHandler, focus_type, state, reason_for_change, additional_info);
    CAudioIO::onInterrupt(pHandler, id, focus_type, state, reason_for_change, additional_info);
}
//LCOV_EXCL_STOP

//LCOV_EXCL_START
void CAudioInput::onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value) {
    assert(pHandler);
    AUDIO_IO_LOGD("[pHandler:0x%x], [signal:%d], [value:%d]", pHandler, signal, value);
    CAudioIO::onSignal(pHandler, signal, value);
}
//LCOV_EXCL_STOP

void CAudioInput::__setInit(bool flag) {
    __mIsInit = flag;
}

bool CAudioInput::__IsInit() {
    return (CAudioIO::isInit() == true && __mIsInit == true);
}

bool CAudioInput::__IsReady() {
    return CAudioIO::IsReady();
}

static void __contextStateChangeCb(pa_context* c, void* user_data) {
    pa_threaded_mainloop *paMainloop = static_cast<pa_threaded_mainloop*>(user_data);
    assert(paMainloop);
    assert(c);

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_READY:
        AUDIO_IO_LOGD("The context is ready");
        pa_threaded_mainloop_signal(paMainloop, 0);
        break;

    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        AUDIO_IO_LOGD("The context is lost");
        pa_threaded_mainloop_signal(paMainloop, 0);
        break;

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    }
}

static void __checkPrivilegeCb(pa_context *c, int success, void *user_data) {
    AUDIO_IO_LOGD("pa_context[%p], success[%d], user_data[%p]", c, success, user_data);
    assert(c);
    assert(user_data);

    PrivilegeData *prData = static_cast<PrivilegeData*>(user_data);
    prData->isPrivilegeAllowed = success ? true : false;

    pa_threaded_mainloop_signal(prData->paMainloop, 0);
}

static bool __IsPrivilegeAllowed() {
    pa_operation *o;
    pa_context *c;
    int err = 0;
    PrivilegeData prData;

    prData.paMainloop = pa_threaded_mainloop_new();
    if (prData.paMainloop == NULL)
        THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_threaded_mainloop_new()");
    c = pa_context_new(pa_threaded_mainloop_get_api(prData.paMainloop), CLIENT_NAME);
    if (c == NULL)
        THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_context_new()");

    pa_context_set_state_callback(c, __contextStateChangeCb, prData.paMainloop);

    if (pa_context_connect(c, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0)
        THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_context_connect()");

    pa_threaded_mainloop_lock(prData.paMainloop);

    if (pa_threaded_mainloop_start(prData.paMainloop) < 0) {
        pa_threaded_mainloop_unlock(prData.paMainloop);
        THROW_ERROR_MSG(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_threaded_mainloop_start()");
    }

    while (true) {
        pa_context_state_t state;
        state = pa_context_get_state(c);

        if (state == PA_CONTEXT_READY)
            break;

        if (!PA_CONTEXT_IS_GOOD(state)) {
            err = pa_context_errno(c);
            pa_threaded_mainloop_unlock(prData.paMainloop);
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "pa_context's state is not good : err[%d]", err);
        }

        /* Wait until the context is ready */
        pa_threaded_mainloop_wait(prData.paMainloop);
    }

    o = pa_context_check_privilege(c, RECORDER_PRIVILEGE, __checkPrivilegeCb, &prData);
    while (pa_operation_get_state(o) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(prData.paMainloop);

    pa_threaded_mainloop_unlock(prData.paMainloop);
    pa_threaded_mainloop_stop(prData.paMainloop);
    pa_context_disconnect(c);
    pa_context_unref(c);
    pa_threaded_mainloop_free(prData.paMainloop);

    return prData.isPrivilegeAllowed;
}

void CAudioInput::initialize() throw(CAudioError) {
    if (__IsInit() == true) {
        return;
    }

    try {
        CAudioIO::initialize();
        if (__IsPrivilegeAllowed() == false) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_PERMISSION_DENIED, "No privilege for record");
        }

        // Create ASM Handler
        mpAudioSessionHandler = new CAudioSessionHandler(CAudioSessionHandler::EAudioSessionType::AUDIO_SESSION_TYPE_CAPTURE, mAudioInfo, this);
        if (mpAudioSessionHandler == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed to allocate CAudioSessionHandler object");
        }

        // Initialize ASM Handler
        mpAudioSessionHandler->initialize();

        __setInit(true);
        CAudioIO::onStateChanged(CAudioInfo::EAudioIOState::AUDIO_IO_STATE_IDLE);
    } catch (CAudioError err) {
        finalize();
        throw err;
    }
}

void CAudioInput::finalize() {
    if (__IsInit() == false) {
        AUDIO_IO_LOGD("Did not initialize");
        return;
    }

    SAFE_FINALIZE(mpAudioSessionHandler);
    SAFE_DELETE(mpAudioSessionHandler);

    CAudioIO::finalize();

    __setInit(false);
}

void CAudioInput::prepare() throw(CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (__IsReady() == true) {
        AUDIO_IO_LOGD("Already prepared CAudioInput");
        return;
    }

    try {
        internalLock();

        // Check to invalid AudioType
        CAudioInfo::EAudioType audioType = mAudioInfo.getAudioType();
        if (audioType < CAudioInfo::EAudioType::AUDIO_IN_TYPE_MEDIA || audioType > CAudioInfo::EAudioType::AUDIO_IN_TYPE_LOOPBACK) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The audioType is invalid [type:%d]", static_cast<int>(audioType));
        }

        if (mpAudioSessionHandler->getId() < 0) {  //Did not registerSound()
            // Check session to skip registration
            if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {
                // Register ASM Listener
                AUDIO_IO_LOGD("Register ASM Listener");
                mpAudioSessionHandler->registerSound();
            }
        }

        // Init StreamSpec
        AUDIO_IO_LOGD("Set Stream Spec : CPulseStreamSpec::STREAM_LATENCY_INPUT_MID");
        CPulseStreamSpec::EStreamLatency streamSpec = CPulseStreamSpec::EStreamLatency::STREAM_LATENCY_INPUT_MID;
        CPulseStreamSpec spec(streamSpec, mAudioInfo);

        // Create PulseAudio Handler
        mpPulseAudioClient = new CPulseAudioClient(CPulseAudioClient::EStreamDirection::STREAM_DIRECTION_RECORD, spec, this);
        if (mpPulseAudioClient == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed to allocate CPulseAudioClient object");
        }

        // Initialize PulseAudio Handler
        mpPulseAudioClient->initialize();

        if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {
            /* Updates ASM to PLAYING */
            mpAudioSessionHandler->updatePlaying();
        }

        internalUnlock();

        // Do Prepare
        CAudioIO::prepare();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::unprepare() throw(CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (__IsReady() == false) {
        AUDIO_IO_LOGD("Already unprepared");
        return;
    }

    try {
        // Do unprepare
        CAudioIO::unprepare();

        internalLock();

        SAFE_FINALIZE(mpPulseAudioClient);
        SAFE_DELETE(mpPulseAudioClient);

        if (mpAudioSessionHandler->getId() >= 0) {
            /* Updates ASM to STOP */
            if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {
                mpAudioSessionHandler->updateStop();
            }

            bool isSkip = mpAudioSessionHandler->isSkipSessionEvent();
            if (isSkip == false) {
                mpAudioSessionHandler->unregisterSound();
            }
        }

        internalUnlock();

        CAudioIO::onStateChanged(CAudioInfo::EAudioIOState::AUDIO_IO_STATE_IDLE);
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::pause() throw(CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    try {
        CAudioIO::pause();

        internalLock();

        /* Updates ASM to STOP */
        if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {
            mpAudioSessionHandler->updateStop();
        }

        internalUnlock();

        CAudioIO::onStateChanged(CAudioInfo::EAudioIOState::AUDIO_IO_STATE_PAUSED);
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::resume() throw(CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    try {
        internalLock();

        if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {
            /* Updates ASM to PLAYING */
            mpAudioSessionHandler->updatePlaying();
        }

        internalUnlock();

        CAudioIO::resume();

        CAudioIO::onStateChanged(CAudioInfo::EAudioIOState::AUDIO_IO_STATE_RUNNING);
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::drain() throw(CAudioError) {
    THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "Did not support drain of CAudioInput");
}

void CAudioInput::flush() throw(CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    try {
        CAudioIO::flush();
    } catch (CAudioError e) {
        throw e;
    }
}

int CAudioInput::getBufferSize() throw(CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (__IsReady() == false) {
        AUDIO_IO_LOGD("Warning: Did not prepare CAudioInput, then return zero");
        return 0;
    }

    int size = 0;

    try {
        size = mpPulseAudioClient->getBufferSize();
    } catch (CAudioError err) {
        throw err;
    }

    return size;
}

void CAudioInput::setStreamCallback(SStreamCallback callback) throw(CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (callback.onStream == NULL) {
        AUDIO_IO_LOGD("__mIsUsedSyncRead = true");
        __mIsUsedSyncRead = true;
    } else {
        AUDIO_IO_LOGD("__mIsUsedSyncRead = false");
        __mIsUsedSyncRead = false;
    }

    CAudioIO::setStreamCallback(callback);
}

size_t CAudioInput::read(void* buffer, size_t length) throw(CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    if (buffer == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL buffer:%p", buffer);
    }

    /* Checks synchronous flag */
    if (__mIsUsedSyncRead == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_INVALID_OPERATION, "Invalid operation of read() if receive stream callback");
    }

    int ret = 0;

    try {
        // Block until read done
        ret = mpPulseAudioClient->read(buffer, length);
    } catch (CAudioError e) {
        throw e;
    }

    return ret;
}

int CAudioInput::peek(const void** buffer, size_t* length) throw(CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    if (buffer == NULL || length == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL buffer:%p, length:%p", buffer, length);
    }

    /* Checks synchronous flag */
    if (__mIsUsedSyncRead == true) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_INVALID_OPERATION, "Invalid operation of peek() if does not receive a stream callback");
    }

    int ret = 0;

    try {
        ret = mpPulseAudioClient->peek(buffer, length);
    } catch (CAudioError e) {
        throw e;
    }

    return ret;
}

int CAudioInput::drop() throw(CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    /* Checks synchronous flag */
    if (__mIsUsedSyncRead == true) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_INVALID_OPERATION, "Invalid operation of drop() if does not receive a stream callback");
    }

    int ret = 0;

    try {
        ret = mpPulseAudioClient->drop();
    } catch (CAudioError e) {
        throw e;
    }

    return ret;
}
