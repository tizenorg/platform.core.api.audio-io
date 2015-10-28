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
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CPulseAudioClient
 */
const char* CPulseAudioClient::CLIENT_NAME = "AUDIO_IO_PA_CLIENT";

CPulseAudioClient::CPulseAudioClient(
        EStreamDirection      direction,
        CPulseStreamSpec&     spec,
        IPulseStreamListener* listener) :
    __mDirection(direction),
    __mSpec(spec),
    __mpListener(listener),
    __mpMainloop(NULL),
    __mpContext(NULL),
    __mpStream(NULL),
    __mpPropList(NULL),
    __mIsInit(false),
    __mIsOperationSuccess(false),
    __mpSyncReadDataPtr(NULL),
    __mSyncReadIndex(0),
    __mSyncReadLength(0),
    __mIsUsedSyncRead(false) {
}

CPulseAudioClient::~CPulseAudioClient() {
    finalize();
}

void CPulseAudioClient::__contextStateChangeCb(pa_context* c, void* user_data) {
    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    assert(pClient);
    assert(c);

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_READY:
        AUDIO_IO_LOGD("The context is ready!");
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        pa_threaded_mainloop_signal(pClient->__mpMainloop, 0);
        break;

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    }
}

void CPulseAudioClient::__successContextCb(pa_context* c, int success, void* user_data) {
    AUDIO_IO_LOGD("pa_context[%p], success[%d], user_data[%p]", c, success, user_data);
    assert(c);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    pClient->__mIsOperationSuccess = static_cast<bool>(success);

    pa_threaded_mainloop_signal(pClient->__mpMainloop, 0);
}

void CPulseAudioClient::__streamStateChangeCb(pa_stream* s, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);

    switch (pa_stream_get_state(s)) {
    case PA_STREAM_READY:
        AUDIO_IO_LOGD("The stream is ready!");
        pClient->__mpListener->onStateChanged(CAudioInfo::EAudioIOState::AUDIO_IO_STATE_RUNNING);
    case PA_STREAM_FAILED:
    case PA_STREAM_TERMINATED:
        pa_threaded_mainloop_signal(pClient->__mpMainloop, 0);
        break;

    case PA_STREAM_UNCONNECTED:
        break;
    case PA_STREAM_CREATING:
        break;
    }
}

void CPulseAudioClient::__streamCaptureCb(pa_stream* s, size_t length, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    assert(pClient->__mpListener);
    assert(pClient->__mpMainloop);

    if (pClient->__mIsUsedSyncRead == true) {
        pa_threaded_mainloop_signal(pClient->__mpMainloop, 0);
    }

    pClient->__mpListener->onStream(pClient, length);
}

void CPulseAudioClient::__streamPlaybackCb(pa_stream* s, size_t length, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    assert(pClient->__mpListener);

    if (pClient->__mIsInit == false) {
        AUDIO_IO_LOGD("Occurred this listener when an out stream is on the way to create : Write dummy, length[%d]", length);

        char* dummy = new char[length];
        memset(dummy, 0, length);
        pa_stream_write(s, dummy, length, NULL, 0LL, PA_SEEK_RELATIVE);
        delete [] dummy;

        return;
    }

    pClient->__mpListener->onStream(pClient, length);
}

void CPulseAudioClient::__streamLatencyUpdateCb(pa_stream* s, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);

    pa_threaded_mainloop_signal(pClient->__mpMainloop, 0);
}

void CPulseAudioClient::__successStreamCb(pa_stream* s, int success, void* user_data) {
    AUDIO_IO_LOGD("pa_stream[%p], success[%d], user_data[%p]", s, success, user_data);
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    pClient->__mIsOperationSuccess = static_cast<bool>(success);

    pa_threaded_mainloop_signal(pClient->__mpMainloop, 0);
}

void CPulseAudioClient::initialize() throw (CAudioError) {
    AUDIO_IO_LOGD("");
    if (__mIsInit == true) {
        return;
    }

    int ret = 0;
    int err = 0;

    try {
        // Allocates PA proplist
        __mpPropList = pa_proplist_new();
        if (__mpPropList == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_proplist_new()");
        }

        // Adds values on proplist for delivery to PULSEAUDIO
        char *streamType = NULL;
        CAudioInfo::EAudioType audioType = __mSpec.getAudioInfo().getAudioType();
        __mSpec.getAudioInfo().convertAudioType2StreamType(audioType, &streamType);
        pa_proplist_sets(__mpPropList, PA_PROP_MEDIA_ROLE, streamType);

        int index = __mSpec.getAudioInfo().getAudioIndex();
        if (index >= 0) {
            pa_proplist_setf(__mpPropList, PA_PROP_MEDIA_PARENT_ID, "%u", (unsigned int) index);
        }

        // Adds latency on proplist for delivery to PULSEAUDIO
        AUDIO_IO_LOGD("LATENCY : %s[%d]", __mSpec.getStreamLatencyToString(), __mSpec.getStreamLatency());
        pa_proplist_setf(__mpPropList, PA_PROP_MEDIA_TIZEN_AUDIO_LATENCY, "%s", __mSpec.getStreamLatencyToString());

        // Allocates PA mainloop
        __mpMainloop = pa_threaded_mainloop_new();
        if (__mpMainloop == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_threaded_mainloop_new()");
        }

        // Allocates PA context
        __mpContext = pa_context_new(pa_threaded_mainloop_get_api(__mpMainloop), CLIENT_NAME);
        if (__mpContext == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_context_new()");
        }

        // Sets context state changed callback
        pa_context_set_state_callback(__mpContext, __contextStateChangeCb, this);

        // Connects this client with PA server
        if (pa_context_connect(__mpContext, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed pa_context_connect()");
        }

        // LOCK for synchronous connection
        pa_threaded_mainloop_lock(__mpMainloop);

        // Start mainloop
        if (pa_threaded_mainloop_start(__mpMainloop) < 0) {
            pa_threaded_mainloop_unlock(__mpMainloop);
            THROW_ERROR_MSG(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_threaded_mainloop_start()");
        }

        // Connection process is asynchronously
        // So, this function will be waited when occurred context state change event
        // If I got a signal, do next processing
        while (true) {
            pa_context_state_t state;
            state = pa_context_get_state(__mpContext);

            if (state == PA_CONTEXT_READY) {
                break;
            }

            if (!PA_CONTEXT_IS_GOOD(state)) {
                err = pa_context_errno(__mpContext);
                pa_threaded_mainloop_unlock(__mpMainloop);
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "pa_context's state is not good : err[%d]", err);
            }

            /* Wait until the context is ready */
            pa_threaded_mainloop_wait(__mpMainloop);
        }

        // Allocates PA stream
        pa_sample_spec ss   = __mSpec.getSampleSpec();
        pa_channel_map map  = __mSpec.getChannelMap();

        __mpStream = pa_stream_new_with_proplist(__mpContext, __mSpec.getStreamName(), &ss, &map, __mpPropList);
        if (__mpStream == NULL) {
            pa_threaded_mainloop_unlock(__mpMainloop);
            THROW_ERROR_MSG(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_new_with_proplist()");
        }

        // Sets stream callbacks
        pa_stream_set_state_callback(__mpStream, __streamStateChangeCb, this);
        pa_stream_set_read_callback(__mpStream, __streamCaptureCb, this);
        pa_stream_set_write_callback(__mpStream, __streamPlaybackCb, this);
        pa_stream_set_latency_update_callback(__mpStream, __streamLatencyUpdateCb, this);

        // Connect stream with PA Server

        if (__mDirection == EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
            pa_stream_flags_t flags = static_cast<pa_stream_flags_t>(
                    PA_STREAM_INTERPOLATE_TIMING |
                    PA_STREAM_ADJUST_LATENCY     |
                    PA_STREAM_AUTO_TIMING_UPDATE);

            ret = pa_stream_connect_playback(__mpStream, NULL, NULL, flags, NULL, NULL);
        } else {
            pa_stream_flags_t flags = static_cast<pa_stream_flags_t>(
                    PA_STREAM_INTERPOLATE_TIMING |
                    PA_STREAM_ADJUST_LATENCY     |
                    PA_STREAM_AUTO_TIMING_UPDATE);

            ret = pa_stream_connect_record(__mpStream, NULL, NULL, flags);
        }

        if (ret != 0) {
            err = pa_context_errno(__mpContext);
            pa_threaded_mainloop_unlock(__mpMainloop);
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_connect() : err[%d]", err);
        }

        while (true) {
            pa_stream_state_t state;
            state = pa_stream_get_state(__mpStream);

            if (state == PA_STREAM_READY) {
                AUDIO_IO_LOGD("STREAM READY");
                break;
            }

            if (!PA_STREAM_IS_GOOD(state)) {
                err = pa_context_errno(__mpContext);
                pa_threaded_mainloop_unlock(__mpMainloop);
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "pa_stream's state is not good : err[%d]", err);
            }

            /* Wait until the stream is ready */
            pa_threaded_mainloop_wait(__mpMainloop);
        }

        // End of synchronous
        pa_threaded_mainloop_unlock(__mpMainloop);

        __mIsInit = true;
    } catch (CAudioError e) {
        finalize();
        throw e;
    }
}

void CPulseAudioClient::finalize() {
    AUDIO_IO_LOGD("");
    if (__mIsInit == false) {
        return;
    }

    if (__mpMainloop != NULL) {
        pa_threaded_mainloop_stop(__mpMainloop);
    }

    if (__mpStream != NULL) {
        pa_stream_disconnect(__mpStream);
        pa_stream_unref(__mpStream);
        __mpStream = NULL;
    }

    if (__mpContext != NULL) {
        pa_context_disconnect(__mpContext);
        pa_context_unref(__mpContext);
        __mpContext = NULL;
    }

    if (__mpMainloop != NULL) {
        pa_threaded_mainloop_free(__mpMainloop);
        __mpMainloop = NULL;
    }

    if (__mpPropList != NULL) {
        pa_proplist_free(__mpPropList);
        __mpPropList = NULL;
    }

    __mIsInit = false;
}

int CPulseAudioClient::read(void* buffer, size_t length) throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (buffer == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The parameter is invalid : buffer[%p]", buffer);
    }

    if (__mDirection == EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    size_t lengthIter = length;
    int ret = 0;

    __mIsUsedSyncRead = true;

    try {
        pa_threaded_mainloop_lock(__mpMainloop);

        while (lengthIter > 0) {
            size_t l;

            while (__mpSyncReadDataPtr == NULL) {
                ret = pa_stream_peek(__mpStream, &__mpSyncReadDataPtr, &__mSyncReadLength);
                if (ret != 0) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "Failed pa_stream_peek() : ret[%d]", ret);
                }

                if (__mSyncReadLength <= 0) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
                    AUDIO_IO_LOGD("readLength(%d byte) is not valid. wait...", __mSyncReadLength);
#endif
                    pa_threaded_mainloop_wait(__mpMainloop);
                } else if (__mpSyncReadDataPtr == NULL) {
                    // Data peeked, but it doesn't have any data
                    ret = pa_stream_drop(__mpStream);
                    if (ret != 0) {
                        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "Failed pa_stream_drop() : ret[%d]", ret);
                    }
                } else {
                    __mSyncReadIndex = 0;
                }
            }

            if (__mSyncReadLength < lengthIter) {
                l = __mSyncReadLength;
            } else {
                l = lengthIter;
            }

            // Copy partial pcm data on out parameter
#ifdef _AUDIO_IO_DEBUG_TIMING_
            AUDIO_IO_LOGD("memcpy() that a peeked buffer[%], index[%d], length[%d] on out buffer", (const uint8_t*)(__mpSyncReadDataPtr) + __mSyncReadIndex, __mSyncReadIndex, l);
#endif
            memcpy(buffer, (const uint8_t*)__mpSyncReadDataPtr + __mSyncReadIndex, l);

            // Move next position
            buffer = (uint8_t*)buffer + l;
            lengthIter -= l;

            // Adjusts the rest length
            __mSyncReadIndex  += l;
            __mSyncReadLength -= l;

            if (__mSyncReadLength == 0) {
#ifdef _AUDIO_IO_DEBUG_TIMING_
                AUDIO_IO_LOGD("__mSyncReadLength is zero, do drop()");
#endif
                ret = pa_stream_drop(__mpStream);
                if (ret != 0) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INTERNAL_OPERATION, "Failed pa_stream_drop() : ret[%d]", ret);
                }

                // Reset the internal pointer
                __mpSyncReadDataPtr = NULL;
                __mSyncReadLength   = 0;
                __mSyncReadIndex    = 0;
            }
        }  // End of while (lengthIter > 0)

        pa_threaded_mainloop_unlock(__mpMainloop);
        __mIsUsedSyncRead = false;
    } catch (CAudioError e) {
        pa_threaded_mainloop_unlock(__mpMainloop);
        __mIsUsedSyncRead = false;
        throw e;
    }

    return length;
}

int CPulseAudioClient::peek(const void** buffer, size_t* length) throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (buffer == NULL || length == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The parameter is invalid : buffer[%p], length[%p]", buffer, length);
    }

    if (__mDirection == EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    int ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        ret = pa_stream_peek(__mpStream, buffer, length);
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        ret = pa_stream_peek(__mpStream, buffer, length);
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("buffer[%p], length[%d]", *buffer, *length);
#endif

    if (ret < 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_peek() : err[%d]", ret);
    }

    return ret;
}

int CPulseAudioClient::drop() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("");
#endif

    checkRunningState();

    if (__mDirection == EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    int ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        ret = pa_stream_drop(__mpStream);
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        ret = pa_stream_drop(__mpStream);
    }

    if (ret < 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_drop() : err[%d]", ret);
    }

    return ret;
}

int CPulseAudioClient::write(const void* data, size_t length) throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (data == NULL || length < 0) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The parameter is invalid");
    }

    if (__mDirection == EStreamDirection::STREAM_DIRECTION_RECORD) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    int ret = 0;

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("data[%p], length[%d]", data, length);
#endif

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        ret = pa_stream_write(__mpStream, data, length, NULL, 0LL, PA_SEEK_RELATIVE);
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        ret = pa_stream_write(__mpStream, data, length, NULL, 0LL, PA_SEEK_RELATIVE);
    }

    if (ret < 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_write() : err[%d]", ret);
    }

    return ret;
}

void CPulseAudioClient::cork(bool cork) throw (CAudioError) {
    AUDIO_IO_LOGD("cork[%d]", cork);

    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    if (isInThread() == true) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "This operation is not supported in callback");
    }

    checkRunningState();

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        pa_operation_unref(pa_stream_cork(__mpStream, static_cast<int>(cork), __successStreamCb, this));
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        pa_operation_unref(pa_stream_cork(__mpStream, static_cast<int>(cork), __successStreamCb, this));
    }

    return;
}

bool CPulseAudioClient::isCorked() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    int isCork = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        isCork = pa_stream_is_corked(__mpStream);
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        isCork = pa_stream_is_corked(__mpStream);
    }

    AUDIO_IO_LOGD("isCork[%d]", isCork);
    return static_cast<bool>(isCork);
}

bool CPulseAudioClient::drain() throw (CAudioError) {
    AUDIO_IO_LOGD("drain");

    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        pa_operation_unref(pa_stream_drain(__mpStream, __successStreamCb, this));
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        pa_operation_unref(pa_stream_drain(__mpStream, __successStreamCb, this));
    }

    return true;
}

bool CPulseAudioClient::flush() throw (CAudioError) {
    AUDIO_IO_LOGD("flush");

    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        pa_operation_unref(pa_stream_flush(__mpStream, __successStreamCb, this));
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        pa_operation_unref(pa_stream_flush(__mpStream, __successStreamCb, this));
    }

    return true;
}

size_t CPulseAudioClient::getWritableSize() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (__mDirection != EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "This client is used for Playback");
    }

    size_t ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        ret = pa_stream_writable_size(__mpStream);
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        ret = pa_stream_writable_size(__mpStream);
    }

    return ret;
}

void CPulseAudioClient::checkRunningState() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    if (__mpContext == NULL || PA_CONTEXT_IS_GOOD(pa_context_get_state(__mpContext)) == 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_INITIALIZED, "The context[%p] is not created or not good state", __mpContext);
    }
    if (__mpStream == NULL || PA_STREAM_IS_GOOD(pa_stream_get_state(__mpStream)) == 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_INITIALIZED, "The stream[%p] is not created or not good state", __mpStream);
    }
    if (pa_context_get_state(__mpContext) != PA_CONTEXT_READY || pa_stream_get_state(__mpStream)   != PA_STREAM_READY) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_INITIALIZED, "The context[%p] or stream[%p] state is not ready", __mpContext, __mpStream);
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("This client is running");
#endif
}

bool CPulseAudioClient::isInThread() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    int ret = pa_threaded_mainloop_in_thread(__mpMainloop);

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("isInThread[%d]", ret);
#endif
    return static_cast<bool>(ret);
}

size_t CPulseAudioClient::getReadableSize() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (__mDirection != EStreamDirection::STREAM_DIRECTION_RECORD) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "This client is used for Capture");
    }

    size_t ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(__mpMainloop);
        ret = pa_stream_writable_size(__mpStream);
        pa_threaded_mainloop_unlock(__mpMainloop);
    } else {
        ret = pa_stream_writable_size(__mpStream);
    }

    return ret;
}

size_t CPulseAudioClient::getBufferSize() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    size_t ret = 0;

    try {
        if (isInThread() == false) {
            pa_threaded_mainloop_lock(__mpMainloop);
        }

        const pa_buffer_attr* attr = pa_stream_get_buffer_attr(__mpStream);
        if (attr == NULL) {
            int _err = pa_context_errno(__mpContext);
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_get_buffer_attr() : err[%d]", _err);
        }

        if (__mDirection == EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
            ret = attr->tlength;
            AUDIO_IO_LOGD("PLAYBACK buffer size[%d]", ret);
        } else {
            ret = attr->fragsize;
            AUDIO_IO_LOGD("RECORD buffer size[%d]", ret);
        }
    } catch (CAudioError err) {
        if (isInThread() == false) {
            pa_threaded_mainloop_unlock(__mpMainloop);
        }
        throw err;
    }

    if (isInThread() == false) {
        pa_threaded_mainloop_unlock(__mpMainloop);
    }

    return ret;
}

pa_usec_t CPulseAudioClient::getLatency() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    pa_usec_t ret = 0;
    int negative  = 0;

    if (isInThread() == false) {
        if (pa_stream_get_latency(__mpStream, &ret, &negative) < 0) {
            int _err = pa_context_errno(__mpContext);
            if (_err != PA_ERR_NODATA) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_get_latency() : err[%d]", _err);
            }
        }
        return negative ? 0 : ret;
    }

    pa_threaded_mainloop_lock(__mpMainloop);

    try {
        while (true) {
            if (pa_stream_get_latency(__mpStream, &ret, &negative) >= 0) {
                break;
            }

            int _err = pa_context_errno(__mpContext);
            if (_err != PA_ERR_NODATA) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed pa_stream_get_latency() : err[%d]", _err);
            }

            /* Wait until latency data is available again */
            pa_threaded_mainloop_wait(__mpMainloop);
        }
    } catch (CAudioError e) {
        pa_threaded_mainloop_unlock(__mpMainloop);
        throw e;
    }

    pa_threaded_mainloop_unlock(__mpMainloop);

    return negative ? 0 : ret;
}

pa_usec_t CPulseAudioClient::getFinalLatency() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    pa_usec_t ret = 0;
    uint32_t  ver = 0;

    try {
        if (isInThread() == false) {
            pa_threaded_mainloop_lock(__mpMainloop);
        }

        ver = pa_context_get_server_protocol_version(__mpContext);
        if (ver >= 13) {
            const pa_buffer_attr* buffer_attr = pa_stream_get_buffer_attr(__mpStream);
            const pa_sample_spec* sample_spec = pa_stream_get_sample_spec(__mpStream);
            const pa_timing_info* timing_info = pa_stream_get_timing_info(__mpStream);

            if (buffer_attr == NULL || sample_spec == NULL || timing_info == NULL) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed to get buffer_attr[%p] or sample_spec[%p] or timing_info[%p] from a pa_stream",
                        buffer_attr, sample_spec, timing_info);
            }

            if (__mDirection == EStreamDirection::STREAM_DIRECTION_PLAYBACK) {
                ret = (pa_bytes_to_usec(buffer_attr->tlength, sample_spec) + timing_info->configured_sink_usec);
                AUDIO_IO_LOGD("FINAL PLAYBACK LATENCY[%d]", ret);
            } else {
                ret = (pa_bytes_to_usec(buffer_attr->fragsize, sample_spec) + timing_info->configured_source_usec);
                AUDIO_IO_LOGD("FINAL RECORD LATENCY[%d]", ret);
            }
        } else {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_SUPPORTED, "This version(ver.%d) is not supported", ver);
        }

        if (isInThread() == false) {
            pa_threaded_mainloop_unlock(__mpMainloop);
        }
    } catch (CAudioError e) {
        if (isInThread() == false) {
            pa_threaded_mainloop_unlock(__mpMainloop);
        }
        throw e;
    }

    return ret;
}

CPulseAudioClient::EStreamDirection CPulseAudioClient::getStreamDirection() {
    return __mDirection;
}

CPulseStreamSpec CPulseAudioClient::getStreamSpec() {
    return __mSpec;
}
