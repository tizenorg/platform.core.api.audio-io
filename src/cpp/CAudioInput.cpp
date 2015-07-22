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


#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioInput inherited by CAudioIO
 */
CAudioInput::CAudioInput(CAudioInfo& info)
    : CAudioIO(info),
      mpSyncReadDataPtr(NULL),
      mSyncReadIndex(0),
      mSyncReadLength(0),
      mIsUsedSyncRead(true) {
}

CAudioInput::CAudioInput(
        unsigned int            sampleRate,
        CAudioInfo::EChannel    channel,
        CAudioInfo::ESampleType type,
        CAudioInfo::EAudioType  audioType)
      : mpSyncReadDataPtr(NULL),
        mSyncReadIndex(0),
        mSyncReadLength(0),
        mIsUsedSyncRead(true) {
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
    if (mIsUsedSyncRead == true) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
        AUDIO_IO_LOGD("Sync Read Mode! - signal! - pClient:[%p], length:[%d]", pClient, length);
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

void CAudioInput::onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info) {
    assert(pHandler);
    AUDIO_IO_LOGD("[pHandler:0x%x], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s]", pHandler, focus_type, state, reason_for_change, additional_info);
    CAudioIO::onInterrupt(pHandler, id, focus_type, state, reason_for_change, additional_info);
}

void CAudioInput::onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value) {
    assert(pHandler);
    AUDIO_IO_LOGD("[pHandler:0x%x], [signal:%d], [value:%d]", pHandler, signal, value);
    CAudioIO::onSignal(pHandler, signal, value);
}

void CAudioInput::setInit(bool flag) {
    mIsInit = flag;
}

bool CAudioInput::IsInit() {
    return (CAudioIO::isInit() == true && mIsInit == true);
}

bool CAudioInput::IsReady() {
    return CAudioIO::IsReady();
}

void CAudioInput::initialize() throw (CAudioError) {
    if (IsInit() == true) {
        return;
    }

    try {
        CAudioIO::initialize();

        // Create ASM Handler
        mpAudioSessionHandler = new CAudioSessionHandler(CAudioSessionHandler::AUDIO_SESSION_TYPE_CAPTURE, mAudioInfo, this);
        if (mpAudioSessionHandler == NULL) {
            THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed to allocate CAudioSessionHandler object");
        }

        // Initialize ASM Handler
        mpAudioSessionHandler->initialize();

        setInit(true);
        CAudioIO::onStateChanged(CAudioInfo::AUDIO_IO_STATE_IDLE);
    } catch (CAudioError err) {
        finalize();
        throw err;
    }
}

void CAudioInput::finalize() {
    if (IsInit() == false) {
        AUDIO_IO_LOGD("Did not initialize");
        return;
    }

    SAFE_FINALIZE(mpAudioSessionHandler);
    SAFE_DELETE(mpAudioSessionHandler);

    CAudioIO::finalize();

    setInit(false);
}

void CAudioInput::prepare() throw (CAudioError) {
    if (IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (IsReady() == true) {
        AUDIO_IO_LOGD("Already prepared CAudioInput");
        return;
    }

    try {
        internalLock();

        // Check to invalid AudioType
        CAudioInfo::EAudioType audioType = mAudioInfo.getAudioType();
        if (audioType < CAudioInfo::AUDIO_IN_TYPE_MEDIA || audioType > CAudioInfo::AUDIO_IN_TYPE_LOOPBACK) {
            THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "The audioType is invalid [type:%d]", static_cast<int>(audioType));
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
        AUDIO_IO_LOGD("Set Strem Spec : CPulseStreamSpec::STREAM_LATENCY_INPUT_MID");
        CPulseStreamSpec::EStreamLatency streamSpec = CPulseStreamSpec::STREAM_LATENCY_INPUT_MID;
        CPulseStreamSpec spec(streamSpec, mAudioInfo);

        // Create PulseAudio Handler
        mpPulseAudioClient = new CPulseAudioClient(CPulseAudioClient::STREAM_DIRECTION_RECORD, spec, this);
        if (mpPulseAudioClient == NULL) {
            THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed to allocate CPulseAudioClient object");
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

void CAudioInput::unprepare() throw (CAudioError) {
    if (IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (IsReady() == false) {
        AUDIO_IO_LOGD("Already unprepared");
        return;
    }

    try{
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

        CAudioIO::onStateChanged(CAudioInfo::AUDIO_IO_STATE_IDLE);
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::pause() throw (CAudioError) {
    if (IsInit() == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    try{
        CAudioIO::pause();

        internalLock();

        /* Updates ASM to STOP */
        if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {
            mpAudioSessionHandler->updateStop();
        }

        internalUnlock();

        CAudioIO::onStateChanged(CAudioInfo::AUDIO_IO_STATE_PAUSED);
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::resume() throw (CAudioError) {
    if (IsInit() == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    try {
        internalLock();

        if (isForceIgnore() == false && mpAudioSessionHandler->isSkipSessionEvent() == false) {

            /* Updates ASM to PLAYING */
            mpAudioSessionHandler->updatePlaying();
        }

        internalUnlock();

        CAudioIO::resume();

        CAudioIO::onStateChanged(CAudioInfo::AUDIO_IO_STATE_RUNNING);
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }
}

void CAudioInput::drain() throw (CAudioError) {
    THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "Did not support drain of CAudioInput");
}

void CAudioInput::flush() throw (CAudioError) {
    if (IsInit() == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    try {
        CAudioIO::flush();
    } catch (CAudioError e) {
        throw e;
    }
}

int CAudioInput::getBufferSize() throw (CAudioError) {
    if (IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (IsReady() == false) {
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

void CAudioInput::setStreamCallback(SStreamCallback callback) throw (CAudioError) {
    if (IsInit() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CAudioInput");
    }

    if (callback.onStream == NULL) {
        AUDIO_IO_LOGD("mIsUsedSyncRead = true");
        mIsUsedSyncRead = true;
    } else {
        AUDIO_IO_LOGD("mIsUsedSyncRead = false");
        mIsUsedSyncRead = false;
    }

    CAudioIO::setStreamCallback(callback);
}

size_t CAudioInput::read(void* buffer, size_t length) throw (CAudioError) {
    if (IsInit() == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    if (buffer == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "Parameters are invalid - buffer:%p, length:%zu", buffer, length);
    }

    /* Checks synchronous flag */
    if (mIsUsedSyncRead == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_INVALID_OPERATION, "Invalid operation of read() if receive stream callback");
    }

    size_t lengthIter = length;
    int ret = 0;

    try {
        internalLock();

        while (lengthIter > 0) {
            size_t l;

            while (mpSyncReadDataPtr == NULL) {
                ret = mpPulseAudioClient->peek(&mpSyncReadDataPtr, &mSyncReadLength);
                if (ret != 0) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "Failed CPulseAudioClient::peek() ret:[%d]", ret);
                }

                if (mSyncReadLength <= 0) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
                    AUDIO_IO_LOGD("readLength(%d byte) is not valid.. wait..", mSyncReadLength);
#endif
                    internalWait();
                } else if (mpSyncReadDataPtr == NULL) {
                    /* There's a hole in the stream, skip it. We could generate
                     * silence, but that wouldn't work for compressed streams.
                     */
                    ret = mpPulseAudioClient->drop();
                    if (ret != 0) {
                        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "Failed CPulseAudioClient::drop() ret:[%d]", ret);
                    }
                } else {
                    mSyncReadIndex = 0;
                }
            }//end of while (pReadData == NULL)

            if (mSyncReadLength < lengthIter) {
                l = mSyncReadLength;
            } else {
                l = lengthIter;
            }

            // Copy partial pcm data on out parameter
#ifdef _AUDIO_IO_DEBUG_TIMING_
            AUDIO_IO_LOGD("memcpy() that a peeked buffer(%p), index(%d), length(%d), on out buffer", (const uint8_t*)(mpSyncReadDataPtr) + mSyncReadIndex, mSyncReadIndex, l);
#endif
            memcpy(buffer, (const uint8_t*)mpSyncReadDataPtr + mSyncReadIndex, l);

            // Move next position
            buffer = (uint8_t*)buffer + l;
            lengthIter -= l;

            // Adjusts the rest length
            mSyncReadIndex  += l;
            mSyncReadLength -= l;

            if (mSyncReadLength == 0) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
                AUDIO_IO_LOGD("mSyncReadLength is zero - Do drop()");
#endif
                ret = mpPulseAudioClient->drop();
                if (ret != 0) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "Failed CPulseAudioClient::drop() ret:[%d]", ret);
                }

                // Reset the internal pointer
                mpSyncReadDataPtr = NULL;
                mSyncReadLength   = 0;
                mSyncReadIndex    = 0;
            }
        }  // End of while (length > 0)

        internalUnlock();
    } catch (CAudioError e) {
        internalUnlock();
        throw e;
    }

    return length;
}

int CAudioInput::peek(const void** buffer, size_t* length) throw (CAudioError) {
    if (IsInit() == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    if (buffer == NULL || length == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "Parameters are NULL buffer:%p, length:%p", buffer, length);
    }

    /* Checks synchronous flag */
    if (mIsUsedSyncRead == true) {
        THROW_ERROR_MSG(CAudioError::ERROR_INVALID_OPERATION, "Invalid operation of peek() if does not receive a stream callback");
    }

    int ret = 0;

    try {
        ret = mpPulseAudioClient->peek(buffer, length);
    } catch (CAudioError e) {
        throw e;
    }

    return ret;
}

int CAudioInput::drop() throw (CAudioError) {
    if (IsInit() == false || IsReady() == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize or prepare CAudioInput");
    }

    /* Checks synchronous flag */
    if (mIsUsedSyncRead == true) {
        THROW_ERROR_MSG(CAudioError::ERROR_INVALID_OPERATION, "Invalid operation of drop() if does not receive a stream callback");
    }

    int ret = 0;

    try {
        ret = mpPulseAudioClient->drop();
    } catch (CAudioError e) {
        throw e;
    }

    return ret;
}
