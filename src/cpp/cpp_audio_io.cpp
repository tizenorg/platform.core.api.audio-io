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


#include "cpp_audio_io.h"
#include <sound_manager_internal.h>
#include "audio_io.h"
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * Defines Structures
 * type : struct
 * Name : audio_io_interrupted_cb_s
 * Declaration : Keeps user callback pointer and user data for delivering an interrupt event
 */
typedef struct audio_io_interrupted_cb_s {
    void* user_data;
    audio_io_interrupted_cb onInterrupt;

    audio_io_interrupted_cb_s() : user_data(NULL), onInterrupt(NULL)
    {/* Empty Body */}
}   audio_io_interrupted_cb_s;

/**
 * Defines Structures
 * type : struct
 * Name : audio_io_stream_cb_s
 * Declaration : Keeps user callback pointer and user data for delivering an stream event
 */
typedef struct audio_io_stream_cb_s {
    void* user_data;
    audio_in_stream_cb onStream;

    audio_io_stream_cb_s() : user_data(NULL), onStream(NULL)
    {/* Empty Body */}
}   audio_io_stream_cb_s;

/**
 * Defines Structures
 * type : struct
 * Name : audio_io_state_changed_cb_s
 * Declaration : Keeps user callback pointer and user data for delivering an state changed event
 */
typedef struct audio_io_state_changed_cb_s {
    void* user_data;
    audio_in_state_changed_cb onStateChanged;

    audio_io_state_changed_cb_s() : user_data(NULL), onStateChanged(NULL)
    {/* Empty Body */}
}   audio_io_state_changed_cb_s;

/**
 * Defines Structures
 * type : struct
 * Name : audio_io_s
 * Declaration : An handle of AudioIO
 * The handle has two struct for user callback
 * And the handle has a pointer of private audioIO object
 * The CAudioIO is a abstract class object about Input and Output
 */
typedef struct audio_io_s {
    CAudioIO*                    audioIoHandle;
    audio_io_interrupted_cb_s    interrupt_callback;
    audio_io_stream_cb_s         stream_callback;
    audio_io_state_changed_cb_s  state_changed_callback;

    audio_io_s() : audioIoHandle(NULL)
    {/* Empty Body */}
}   audio_io_s;


/**
 * Internal functions
 */
static audio_io_error_e __convert_CAudioError(CAudioError& error) {
    audio_io_error_e    ret = AUDIO_IO_ERROR_NONE;
    CAudioError::EError err = error.getError();

    switch (err) {
    case CAudioError::EError::ERROR_NONE:
        ret = AUDIO_IO_ERROR_NONE;
        break;
    case CAudioError::EError::ERROR_INVALID_ARGUMENT:
    case CAudioError::EError::ERROR_INVALID_HANDLE:
    case CAudioError::EError::ERROR_INVALID_SAMPLERATE:
    case CAudioError::EError::ERROR_INVALID_CHANNEL:
    case CAudioError::EError::ERROR_INVALID_FORMAT:
        ret = AUDIO_IO_ERROR_INVALID_PARAMETER;
        break;
    case CAudioError::EError::ERROR_DEVICE_NOT_OPENED:
        ret = AUDIO_IO_ERROR_DEVICE_NOT_OPENED;
        break;
    case CAudioError::EError::ERROR_DEVICE_NOT_CLOSED:
        ret = AUDIO_IO_ERROR_DEVICE_NOT_CLOSED;
        break;
    case CAudioError::EError::ERROR_PERMISSION_DENIED:
        ret = AUDIO_IO_ERROR_PERMISSION_DENIED;
        break;
    case CAudioError::EError::ERROR_DEVICE_POLICY_RESTRICTION:
        ret = AUDIO_IO_ERROR_DEVICE_POLICY_RESTRICTION;
        break;
    case CAudioError::EError::ERROR_NOT_SUPPORTED:
        ret = AUDIO_IO_ERROR_NOT_SUPPORTED;
        break;
    case CAudioError::EError::ERROR_NOT_SUPPORTED_TYPE:
        ret = AUDIO_IO_ERROR_NOT_SUPPORTED_TYPE;
        break;
    case CAudioError::EError::ERROR_MAX:
    case CAudioError::EError::ERROR_INTERNAL_OPERATION:
    case CAudioError::EError::ERROR_NOT_INITIALIZED:
    case CAudioError::EError::ERROR_FAILED_OPERATION:
    case CAudioError::EError::ERROR_INVALID_OPERATION:
        ret = AUDIO_IO_ERROR_INVALID_OPERATION;
        break;
    case CAudioError::EError::ERROR_OUT_OF_MEMORY:
    case CAudioError::EError::ERROR_INVALID_POINTER:
        ret = AUDIO_IO_ERROR_INVALID_BUFFER;
        break;
    case CAudioError::EError::ERROR_POLICY_BLOCKED:
    case CAudioError::EError::ERROR_POLICY_INTERRUPTED:
    case CAudioError::EError::ERROR_POLICY_DUPLICATED:
        ret = AUDIO_IO_ERROR_SOUND_POLICY;
        break;
    }

    return ret;
}

static void __convert_channel_2_audio_info_channel(const audio_channel_e& src_channel, CAudioInfo::EChannel& dst_channel) {
    switch (src_channel) {
    case AUDIO_CHANNEL_MONO:
        dst_channel = CAudioInfo::EChannel::CHANNEL_MONO;
        break;
    case AUDIO_CHANNEL_STEREO:
        dst_channel = CAudioInfo::EChannel::CHANNEL_STEREO;
        break;
    default:
        dst_channel = CAudioInfo::EChannel::CHANNEL_MONO;
    }
}

static void __convert_audio_info_channel_2_channel(const CAudioInfo::EChannel& src_channel, audio_channel_e& dst_channel) {
    switch (src_channel) {
    case CAudioInfo::EChannel::CHANNEL_MONO:
        dst_channel = AUDIO_CHANNEL_MONO;
        break;
    case CAudioInfo::EChannel::CHANNEL_STEREO:
        dst_channel = AUDIO_CHANNEL_STEREO;
        break;
    default:
        dst_channel = AUDIO_CHANNEL_MONO;
    }
}

static void __convert_sample_type_2_audio_info_sample_type(const audio_sample_type_e& src_type, CAudioInfo::ESampleType& dst_type) {
    switch (src_type) {
    case AUDIO_SAMPLE_TYPE_U8:
        dst_type = CAudioInfo::ESampleType::SAMPLE_TYPE_U8;
        break;
    case AUDIO_SAMPLE_TYPE_S16_LE:
        dst_type = CAudioInfo::ESampleType::SAMPLE_TYPE_S16_LE;
        break;
    default:
        dst_type = CAudioInfo::ESampleType::SAMPLE_TYPE_U8;
    }
}

static void __convert_audio_info_sample_type_2_sample_type(const CAudioInfo::ESampleType& src_type, audio_sample_type_e& dst_type) {
    switch (src_type) {
    case CAudioInfo::ESampleType::SAMPLE_TYPE_U8:
        dst_type = AUDIO_SAMPLE_TYPE_U8;
        break;
    case CAudioInfo::ESampleType::SAMPLE_TYPE_S16_LE:
        dst_type = AUDIO_SAMPLE_TYPE_S16_LE;
        break;
    default:
        dst_type = AUDIO_SAMPLE_TYPE_U8;
    }
}

static void __convert_sound_type_2_audio_info_audio_type(const sound_type_e& src_type, CAudioInfo::EAudioType& dst_type) {
    switch (src_type) {
    case SOUND_TYPE_SYSTEM:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_SYSTEM;
        break;
    case SOUND_TYPE_NOTIFICATION:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_NOTIFICATION;
        break;
    case SOUND_TYPE_ALARM:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_ALARM;
        break;
    case SOUND_TYPE_RINGTONE:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_RINGTONE_VOIP;
        break;
    case SOUND_TYPE_MEDIA:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA;
        break;
    case SOUND_TYPE_CALL:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_SYSTEM;
        break;
    case SOUND_TYPE_VOIP:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_VOIP;
        break;
    case SOUND_TYPE_VOICE:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_VOICE_INFORMATION;
        break;
    default:
        dst_type = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA;
        break;
    }
}

static void __convert_audio_info_audio_type_2_sound_type(const CAudioInfo::EAudioType& src_type, sound_type_e& dst_type) {
    switch (src_type) {
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA:
        dst_type = SOUND_TYPE_MEDIA;
        break;
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_SYSTEM:
        dst_type = SOUND_TYPE_SYSTEM;
        break;
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_ALARM:
        dst_type = SOUND_TYPE_ALARM;
        break;
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_NOTIFICATION:
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_EMERGENCY:
        dst_type = SOUND_TYPE_NOTIFICATION;
        break;
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_VOICE_INFORMATION:
        dst_type = SOUND_TYPE_VOICE;
        break;
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_RINGTONE_VOIP:
        dst_type = SOUND_TYPE_RINGTONE;
        break;
    case CAudioInfo::EAudioType::AUDIO_OUT_TYPE_VOIP:
        dst_type = SOUND_TYPE_VOIP;
        break;
    default:
        dst_type = SOUND_TYPE_MEDIA;
        break;
    }
}

static audio_io_state_e __convert_state_type(const CAudioInfo::EAudioIOState src_state) {
    audio_io_state_e dst_state;
    switch (src_state) {
    case CAudioInfo::EAudioIOState::AUDIO_IO_STATE_NONE:
        dst_state = AUDIO_IO_STATE_IDLE;
        break;
    case CAudioInfo::EAudioIOState::AUDIO_IO_STATE_IDLE:
        dst_state = AUDIO_IO_STATE_IDLE;
        break;
    case CAudioInfo::EAudioIOState::AUDIO_IO_STATE_RUNNING:
        dst_state = AUDIO_IO_STATE_RUNNING;
        break;
    case CAudioInfo::EAudioIOState::AUDIO_IO_STATE_PAUSED:
        dst_state = AUDIO_IO_STATE_PAUSED;
        break;
    default:
        dst_state = AUDIO_IO_STATE_IDLE;
    }
    return dst_state;
}

static void __check_audio_param(int sample_rate, audio_channel_e channel, audio_sample_type_e type) throw(CAudioError) {
    if (sample_rate < 0) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Invalid sample rate :%d", sample_rate);
    }

    if (channel != AUDIO_CHANNEL_MONO && channel != AUDIO_CHANNEL_STEREO) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Invalid channel :%d", channel);
    }

    if (type != AUDIO_SAMPLE_TYPE_U8 && type != AUDIO_SAMPLE_TYPE_S16_LE) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Invalid sample type :%d", type);
    }
}

static void __check_audio_param(int sample_rate, audio_channel_e channel, audio_sample_type_e type, sound_type_e sound_type) throw(CAudioError) {
    __check_audio_param(sample_rate, channel, type);

    if (sound_type < SOUND_TYPE_SYSTEM || sound_type > SOUND_TYPE_VOICE) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Invalid sound type : %d", sound_type);
    }
}

static CAudioInfo __generate_audio_input_info(int sampleRate, audio_channel_e channel, audio_sample_type_e sample_type) throw(CAudioError) {
    CAudioInfo::EChannel     dstChannel;
    CAudioInfo::ESampleType dstSampleType;
    CAudioInfo::EAudioType  dstAudioType = CAudioInfo::EAudioType::AUDIO_IN_TYPE_MEDIA;

    __convert_channel_2_audio_info_channel(channel, dstChannel);
    __convert_sample_type_2_audio_info_sample_type(sample_type, dstSampleType);

    return CAudioInfo(sampleRate, dstChannel, dstSampleType, dstAudioType, -1);
}

static CAudioInfo __generate_audio_input_loopback_info(int sampleRate, audio_channel_e channel, audio_sample_type_e sample_type) throw(CAudioError) {
    CAudioInfo::EChannel     dstChannel;
    CAudioInfo::ESampleType dstSampleType;
    CAudioInfo::EAudioType  dstAudioType = CAudioInfo::EAudioType::AUDIO_IN_TYPE_LOOPBACK;

    __convert_channel_2_audio_info_channel(channel, dstChannel);
    __convert_sample_type_2_audio_info_sample_type(sample_type, dstSampleType);

    return CAudioInfo(sampleRate, dstChannel, dstSampleType, dstAudioType, -1);
}

static CAudioInfo __generate_audio_output_info(int sampleRate, audio_channel_e channel, audio_sample_type_e sample_type, sound_type_e sound_type) throw(CAudioError) {
    CAudioInfo::EChannel     dstChannel;
    CAudioInfo::ESampleType dstSampleType;
    CAudioInfo::EAudioType  dstAudioType;

    __convert_channel_2_audio_info_channel(channel, dstChannel);
    __convert_sample_type_2_audio_info_sample_type(sample_type, dstSampleType);
    __convert_sound_type_2_audio_info_audio_type(sound_type, dstAudioType);

    return CAudioInfo(sampleRate, dstChannel, dstSampleType, dstAudioType, -1);
}

static audio_io_interrupted_code_e __convert_interrupted_code(IAudioSessionEventListener::EInterruptCode code) {
    switch (code) {
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_COMPLETED:
        return AUDIO_IO_INTERRUPTED_COMPLETED;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_CALL:
        return AUDIO_IO_INTERRUPTED_BY_CALL;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_EARJACK_UNPLUG:
        return AUDIO_IO_INTERRUPTED_BY_EARJACK_UNPLUG;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_RESOURCE_CONFLICT:
        return AUDIO_IO_INTERRUPTED_BY_RESOURCE_CONFLICT;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_ALARM:
        return AUDIO_IO_INTERRUPTED_BY_ALARM;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_EMERGENCY:
        return AUDIO_IO_INTERRUPTED_BY_EMERGENCY;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_NOTIFICATION:
        return AUDIO_IO_INTERRUPTED_BY_NOTIFICATION;
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_BY_MEDIA:
    case IAudioSessionEventListener::EInterruptCode::INTERRUPT_MAX:
    default:
        return AUDIO_IO_INTERRUPTED_BY_MEDIA;
    }
}

/**
 * Implements CAPI functions
 */
int cpp_audio_in_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type, audio_in_h *input) {
    audio_io_s* handle = NULL;
    try {
        if (input == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        __check_audio_param(sample_rate, channel, type);

        handle = new audio_io_s;
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation handle");
        }

        CAudioInfo audioInfo = __generate_audio_input_info(sample_rate, channel, type);

        handle->audioIoHandle = new CAudioInput(audioInfo);
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation internal handle");
        }

        handle->audioIoHandle->initialize();

        *input = handle;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());

        VALID_POINTER_START(handle)
            SAFE_FINALIZE(handle->audioIoHandle);
            SAFE_DELETE(handle->audioIoHandle);
            SAFE_DELETE(handle);
        VALID_POINTER_END

        VALID_POINTER_START(input)
            *input = NULL;
        VALID_POINTER_END

        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_create_loopback(int sample_rate, audio_channel_e channel, audio_sample_type_e type , audio_in_h* input) {
    audio_io_s* handle = NULL;
    try {
        if (input == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        __check_audio_param(sample_rate, channel, type);

        handle = new audio_io_s;
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation handle");
        }

        CAudioInfo audioInfo = __generate_audio_input_loopback_info(sample_rate, channel, type);

        handle->audioIoHandle = new CAudioInput(audioInfo);
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation internal handle");
        }

        handle->audioIoHandle->initialize();

        *input = handle;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());

        VALID_POINTER_START(handle)
            SAFE_FINALIZE(handle->audioIoHandle);
            SAFE_DELETE(handle->audioIoHandle);
            SAFE_DELETE(handle);
        VALID_POINTER_END

        VALID_POINTER_START(input)
            *input = NULL;
        VALID_POINTER_END

        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_destroy(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        SAFE_FINALIZE(handle->audioIoHandle);
        SAFE_DELETE(handle->audioIoHandle);
        SAFE_DELETE(handle);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_set_stream_info(audio_in_h input, sound_stream_info_h stream_info) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || stream_info == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, stream_info:%p", input, stream_info);
        }

        assert(handle->audioIoHandle);

        int errorCode = SOUND_MANAGER_ERROR_NONE;
        CAudioInfo::EAudioType audioType = CAudioInfo::EAudioType::AUDIO_IN_TYPE_MEDIA;
        char *type = NULL;
        int index = -1;
        bool avail = false;

        if ((errorCode = sound_manager_is_available_stream_information(stream_info, NATIVE_API_AUDIO_IO, &avail)) != SOUND_MANAGER_ERROR_NONE) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter stream_info is invalid [ret:%d]", errorCode);
        }

        if (avail) {
            if ((errorCode = sound_manager_get_type_from_stream_information(stream_info, &type)) != SOUND_MANAGER_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter stream_info->stream_type is invalid [ret:%d]", errorCode);
            }
            handle->audioIoHandle->getAudioInfo().convertInputStreamType2AudioType(type, &audioType);
            handle->audioIoHandle->getAudioInfo().setAudioType(audioType);

            if ((errorCode = sound_manager_get_index_from_stream_information(stream_info, &index)) != SOUND_MANAGER_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter stream_info->index is invalid [ret:%d]", errorCode);
            }
            handle->audioIoHandle->getAudioInfo().setAudioIndex(index);
        } else {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_SUPPORTED_TYPE, "Input stream is not supported");
        }
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_prepare(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->prepare();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_unprepare(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->unprepare();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_pause(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->pause();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_resume(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->resume();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_drain(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->drain();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_flush(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->flush();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_read(audio_in_h input, void *buffer, unsigned int length) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);
    int ret = 0;

    try {
        if (handle == NULL || buffer == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, buffer:%p", input, buffer);
        }

        assert(handle->audioIoHandle);

        CAudioInput* inputHandle = dynamic_cast<CAudioInput*>(handle->audioIoHandle);
        if (inputHandle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_HANDLE, "Handle is NULL");
        }
        size_t readn = inputHandle->read(buffer, static_cast<size_t>(length));
        ret = static_cast<int>(readn);
#ifdef _AUDIO_IO_DEBUG_TIMING_
        AUDIO_IO_LOGD("readn:%d", readn);
#endif
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return ret;
}

int cpp_audio_in_get_buffer_size(audio_in_h input, int *size) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || size == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, size:%p", input, size);
        }

        assert(handle->audioIoHandle);

        CAudioIO* inputHandle = dynamic_cast<CAudioInput*>(handle->audioIoHandle);
        if (inputHandle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_HANDLE, "Handle is NULL");
        }
        *size = inputHandle->getBufferSize();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_get_sample_rate(audio_in_h input, int *sample_rate) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || sample_rate == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, sample_rate:%p", input, sample_rate);
        }

        assert(handle->audioIoHandle);
        *sample_rate = handle->audioIoHandle->getAudioInfo().getSampleRate();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_get_channel(audio_in_h input, audio_channel_e *channel) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || channel == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, channel:%p", input, channel);
        }

        assert(handle->audioIoHandle);

        const CAudioInfo::EChannel srcChannel = handle->audioIoHandle->getAudioInfo().getChannel();
        audio_channel_e dstChannel = AUDIO_CHANNEL_MONO;
        __convert_audio_info_channel_2_channel(srcChannel, dstChannel);

        *channel = dstChannel;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_get_sample_type(audio_in_h input, audio_sample_type_e *type) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || type == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, type:%p", input, type);
        }

        assert(handle->audioIoHandle);

        const CAudioInfo::ESampleType srcSampleType = handle->audioIoHandle->getAudioInfo().getSampleType();
        audio_sample_type_e     dstSampleType = AUDIO_SAMPLE_TYPE_U8;
        __convert_audio_info_sample_type_2_sample_type(srcSampleType, dstSampleType);

        *type = dstSampleType;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

static void __interrupt_cb_internal(IAudioSessionEventListener::EInterruptCode _code, void* user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(user_data);
    audio_io_interrupted_code_e code = __convert_interrupted_code(_code);

    assert(handle);

    if (handle->interrupt_callback.onInterrupt != NULL) {
        handle->interrupt_callback.onInterrupt(code, handle->interrupt_callback.user_data);
    }
}

int cpp_audio_in_set_interrupted_cb(audio_in_h input, audio_io_interrupted_cb callback, void *user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || callback == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, callback:%p", input, callback);
        }

        assert(handle->audioIoHandle);

        handle->interrupt_callback.onInterrupt = callback;
        handle->interrupt_callback.user_data    = user_data;

        CAudioIO::SInterruptCallback cb = handle->audioIoHandle->getInterruptCallback();
        cb.mUserData   = static_cast<void*>(handle);
        cb.onInterrupt = __interrupt_cb_internal;

        handle->audioIoHandle->setInterruptCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_unset_interrupted_cb(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->interrupt_callback.onInterrupt = NULL;
        handle->interrupt_callback.user_data    = NULL;

        CAudioIO::SInterruptCallback cb = handle->audioIoHandle->getInterruptCallback();
        cb.mUserData   = NULL;
        cb.onInterrupt = NULL;

        handle->audioIoHandle->setInterruptCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_ignore_session(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        if (handle->stream_callback.onStream) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_INVALID_OPERATION, "Not support ignore session in async mode");
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->ignoreSession();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

static void __stream_cb_internal(size_t nbytes, void *user_data) {
    audio_io_s* audioIo = static_cast<audio_io_s*>(user_data);
    assert(audioIo);

    if (audioIo->stream_callback.onStream != NULL) {
        audioIo->stream_callback.onStream(audioIo, nbytes, audioIo->stream_callback.user_data);
    }
}

static void __state_changed_cb_internal(CAudioInfo::EAudioIOState state, CAudioInfo::EAudioIOState state_prev, bool by_policy, void *user_data) {
    audio_io_s* audioIo = static_cast<audio_io_s*>(user_data);
    assert(audioIo);

    if (audioIo->state_changed_callback.onStateChanged != NULL) {
        audioIo->state_changed_callback.onStateChanged(audioIo, __convert_state_type(state_prev), __convert_state_type(state), by_policy, audioIo->state_changed_callback.user_data);
    }
}

int cpp_audio_in_set_stream_cb(audio_in_h input, audio_in_stream_cb callback, void* user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || callback == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, callback:%p", input, callback);
        }

        assert(handle->audioIoHandle);

        handle->stream_callback.onStream = callback;
        handle->stream_callback.user_data = user_data;

        CAudioIO::SStreamCallback cb = handle->audioIoHandle->getStreamCallback();
        cb.mUserData = static_cast<void*>(handle);
        cb.onStream  = __stream_cb_internal;

        handle->audioIoHandle->setStreamCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_unset_stream_cb(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->stream_callback.onStream = NULL;
        handle->stream_callback.user_data = NULL;

        CAudioIO::SStreamCallback cb = handle->audioIoHandle->getStreamCallback();
        cb.mUserData = NULL;
        cb.onStream  = NULL;

        handle->audioIoHandle->setStreamCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_peek(audio_in_h input, const void **buffer, unsigned int *length) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);
    size_t _length = 0;

    try {
        if (handle == NULL || buffer == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, buffer:%p", input, buffer);
        }

        CAudioInput* inputHandle = dynamic_cast<CAudioInput*>(handle->audioIoHandle);
        assert(inputHandle);

        inputHandle->peek(buffer, &_length);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    *length = (unsigned int)_length;

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_drop(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p", input);
        }

        CAudioInput* inputHandle = dynamic_cast<CAudioInput*>(handle->audioIoHandle);
        assert(inputHandle);

        inputHandle->drop();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_set_state_changed_cb(audio_in_h input, audio_in_state_changed_cb callback, void* user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL || callback == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL input:%p, callback:%p", input, callback);
        }

        assert(handle->audioIoHandle);

        handle->state_changed_callback.onStateChanged = callback;
        handle->state_changed_callback.user_data = user_data;

        CAudioIO::SStateChangedCallback cb = handle->audioIoHandle->getStateChangedCallback();
        cb.mUserData = static_cast<void*>(handle);
        cb.onStateChanged = __state_changed_cb_internal;

        handle->audioIoHandle->setStateChangedCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_in_unset_state_changed_cb(audio_in_h input) {
    audio_io_s* handle = static_cast<audio_io_s*>(input);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", input);
        }

        assert(handle->audioIoHandle);

        handle->state_changed_callback.onStateChanged = NULL;
        handle->state_changed_callback.user_data = NULL;

        CAudioIO::SStateChangedCallback cb = handle->audioIoHandle->getStateChangedCallback();
        cb.mUserData = NULL;
        cb.onStateChanged  = NULL;

        handle->audioIoHandle->setStateChangedCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}


/**
 * Audio Out
 */
int cpp_audio_out_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type, sound_type_e sound_type, audio_out_h *output) {
    audio_io_s* handle = NULL;
    try {
        if (output == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", output);
        }

        __check_audio_param(sample_rate, channel, type, sound_type);

        handle = new audio_io_s;
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation handle");
        }

        CAudioInfo audioInfo = __generate_audio_output_info(sample_rate, channel, type, sound_type);

        handle->audioIoHandle = new CAudioOutput(audioInfo);
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation internal handle");
        }

        handle->audioIoHandle->initialize();

        *output = handle;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());

        VALID_POINTER_START(handle)
            SAFE_FINALIZE(handle->audioIoHandle);
            SAFE_DELETE(handle->audioIoHandle);
            SAFE_DELETE(handle);
        VALID_POINTER_END

        VALID_POINTER_START(output)
            *output = NULL;
        VALID_POINTER_END

        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_create_new(int sample_rate, audio_channel_e channel, audio_sample_type_e type, audio_out_h *output) {
    audio_io_s* handle = NULL;
    try {
        if (output == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", output);
        }

        __check_audio_param(sample_rate, channel, type, SOUND_TYPE_SYSTEM /*default check */);

        handle = new audio_io_s;
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation handle");
        }

        CAudioInfo audioInfo = __generate_audio_output_info(sample_rate, channel, type, SOUND_TYPE_MEDIA /* default sound_type */);

        handle->audioIoHandle = new CAudioOutput(audioInfo);
        if (handle == NULL) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_OUT_OF_MEMORY, "Failed allocation internal handle");
        }

        handle->audioIoHandle->initialize();

        *output = handle;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());

        VALID_POINTER_START(handle)
            SAFE_FINALIZE(handle->audioIoHandle);
            SAFE_DELETE(handle->audioIoHandle);
            SAFE_DELETE(handle);
        VALID_POINTER_END

        VALID_POINTER_START(output)
            *output = NULL;
        VALID_POINTER_END

        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_destroy(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        SAFE_FINALIZE(handle->audioIoHandle);
        SAFE_DELETE(handle->audioIoHandle);
        SAFE_DELETE(handle);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_set_stream_info(audio_out_h output, sound_stream_info_h stream_info) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || stream_info == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, stream_info:%p", output, stream_info);
        }

        assert(handle->audioIoHandle);

        int errorCode = SOUND_MANAGER_ERROR_NONE;
        CAudioInfo::EAudioType audioType = CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA;
        char *type = NULL;
        int index = -1;
        bool avail = false;

        if ((errorCode = sound_manager_is_available_stream_information(stream_info, NATIVE_API_AUDIO_IO, &avail)) != SOUND_MANAGER_ERROR_NONE) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter stream_info is invalid [ret:%d]", errorCode);
        }

        if (avail) {
            if ((errorCode = sound_manager_get_type_from_stream_information(stream_info, &type)) != SOUND_MANAGER_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter stream_info->stream_type is invalid [ret:%d]", errorCode);
            }
            handle->audioIoHandle->getAudioInfo().convertOutputStreamType2AudioType(type, &audioType);
            handle->audioIoHandle->getAudioInfo().setAudioType(audioType);

            if ((errorCode = sound_manager_get_index_from_stream_information(stream_info, &index)) != SOUND_MANAGER_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter stream_info->index is invalid [ret:%d]", errorCode);
            }
            handle->audioIoHandle->getAudioInfo().setAudioIndex(index);
        } else {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_SUPPORTED_TYPE, "Output stream is not supported");
        }
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_prepare(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->prepare();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_unprepare(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->unprepare();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_pause(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->pause();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_resume(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->resume();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_drain(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->drain();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_flush(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->flush();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_write(audio_out_h output, void *buffer, unsigned int length) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);
    int ret = 0;

    try {
        if (handle == NULL || buffer == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameter is NULL output:%p, buffer:%p", output, buffer);
        }

        assert(handle->audioIoHandle);

        CAudioOutput* outputHandle = dynamic_cast<CAudioOutput*>(handle->audioIoHandle);
        if (outputHandle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_HANDLE, "Handle is NULL");
        }
        size_t writen = outputHandle->write(buffer, static_cast<size_t>(length));
        ret = static_cast<int>(writen);
#ifdef _AUDIO_IO_DEBUG_TIMING_
        AUDIO_IO_LOGD("writen:%d", writen);
#endif
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return ret;
}

int cpp_audio_out_get_buffer_size(audio_out_h output, int *size) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || size == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, size:%p", output, size);
        }

        assert(handle->audioIoHandle);

        CAudioOutput* outputHandle = dynamic_cast<CAudioOutput*>(handle->audioIoHandle);
        if (outputHandle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_HANDLE, "Handle is NULL");
        }
        *size = outputHandle->getBufferSize();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_get_sample_rate(audio_out_h output, int *sample_rate) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || sample_rate == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, sample_rate:%p", output, sample_rate);
        }

        assert(handle->audioIoHandle);
        *sample_rate = handle->audioIoHandle->getAudioInfo().getSampleRate();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_get_channel(audio_out_h output, audio_channel_e *channel) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || channel == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, channel:%p", output, channel);
        }

        assert(handle->audioIoHandle);

        const CAudioInfo::EChannel srcChannel = handle->audioIoHandle->getAudioInfo().getChannel();
        audio_channel_e dstChannel = AUDIO_CHANNEL_MONO;
        __convert_audio_info_channel_2_channel(srcChannel, dstChannel);

        *channel = dstChannel;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_get_sample_type(audio_out_h output, audio_sample_type_e *type) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || type == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, type:%p", output, type);
        }

        assert(handle->audioIoHandle);

        const CAudioInfo::ESampleType srcSampleType = handle->audioIoHandle->getAudioInfo().getSampleType();
        audio_sample_type_e     dstSampleType = AUDIO_SAMPLE_TYPE_U8;
        __convert_audio_info_sample_type_2_sample_type(srcSampleType, dstSampleType);

        *type = dstSampleType;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_get_sound_type(audio_out_h output, sound_type_e *type) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || type == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, type:%p", output, type);
        }

        assert(handle->audioIoHandle);

        const CAudioInfo::EAudioType srcAudioType = handle->audioIoHandle->getAudioInfo().getAudioType();
        sound_type_e                 dstSoundType = SOUND_TYPE_MEDIA;
        __convert_audio_info_audio_type_2_sound_type(srcAudioType, dstSoundType);

        *type = dstSoundType;
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_set_interrupted_cb(audio_out_h output, audio_io_interrupted_cb callback, void *user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || callback == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, callback:%p", output, callback);
        }

        assert(handle->audioIoHandle);

        handle->interrupt_callback.onInterrupt = callback;
        handle->interrupt_callback.user_data    = user_data;

        CAudioIO::SInterruptCallback cb = handle->audioIoHandle->getInterruptCallback();
        cb.mUserData   = static_cast<void*>(handle);
        cb.onInterrupt = __interrupt_cb_internal;

        handle->audioIoHandle->setInterruptCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_unset_interrupted_cb(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->interrupt_callback.onInterrupt = NULL;
        handle->interrupt_callback.user_data    = NULL;

        CAudioIO::SInterruptCallback cb = handle->audioIoHandle->getInterruptCallback();
        cb.mUserData   = NULL;
        cb.onInterrupt = NULL;

        handle->audioIoHandle->setInterruptCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_ignore_session(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", output);
        }

        if (handle->stream_callback.onStream) {
            THROW_ERROR_MSG(CAudioError::EError::ERROR_INVALID_OPERATION, "Not support ignore session in async mode");
        }

        assert(handle->audioIoHandle);

        handle->audioIoHandle->ignoreSession();
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_set_stream_cb(audio_out_h output, audio_out_stream_cb callback, void* user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || callback == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, callback:%p", output, callback);
        }

        assert(handle->audioIoHandle);

        handle->stream_callback.onStream = callback;
        handle->stream_callback.user_data = user_data;

        CAudioIO::SStreamCallback cb = handle->audioIoHandle->getStreamCallback();
        cb.mUserData = static_cast<void*>(handle);
        cb.onStream  = __stream_cb_internal;

        handle->audioIoHandle->setStreamCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_unset_stream_cb(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->stream_callback.onStream = NULL;
        handle->stream_callback.user_data = NULL;

        CAudioIO::SStreamCallback cb = handle->audioIoHandle->getStreamCallback();
        cb.mUserData = NULL;
        cb.onStream  = NULL;

        handle->audioIoHandle->setStreamCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_set_state_changed_cb(audio_out_h output, audio_in_state_changed_cb callback, void* user_data) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL || callback == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p, callback:%p", output, callback);
        }

        assert(handle->audioIoHandle);

        handle->state_changed_callback.onStateChanged = callback;
        handle->state_changed_callback.user_data = user_data;

        CAudioIO::SStateChangedCallback cb = handle->audioIoHandle->getStateChangedCallback();
        cb.mUserData = static_cast<void*>(handle);
        cb.onStateChanged = __state_changed_cb_internal;

        handle->audioIoHandle->setStateChangedCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}

int cpp_audio_out_unset_state_changed_cb(audio_out_h output) {
    audio_io_s* handle = static_cast<audio_io_s*>(output);

    try {
        if (handle == NULL) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "Parameters are NULL output:%p", output);
        }

        assert(handle->audioIoHandle);

        handle->state_changed_callback.onStateChanged = NULL;
        handle->state_changed_callback.user_data = NULL;

        CAudioIO::SStateChangedCallback cb = handle->audioIoHandle->getStateChangedCallback();
        cb.mUserData = NULL;
        cb.onStateChanged  = NULL;

        handle->audioIoHandle->setStateChangedCallback(cb);
    } catch (CAudioError e) {
        AUDIO_IO_LOGE("%s", e.getErrorMsg());
        return __convert_CAudioError(e);
    }

    return AUDIO_IO_ERROR_NONE;
}
