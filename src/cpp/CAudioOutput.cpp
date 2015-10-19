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


#include <vconf.h>
#include "CAudioIODef.h"

using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioOutput
 */
CAudioOutput::CAudioOutput(CAudioInfo& info) :
    CAudioIO(info),
    __mIsUsedSyncWrite(false) {
}

CAudioOutput::CAudioOutput(
        unsigned int            sampleRate,
        CAudioInfo::EChannel    channel,
        CAudioInfo::ESampleType sampleType,
        CAudioInfo::EAudioType  audioType) :
    __mIsUsedSyncWrite(false) {
    mAudioInfo = CAudioInfo(sampleRate, channel, sampleType, audioType, -1);
}

CAudioOutput::~CAudioOutput() {

}

void CAudioOutput::onStream(CPulseAudioClient* pClient, size_t length) {
    assert(pClient);

    /*
     * Does not call CAudioIO::onStream() for synchronization
     * if a user is using write()
     */
    if (__mIsUsedSyncWrite == true) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
        AUDIO_IO_LOGD("Sync Write Mode! - signal! - pClient:[%p], length:[%d]", pClient, length);
#endif
        internalSignal();
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

void CAudioOutput::onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info) {
    assert(pHandler);
    AUDIO_IO_LOGD("[pHandler:0x%x], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s]", pHandler, focus_type, state, reason_for_change, additional_info);
    CAudioIO::onInterrupt(pHandler, id, focus_type, state, reason_for_change, additional_info);
}

void CAudioOutput::onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value) {
    assert(pHandler);
    AUDIO_IO_LOGD("[pHandler:0x%x], [signal:%d], [value:%d]", pHandler, signal, value);
    CAudioIO::onSignal(pHandler, signal, value);
}

void CAudioOutput::__setInit(bool flag) {
    __mIsInit = flag;
}

bool CAudioOutput::__IsInit() {
    return (CAudioIO::isInit() == true && __mIsInit == true);
}

bool CAudioOutput::__IsReady() {
    return CAudioIO::IsReady();
}

void CAudioOutput::initialize() throw (CAudioError) {
    if (__IsInit() == true) {
        return;
    }

    try {
        CAudioIO::initialize();

        // Create ASM Handler
        mpAudioSessionHandler = new CAudioSessionHandler(CAudioSessionHandler::EAudioSessionType::AUDIO_SESSION_TYPE_PLAYBACK, mAudioInfo, this);
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

void CAudioOutput::finalize() {
    if (__IsInit() == false) {
        AUDIO_IO_LOGD("Did not initialize");
        return;
    }

    SAFE_FINALIZE(mpAudioSessionHandler);
    SAFE_DELETE(mpAudioSessionHandler);

    CAudioIO::finalize();

    __setInit(false);
}

void CAudioOutput::prepare() throw (CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioOutput");
    }

    if (__IsReady() == true) {
        AUDIO_IO_LOGD("Already prepared CAudioOutput");
        return;
    }

    try {
        internalLock();

        // Check to invalid AudioType
        CAudioInfo::EAudioType audioType = mAudioInfo.getAudioType();
        if (audioType < CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA || audioType >= CAudioInfo::EAudioType::AUDIO_TYPE_MAX) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The audioType is invalid [type:%d]", static_cast<int>(audioType));
        }

        if (mpAudioSessionHandler->getId() < 0) {  //Did not registerSound()
            if (isForceIgnore() == false) {
                // Register ASM Listener
                AUDIO_IO_LOGD("Register ASM Listener");
                mpAudioSessionHandler->registerSound();
            }
        }

        // Init StreamSpec
        AUDIO_IO_LOGD("Set Stream Spec : CPulseStreamSpec::STREAM_LATENCY_OUTPUT_MID");
        CPulseStreamSpec::EStreamLatency streamSpec = CPulseStreamSpec::EStreamLatency::STREAM_LATENCY_OUTPUT_MID;
        CPulseStreamSpec spec(streamSpec, mAudioInfo);

        // Create PulseAudio Handler
        mpPulseAudioClient = new CPulseAudioClient(CPulseAudioClient::EStreamDirection::STREAM_DIRECTION_PLAYBACK, spec, this);
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

        CAudioIO::prepare();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioOutput::unprepare() throw (CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioOutput");
    }

    if (__IsReady() == false) {
        AUDIO_IO_LOGD("Already unprepared");
        return;
    }

    try {
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

void CAudioOutput::pause() throw (CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioOutput");
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

void CAudioOutput::resume() throw (CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioOutput");
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

void CAudioOutput::drain() throw (CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioOutput");
    }

    try {
        CAudioIO::drain();
    } catch (CAudioError e) {
        throw e;
    }
}

void CAudioOutput::flush() throw (CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioOutput");
    }

    try {
        CAudioIO::flush();
    } catch (CAudioError e) {
        throw e;
    }
}

int CAudioOutput::getBufferSize() throw (CAudioError) {
    if (__IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioOutput");
    }

    if (__IsReady() == false) {
        AUDIO_IO_LOGD("Warning: Did not prepare CAudioOutput, then return zero");
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

size_t CAudioOutput::write(const void* buffer, size_t length) throw (CAudioError) {
    if (__IsInit() == false || __IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioOutput");
    }

    if (buffer == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are invalid - buffer:%p, length:%zu", buffer, length);
    }

    /* When write() is called in PulseAudio callback, bypass a pcm data to PulseAudioClient (For Asynchronous) */
    if (mpPulseAudioClient->isInThread() == true) {
        int ret = mpPulseAudioClient->write(buffer, length);
        if (ret == 0) {
            return length;
        }
    }

    try {
        /* For synchronization */
        internalLock();

        // Sets synchronous flag
        __mIsUsedSyncWrite = true;

        size_t lengthIter = length;

        while (lengthIter > 0) {
            size_t l;

            while ((l = mpPulseAudioClient->getWritableSize()) == 0) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
                AUDIO_IO_LOGD("writableSize is [%d].. wait", l);
#endif
                internalWait();
            }

            if (l > lengthIter) {
                l = lengthIter;
            }

#ifdef _AUDIO_IO_DEBUG_TIMING_
            AUDIO_IO_LOGD("PulseAudioClient->write(buffer:%p, length:%d)", buffer, l);
#endif

            int r = mpPulseAudioClient->write(buffer, l);
            if (r < 0) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "The written result is invalid ret:%d", r);
            }

            buffer = static_cast<const uint8_t*>(buffer) + l;
            lengthIter -= l;
        }  // End of while (length > 0)

        // Unsets synchronous flag
        __mIsUsedSyncWrite = false;
        internalUnlock();
    } catch (CAudioError e) {
        // Unsets synchronous flag
        __mIsUsedSyncWrite = false;
        internalUnlock();
        throw e;
    }

    return length;
}