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

CPulseAudioClient::CPulseAudioClient(EStreamDirection      direction,
                                     CPulseStreamSpec&     spec,
                                     IPulseStreamListener* listener) :
    mDirection(direction),
    mSpec(spec),
    mpListener(listener),
    mpMainloop(NULL),
    mpContext(NULL),
    mpStream(NULL),
    mpPropList(NULL),
    mIsInit(false),
    mIsOperationSuccess(false) {
}

CPulseAudioClient::~CPulseAudioClient() {
    finalize();
}

void CPulseAudioClient::_contextStateChangeCb(pa_context* c, void* user_data) {
    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    assert(pClient);
    assert(c);

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_READY:
        AUDIO_IO_LOGD("The context is ready!");
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        pa_threaded_mainloop_signal(pClient->mpMainloop, 0);
        break;

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    }
}

void CPulseAudioClient::_successContextCb(pa_context* c, int success, void* user_data) {
    AUDIO_IO_LOGD("pa_context[%p], success[%d], user_data[%p]", c, success, user_data);
    assert(c);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    pClient->mIsOperationSuccess = static_cast<bool>(success);

    pa_threaded_mainloop_signal(pClient->mpMainloop, 0);
}

void CPulseAudioClient::_streamStateChangeCb(pa_stream* s, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);

    switch (pa_stream_get_state(s)) {
    case PA_STREAM_READY:
        AUDIO_IO_LOGD("The stream is ready!");
        pClient->mpListener->onStateChanged(CAudioInfo::AUDIO_IO_STATE_RUNNING);
    case PA_STREAM_FAILED:
    case PA_STREAM_TERMINATED:
        pa_threaded_mainloop_signal(pClient->mpMainloop, 0);
        break;

    case PA_STREAM_UNCONNECTED:
        break;
    case PA_STREAM_CREATING:
        break;
    }
}

void CPulseAudioClient::_streamCaptureCb(pa_stream* s, size_t length, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    assert(pClient->mpListener);

    pClient->mpListener->onStream(pClient, length);
}

void CPulseAudioClient::_streamPlaybackCb(pa_stream* s, size_t length, void* user_data) {
    //AUDIO_IO_LOGD("_streamPlaybackCb()");
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    assert(pClient->mpListener);

    if (pClient->mIsInit == false) {
        AUDIO_IO_LOGD("Occurred this listener when an out stream is on the way to create - Dummy write[length:%d]", length);

        char* dummy = new char[length];
        memset(dummy, 0, length);
        pa_stream_write(s, dummy, length, NULL, 0LL, PA_SEEK_RELATIVE);
        delete [] dummy;

        return;
    }

    pClient->mpListener->onStream(pClient, length);
}

void CPulseAudioClient::_streamLatencyUpdateCb(pa_stream* s, void* user_data) {
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);

    pa_threaded_mainloop_signal(pClient->mpMainloop, 0);
}

void CPulseAudioClient::_successStreamCb(pa_stream* s, int success, void* user_data) {
    AUDIO_IO_LOGD("pa_stream[%p], success[%d], user_data[%p]", s, success, user_data);
    assert(s);
    assert(user_data);

    CPulseAudioClient* pClient = static_cast<CPulseAudioClient*>(user_data);
    pClient->mIsOperationSuccess = static_cast<bool>(success);

    pa_threaded_mainloop_signal(pClient->mpMainloop, 0);
}

void CPulseAudioClient::initialize() throw (CAudioError) {
    AUDIO_IO_LOGD("");
    if (mIsInit == true) {
        return;
    }

    int ret = 0;
    int err = 0;

    try {
        // Allocates PA proplist
        mpPropList = pa_proplist_new();
        if (mpPropList == NULL) {
            THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pa_proplist_new()");
        }

        // Adds values on proplist for delivery to PULSEAUDIO
        char *streamType = NULL;
        CAudioInfo::EAudioType audioType = mSpec.getAudioInfo().getAudioType();
        mSpec.getAudioInfo().convertAudioType2StreamType(audioType, &streamType);
        pa_proplist_sets(mpPropList, PA_PROP_MEDIA_ROLE, streamType);

        int index = mSpec.getAudioInfo().getAudioIndex();
        if (index >= 0) {
            pa_proplist_setf(mpPropList, PA_PROP_MEDIA_PARENT_ID, "%u", (unsigned int) index);
        }

        // Adds latency on proplist for delivery to PULSEAUDIO
        AUDIO_IO_LOGD("LATENCY : %s(%d)", mSpec.getStreamLatencyToString(), mSpec.getStreamLatency());
        pa_proplist_setf(mpPropList, PA_PROP_MEDIA_TIZEN_AUDIO_LATENCY, "%s", mSpec.getStreamLatencyToString());

        // Allocates PA mainloop
        mpMainloop = pa_threaded_mainloop_new();
        if (mpMainloop == NULL) {
            THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pa_threaded_mainloop_new()");
        }

        // Allocates PA context
        mpContext = pa_context_new(pa_threaded_mainloop_get_api(mpMainloop), CLIENT_NAME);
        if (mpContext == NULL) {
            THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pa_context_new()");
        }

        // Sets context state changed callback
        pa_context_set_state_callback(mpContext, _contextStateChangeCb, this);

        // Connects this client with PA server
        if (pa_context_connect(mpContext, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
            THROW_ERROR_MSG(CAudioError::ERROR_OUT_OF_MEMORY, "Failed pa_context_connect()");
        }

        // LOCK for synchronous connection
        pa_threaded_mainloop_lock(mpMainloop);

        // Start mainloop
        if (pa_threaded_mainloop_start(mpMainloop) < 0) {
            pa_threaded_mainloop_unlock(mpMainloop);
            THROW_ERROR_MSG(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_threaded_mainloop_start()");
        }

        // Connection process is asynchronously
        // So, this function will be waited when occurred context state change event
        // If I got a signal, do next processing
        while (true) {
            pa_context_state_t state;
            state = pa_context_get_state(mpContext);

            if (state == PA_CONTEXT_READY) {
                break;
            }

            if (!PA_CONTEXT_IS_GOOD(state)) {
                err = pa_context_errno(mpContext);
                pa_threaded_mainloop_unlock(mpMainloop);
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "pa_context's state is not good err:[%d]", err);
            }

            /* Wait until the context is ready */
            pa_threaded_mainloop_wait(mpMainloop);
        }

        // Allocates PA stream
        pa_sample_spec ss   = mSpec.getSampleSpec();
        pa_channel_map map  = mSpec.getChannelMap();

        mpStream = pa_stream_new_with_proplist(mpContext, mSpec.getStreamName(), &ss, &map, mpPropList);
        if (mpStream == NULL) {
            pa_threaded_mainloop_unlock(mpMainloop);
            THROW_ERROR_MSG(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_new_with_proplist()()");
        }

        // Sets stream callbacks
        pa_stream_set_state_callback(mpStream, _streamStateChangeCb, this);
        pa_stream_set_read_callback(mpStream, _streamCaptureCb, this);
        pa_stream_set_write_callback(mpStream, _streamPlaybackCb, this);
        pa_stream_set_latency_update_callback(mpStream, _streamLatencyUpdateCb, this);

        // Connect stream with PA Server

        if (mDirection == STREAM_DIRECTION_PLAYBACK) {
            pa_stream_flags_t flags = static_cast<pa_stream_flags_t>(
                    PA_STREAM_INTERPOLATE_TIMING |
                    PA_STREAM_ADJUST_LATENCY     |
                    PA_STREAM_AUTO_TIMING_UPDATE);

            ret = pa_stream_connect_playback(mpStream, NULL, NULL, flags, NULL, NULL);
        } else {
            pa_stream_flags_t flags = static_cast<pa_stream_flags_t>(
                    PA_STREAM_INTERPOLATE_TIMING |
                    PA_STREAM_ADJUST_LATENCY     |
                    PA_STREAM_AUTO_TIMING_UPDATE);

            ret = pa_stream_connect_record(mpStream, NULL, NULL, flags);
        }

        if (ret != 0) {
            err = pa_context_errno(mpContext);
            pa_threaded_mainloop_unlock(mpMainloop);
            THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_connect() err:[%d]", err);
        }

        while (true) {
            pa_stream_state_t state;
            state = pa_stream_get_state(mpStream);

            if (state == PA_STREAM_READY) {
                AUDIO_IO_LOGD("STREAM READY");
                break;
            }

            if (!PA_STREAM_IS_GOOD(state)) {
                err = pa_context_errno(mpContext);
                pa_threaded_mainloop_unlock(mpMainloop);
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "pa_stream's state is not good err:[%d]", err);
            }

            /* Wait until the stream is ready */
            pa_threaded_mainloop_wait(mpMainloop);
        }

        // End of synchronous
        pa_threaded_mainloop_unlock(mpMainloop);

        mIsInit = true;
    } catch (CAudioError e) {
        finalize();
        throw e;
    }
}

void CPulseAudioClient::finalize() {
    AUDIO_IO_LOGD("");
    if (mIsInit == false) {
        return;
    }

    if (mpMainloop != NULL) {
        pa_threaded_mainloop_stop(mpMainloop);
    }
    if (mpStream != NULL) {
        pa_stream_disconnect(mpStream);
        mpStream = NULL;
    }

    if (mpContext != NULL) {
        pa_context_disconnect(mpContext);
        pa_context_unref(mpContext);
        mpContext = NULL;
    }

    if (mpMainloop != NULL) {
        pa_threaded_mainloop_free(mpMainloop);
        mpMainloop = NULL;
    }

    if (mpPropList != NULL) {
        pa_proplist_free(mpPropList);
        mpPropList = NULL;
    }

    mIsInit = false;
}

int CPulseAudioClient::peek(const void** data, size_t* length) throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("data:[%p], length:[%p]", data, length);
#endif

    checkRunningState();

    if (data == NULL || length == NULL) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "The parameter is invalid - data:%p, length:%p", data, length);
    }

    if (mDirection == STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    int ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        ret = pa_stream_peek(mpStream, data, length);
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        ret = pa_stream_peek(mpStream, data, length);
    }

    if (ret < 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_peek()");
    }

    return ret;
}

int CPulseAudioClient::drop() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("");
#endif

    checkRunningState();

    if (mDirection == STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    int ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        ret = pa_stream_drop(mpStream);
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        ret = pa_stream_drop(mpStream);
    }

    if (ret < 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_drop()");
    }

    return ret;
}

int CPulseAudioClient::write(const void* data, size_t length) throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("data[%p], length:[%d]", data, length);
#endif

    checkRunningState();

    if (data == NULL || length < 0) {
        THROW_ERROR_MSG(CAudioError::ERROR_INVALID_ARGUMENT, "The parameter is invalid");
    }

    if (mDirection == STREAM_DIRECTION_RECORD) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "The Playback client couldn't use this function");
    }

    int ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        ret = pa_stream_write(mpStream, data, length, NULL, 0LL, PA_SEEK_RELATIVE);
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        ret = pa_stream_write(mpStream, data, length, NULL, 0LL, PA_SEEK_RELATIVE);
    }

    if (ret < 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_write() err:%d", ret);
    }

    return ret;
}

void CPulseAudioClient::cork(bool cork) throw (CAudioError) {
    AUDIO_IO_LOGD("bool cork:%d", cork);

    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    if (isInThread() == true) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "This operation is not supported in callback");
    }

    checkRunningState();

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        pa_operation_unref(pa_stream_cork(mpStream, static_cast<int>(cork), _successStreamCb, this));
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        pa_operation_unref(pa_stream_cork(mpStream, static_cast<int>(cork), _successStreamCb, this));
    }

    return;
}

bool CPulseAudioClient::isCorked() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    int isCork = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        isCork = pa_stream_is_corked(mpStream);
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        isCork = pa_stream_is_corked(mpStream);
    }

    AUDIO_IO_LOGD("isCork:%d", isCork);
    return static_cast<bool>(isCork);
}

bool CPulseAudioClient::drain() throw (CAudioError) {
    AUDIO_IO_LOGD("drain");

    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        pa_operation_unref(pa_stream_drain(mpStream, _successStreamCb, this));
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        pa_operation_unref(pa_stream_drain(mpStream, _successStreamCb, this));
    }

    return true;
}

bool CPulseAudioClient::flush() throw (CAudioError) {
    AUDIO_IO_LOGD("flush");

    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        pa_operation_unref(pa_stream_flush(mpStream, _successStreamCb, this));
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        pa_operation_unref(pa_stream_flush(mpStream, _successStreamCb, this));
    }

    return true;
}

size_t CPulseAudioClient::getWritableSize() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (mDirection != STREAM_DIRECTION_PLAYBACK) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "This client is used for Playback");
    }

    size_t ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        ret = pa_stream_writable_size(mpStream);
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        ret = pa_stream_writable_size(mpStream);
    }

    return ret;
}

void CPulseAudioClient::checkRunningState() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    if (mpContext == NULL || PA_CONTEXT_IS_GOOD(pa_context_get_state(mpContext)) == 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_INITIALIZED, "The context[%p] is not created or not good state", mpContext);
    }
    if (mpStream == NULL || PA_STREAM_IS_GOOD(pa_stream_get_state(mpStream)) == 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_INITIALIZED, "The stream[%p] is not created or not good state", mpStream);
    }
    if (pa_context_get_state(mpContext) != PA_CONTEXT_READY || pa_stream_get_state(mpStream)   != PA_STREAM_READY) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_INITIALIZED, "The context[%p] or stream[%p] state is not ready", mpContext, mpStream);
    }

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("This client is running");
#endif
}

bool CPulseAudioClient::isInThread() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    int ret = pa_threaded_mainloop_in_thread(mpMainloop);

#ifdef _AUDIO_IO_DEBUG_TIMING_
    AUDIO_IO_LOGD("isInThread : [%d][TRUE:1][FALSE:0]", ret);
#endif
    return static_cast<bool>(ret);
}

size_t CPulseAudioClient::getReadableSize() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    if (mDirection != STREAM_DIRECTION_RECORD) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "This client is used for Capture");
    }

    size_t ret = 0;

    if (isInThread() == false) {
        pa_threaded_mainloop_lock(mpMainloop);
        ret = pa_stream_writable_size(mpStream);
        pa_threaded_mainloop_unlock(mpMainloop);
    } else {
        ret = pa_stream_writable_size(mpStream);
    }

    return ret;
}

size_t CPulseAudioClient::getBufferSize() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    size_t ret = 0;

    try {
        if (isInThread() == false) {
            pa_threaded_mainloop_lock(mpMainloop);
        }

        const pa_buffer_attr* attr = pa_stream_get_buffer_attr(mpStream);
        if (attr == NULL) {
            int _err = pa_context_errno(mpContext);
            THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_get_buffer_attr() err:%d", _err);
        }

        if (mDirection == STREAM_DIRECTION_PLAYBACK) {
            ret = attr->tlength;
            AUDIO_IO_LOGD("PLAYBACK buffer size : %d", ret);
        } else {
            ret = attr->fragsize;
            AUDIO_IO_LOGD("RECORD buffer size : %d", ret);
        }
    } catch (CAudioError err) {
        if (isInThread() == false) {
            pa_threaded_mainloop_unlock(mpMainloop);
        }
        throw err;
    }

    if (isInThread() == false) {
        pa_threaded_mainloop_unlock(mpMainloop);
    }

    return ret;
}

pa_usec_t CPulseAudioClient::getLatency() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    pa_usec_t ret = 0;
    int negative  = 0;

    if (isInThread() == false) {
        if (pa_stream_get_latency(mpStream, &ret, &negative) < 0) {
            int _err = pa_context_errno(mpContext);
            if (_err != PA_ERR_NODATA) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_get_latency() err:%d", _err);
            }
        }
        return negative ? 0 : ret;
    }

        pa_threaded_mainloop_lock(mpMainloop);

    try {
        while (true) {
            if (pa_stream_get_latency(mpStream, &ret, &negative) >= 0) {
                break;
            }

            int _err = pa_context_errno(mpContext);
            if (_err != PA_ERR_NODATA) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_FAILED_OPERATION, "Failed pa_stream_get_latency() err:%d", _err);
            }

            /* Wait until latency data is available again */
            pa_threaded_mainloop_wait(mpMainloop);
        }
    } catch (CAudioError e) {
        pa_threaded_mainloop_unlock(mpMainloop);
        throw e;
    }

    pa_threaded_mainloop_unlock(mpMainloop);

    return negative ? 0 : ret;
}

pa_usec_t CPulseAudioClient::getFinalLatency() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Did not initialize CPulseAudioClient");
    }

    checkRunningState();

    pa_usec_t ret = 0;
    uint32_t  ver = 0;

    try {
        if (isInThread() == false) {
            pa_threaded_mainloop_lock(mpMainloop);
        }

        ver = pa_context_get_server_protocol_version(mpContext);
        if (ver >= 13) {
            const pa_buffer_attr* buffer_attr = pa_stream_get_buffer_attr(mpStream);
            const pa_sample_spec* sample_spec = pa_stream_get_sample_spec(mpStream);
            const pa_timing_info* timing_info = pa_stream_get_timing_info(mpStream);

            if (buffer_attr == NULL || sample_spec == NULL || timing_info == NULL) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_OUT_OF_MEMORY, "Failed to get buffer_attr[%p] or sample_spec[%p] or timing_info[%p] from a pa_stream",
                        buffer_attr, sample_spec, timing_info);
            }

            if (mDirection == STREAM_DIRECTION_PLAYBACK) {
                ret = (pa_bytes_to_usec(buffer_attr->tlength, sample_spec) + timing_info->configured_sink_usec);
                AUDIO_IO_LOGD("FINAL PLAYBACK LATENCY : %d", ret);
            } else {
                ret = (pa_bytes_to_usec(buffer_attr->fragsize, sample_spec) + timing_info->configured_source_usec);
                AUDIO_IO_LOGD("FINAL RECORD LATENCY : %d", ret);
            }
        } else {
            THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_SUPPORTED, "This version(ver.%d) is not supported", ver);
        }

        if (isInThread() == false) {
            pa_threaded_mainloop_unlock(mpMainloop);
        }
    } catch (CAudioError e) {
        if (isInThread() == false) {
            pa_threaded_mainloop_unlock(mpMainloop);
        }
        throw e;
    }

    return ret;
}

CPulseAudioClient::EStreamDirection CPulseAudioClient::getStreamDirection() {
    return mDirection;
}

CPulseStreamSpec CPulseAudioClient::getStreamSpec() {
    return mSpec;
}
