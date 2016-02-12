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


#include <unistd.h>
#include <mm_error.h>
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioSessionHandler
 */
int CAudioSessionHandler::__sCaptureRef = 0;

int CAudioSessionHandler::__pcmCaptureCountInc() {
    int actual;
    do {
        actual = __sCaptureRef;
    } while (!__sync_bool_compare_and_swap(&__sCaptureRef, actual, actual + 1));
    AUDIO_IO_LOGD("CaptureRefCount+1 > [%d]", __sCaptureRef);
    return __sCaptureRef;
}

int CAudioSessionHandler::__pcmCaptureCountDec() {
    int actual;
    do {
        actual = __sCaptureRef;
    } while (!__sync_bool_compare_and_swap(&__sCaptureRef, actual, actual - 1));
    AUDIO_IO_LOGD("CaptureRefCount-1 > [%d]", __sCaptureRef);
    if (__sCaptureRef < 0) {
        AUDIO_IO_LOGE("A CaptureRef[%d] is not valid! Something is wrong!", __sCaptureRef);
        __sCaptureRef = 0;
    }
    return __sCaptureRef;
}

int CAudioSessionHandler::__pcmCaptureCountGet() {
    AUDIO_IO_LOGD("CaptureRefCount > [%d]", __sCaptureRef);
    return __sCaptureRef;
}

int CAudioSessionHandler::__sFocusRef = 0;

int CAudioSessionHandler::__focusIdCountInc() {
    int actual;
    do {
        actual = __sFocusRef;
    } while (!__sync_bool_compare_and_swap(&__sFocusRef, actual, actual + 1));
    AUDIO_IO_LOGD("FocusRefCount+1 > [%d]", __sFocusRef);
    return __sFocusRef;
}

int CAudioSessionHandler::__focusIdCountDec() {
    int actual;
    do {
        actual = __sFocusRef;
    } while (!__sync_bool_compare_and_swap(&__sFocusRef, actual, actual - 1));
    AUDIO_IO_LOGD("FocusRefCount-1 > [%d]", __sFocusRef);
    return __sFocusRef;
}

int CAudioSessionHandler::__focusIdCountGet() {
    /* AUDIO_IO_LOGD("FocusRefCount > [%d]", __sFocusRef); */
    return __sFocusRef;
}

CAudioSessionHandler::CAudioSessionHandler(EAudioSessionType sessionType, CAudioInfo& audioInfo, IAudioSessionEventListener* listener) :
    __mId(-1),
    __mOptions(0),
    __mAudioSession(sessionType),
    __mMultimediaSession(MM_SESSION_TYPE_MEDIA),
    __mpEventListener(listener),
    __mIsInit(false),
    __mSubscribeId(-1),
    __mUseFocus(false),
    __mFocusType(FOCUS_NONE),
    __mState(FOCUS_IS_RELEASED),
    __mReasonForChange(NULL),
    __mAdditionalInfo(NULL) {
    __mAudioInfo = audioInfo;
}

CAudioSessionHandler::~CAudioSessionHandler() {
}

CAudioError CAudioSessionHandler::__convertStreamType(EAudioSessionType type1, MMSessionType type2, int *index) {
    unsigned int i;
    int idx = -1;

    assert(index != NULL);

    if (type1 == EAudioSessionType::AUDIO_SESSION_TYPE_CAPTURE) {
        for (i = 0 ; i < sizeof(__STREAM_TYPE_TABLE_IN) / sizeof(__STREAM_TYPE_TABLE_IN[0]) ; i++) {
            if (__STREAM_TYPE_TABLE_IN[i].type == type2) {
                idx = i;
                break;
            }
        }
    } else {
        for (i = 0 ; i < sizeof(__STREAM_TYPE_TABLE_OUT) / sizeof(__STREAM_TYPE_TABLE_OUT[0]) ; i++) {
            if (__STREAM_TYPE_TABLE_OUT[i].type == type2) {
                idx = i;
                break;
            }
        }
    }

    if (idx < 0) {
        RET_ERROR_MSG(CAudioError::EError::ERROR_NOT_SUPPORTED, "Does not support session type.");
    }
    *index = idx;
    RET_ERROR(CAudioError::EError::ERROR_NONE);
}

CAudioError CAudioSessionHandler::__getAsmInformation(MMSessionType *type, int *options) {
    assert(type != NULL);
    assert(options != NULL);

    MMSessionType currentSession = MM_SESSION_TYPE_MEDIA;
    int           sessionOptions = 0;

    /* Read session information */
    int ret = 0;
    if ((ret = _mm_session_util_read_information(-1, (int*)&currentSession, &sessionOptions)) < 0) {
        if (ret == (int) MM_ERROR_INVALID_HANDLE) {
            RET_ERROR_MSG(CAudioError::EError::ERROR_INVALID_HANDLE, "Failed _mm_session_util_read_information(). Invalid handle");
        } else {
            RET_ERROR_MSG(CAudioError::EError::ERROR_FAILED_OPERATION, "Failed _mm_session_util_read_information(). Not exist");
        }
    }

    *type    = currentSession;
    *options = sessionOptions;

    RET_ERROR(CAudioError::EError::ERROR_NONE);
}

bool CAudioSessionHandler::__isFocusRequired(MMSessionType type, int options) {
    if ((options & MM_SESSION_OPTION_PAUSE_OTHERS)
        || ((type != MM_SESSION_TYPE_MEDIA) && (type != MM_SESSION_TYPE_MEDIA_RECORD)))
        return true;
    else
        return false;
}

int CAudioSessionHandler::getId() {
    return __mId;
}

int CAudioSessionHandler::getOptions() {
    return __mOptions;
}

CAudioSessionHandler::EAudioSessionType CAudioSessionHandler::getAudioSession() {
    return __mAudioSession;
}

MMSessionType CAudioSessionHandler::getMultimediaSession() {
    return __mMultimediaSession;
}

int CAudioSessionHandler::getSubscribeId() {
    return __mSubscribeId;
}

CAudioInfo CAudioSessionHandler::getAudioInfo() {
    return __mAudioInfo;
}

void CAudioSessionHandler::__sound_pcm_signal_cb(mm_sound_signal_name_t signal, int value, void *user_data) {
    assert(user_data);

    AUDIO_IO_LOGD("[signal:%d], [value:%d], [user_data:0x%x]", signal, value, user_data);

    CAudioSessionHandler* pHandler = static_cast<CAudioSessionHandler*>(user_data);
    if (pHandler->__mpEventListener != NULL) {
        pHandler->__mpEventListener->onSignal(pHandler, signal, value);
    }
}

void CAudioSessionHandler::initialize() throw (CAudioError) {
    AUDIO_IO_LOGD("");
    if (__mIsInit == true) {
        return;
    }

    MMSessionType currentSession = MM_SESSION_TYPE_MEDIA;
    int           sessionOptions = 0;  // Mix with others by default

    CAudioError err = __getAsmInformation(&currentSession, &sessionOptions);
    if (err == CAudioError::EError::ERROR_NONE) {
        if (currentSession == MM_SESSION_TYPE_REPLACED_BY_STREAM) {
            __mUseFocus = false;
            AUDIO_IO_LOGD("Stream info. was created outside, skip audio focus concept internally!");
        } else {
            // Session was configured before, use focus callback
            __mUseFocus = true;
            AUDIO_IO_LOGD("Use audio focus concept internally!");
            }
    } else {
        if (err == CAudioError::EError::ERROR_INVALID_HANDLE) {
            // No session, No stream_info, No focus watch callback before
            // Use focus watch callback with signal subscribe
            unsigned int subscribe_id;
            int errorCode = mm_sound_subscribe_signal(MM_SOUND_SIGNAL_RELEASE_INTERNAL_FOCUS, &subscribe_id, __sound_pcm_signal_cb, static_cast<void*>(this));
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_get_signal_value() err:0x%x", errorCode);
            }

            __mSubscribeId = (int)subscribe_id;
            AUDIO_IO_LOGD("Subscribed mm_sound signal");

            sessionOptions = 0;  // Mix with others by default
            __mUseFocus = true;
            AUDIO_IO_LOGD("Use audio focus(watch) concept internally!");
        } else {
            __mUseFocus = false;
            AUDIO_IO_LOGD("Skip audio focus concept!");
        }

        if (__mAudioSession == EAudioSessionType::AUDIO_SESSION_TYPE_CAPTURE) {
            AUDIO_IO_LOGD("Set default \"Media_Record\" type");
            currentSession = MM_SESSION_TYPE_MEDIA_RECORD;
        } else {
            AUDIO_IO_LOGD("Set default \"Media\" type");
            currentSession = MM_SESSION_TYPE_MEDIA;
        }
    }

    // Updates session information
    __mMultimediaSession = currentSession;
    __mOptions           = sessionOptions;

    if (this->__mAudioSession == EAudioSessionType::AUDIO_SESSION_TYPE_CAPTURE) {
        __pcmCaptureCountInc();
    }

    __mIsInit = true;
}

void CAudioSessionHandler::finalize() {
    AUDIO_IO_LOGD("");
    if (__mIsInit == false) {
        return;
    }

    if (__mAudioSession == EAudioSessionType::AUDIO_SESSION_TYPE_CAPTURE) {
        __pcmCaptureCountDec();
    }

    if (__mSubscribeId >= 0) {
        mm_sound_unsubscribe_signal(__mSubscribeId);
    }

    __mIsInit = false;
}

bool CAudioSessionHandler::isSkipSessionEvent() throw (CAudioError) {
    bool ret = false;

    // To be regarded...
#if 0
    /* Only Support below Event */
    if (mEvent != ASM_EVENT_CALL              && mEvent != ASM_EVENT_VOIP              &&
        mEvent != ASM_EVENT_VIDEOCALL         && mEvent != ASM_EVENT_VOICE_RECOGNITION &&
        mEvent != ASM_EVENT_MMCAMCORDER_AUDIO && mEvent != ASM_EVENT_MMCAMCORDER_VIDEO) {

        // Check AudioType
        switch (__mAudioInfo.getAudioType()) {
        case CAudioInfo::AUDIO_IN_TYPE_MEDIA:
        case CAudioInfo::AUDIO_IN_TYPE_VOICECONTROL:
            ret = false;
            break;

        case CAudioInfo::AUDIO_IN_TYPE_MIRRORING:
        case CAudioInfo::AUDIO_IN_TYPE_LOOPBACK:
            ret = true;
            break;

        default:
            return false;
        }

        if (ret == true) {
            int captureCount = CAudioSessionHandler::__pcmCaptureCountGet();
            if (captureCount == 1) {/* If this is last one */
                /* Recover session information to MEDIA */
                int sessionResult = _mm_session_util_write_information(-1, MM_SESSION_TYPE_MEDIA, __mOptions);
                if (sessionResult != 0) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "Failed _mm_session_util_write_information() ret:%d", sessionResult);
                }
            }
        }
    }
#endif

    return ret;
}

void CAudioSessionHandler::__sound_pcm_focus_cb(int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data) {
    assert(user_data);

    AUDIO_IO_LOGD("[id:%d], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s], [user_data:0x%x]", id, focus_type, state, reason_for_change, additional_info, user_data);

    CAudioSessionHandler* pHandler = static_cast<CAudioSessionHandler*>(user_data);
    pHandler->__mFocusType       = focus_type;
    pHandler->__mState           = state;
    pHandler->__mReasonForChange = (char *)reason_for_change;
    pHandler->__mAdditionalInfo  = (char *)additional_info;

    if (pHandler->__mpEventListener != NULL) {
        pHandler->__mpEventListener->onInterrupt(pHandler, id, focus_type, state, reason_for_change, additional_info);
    }

    return;
}

void CAudioSessionHandler::__sound_pcm_focus_watch_cb(int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data) {
    AUDIO_IO_LOGD("[id:%d], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s], [user_data:0x%x]", id, focus_type, state, reason_for_change, additional_info, user_data);

    CAudioSessionHandler::__sound_pcm_focus_cb(-1, focus_type, state, reason_for_change, additional_info, user_data);

    return;
}

void CAudioSessionHandler::registerSound() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (__mUseFocus == true) {
        if (__mId >= 0) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Already registered [id:%d]", __mId);
        }

        int errorCode = 0;

        if (__isFocusRequired(__mMultimediaSession, __mOptions)) {
            int index = 0;
            CAudioError err = __convertStreamType(__mAudioSession, __mMultimediaSession, &index);
            if (err != CAudioError::EError::ERROR_NONE) {
                throw err;
            }

            errorCode = mm_sound_focus_get_id(&__mId);
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_focus_get_id() err:0x%x", errorCode);
            }

            // Register focus callback
            errorCode = mm_sound_register_focus_for_session(__mId,
                                                getpid(),
                                                __mAudioSession == EAudioSessionType::AUDIO_SESSION_TYPE_CAPTURE ? __STREAM_TYPE_TABLE_IN[index].name : __STREAM_TYPE_TABLE_OUT[index].name,
                                                __sound_pcm_focus_cb,
                                                static_cast<void*>(this));
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_register_focus_for_session() err:0x%x", errorCode);
            }

            __focusIdCountInc();

            AUDIO_IO_LOGD("Focus callback registered successfully [id:%d]", __mId);
        } else if (!(__mOptions & MM_SESSION_OPTION_UNINTERRUPTIBLE)) {
            // Register focus watch callback
            errorCode = mm_sound_set_focus_watch_callback_for_session(getpid(), FOCUS_FOR_BOTH, __sound_pcm_focus_watch_cb, static_cast<void*>(this), &__mId);
            if (errorCode < 0) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_set_focus_watch_callback_for_session() err:0x%x", errorCode);
            }

            __focusIdCountInc();

            AUDIO_IO_LOGD("Focus watch callback registered successfully [id:%d]", __mId);
        }
    }
}

void CAudioSessionHandler::unregisterSound() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (__mUseFocus == true) {
        if (__mId < 0) {
            THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Did not register [id:%d]", __mId);
        }

        int errorCode = 0;

        if (__isFocusRequired(__mMultimediaSession, __mOptions)) {
            // Unregister focus callback
            errorCode = mm_sound_unregister_focus(__mId);
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_unregister_focus() err:0x%x", errorCode);
            }

            __focusIdCountDec();

            AUDIO_IO_LOGD("Focus callback unregistered successfully [id:%d]", __mId);
            __mId = -1;
        } else if (!(__mOptions & MM_SESSION_OPTION_UNINTERRUPTIBLE)) {
            // Unregister focus watch callback.
            errorCode = mm_sound_unset_focus_watch_callback(__mId);
            if (errorCode < 0) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_unset_focus_watch_callback() err:0x%x", errorCode);
            }

            __focusIdCountDec();

            AUDIO_IO_LOGD("Focus watch callback unregistered successfully [id:%d]", __mId);
            __mId = -1;
        }
    }
}

void CAudioSessionHandler::updatePlaying() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (__mUseFocus && __isFocusRequired(__mMultimediaSession, __mOptions)) {
        if (__mId >= 0) {
            int ret = mm_sound_acquire_focus(__mId, FOCUS_FOR_BOTH, "audio-io acquire focus");
            if (ret != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_acquire_focus() err:0x%x", ret);
            }
            AUDIO_IO_LOGD("Focus acquired successfully [id:%d]", __mId);
        }
    }
}

void CAudioSessionHandler::updateStop() throw (CAudioError) {
    if (__mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::EError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (__mUseFocus && __isFocusRequired(__mMultimediaSession, __mOptions)) {
        if (__mId >= 0) {
            int ret = mm_sound_release_focus(__mId, FOCUS_FOR_BOTH, "audio-io release focus");
            if (ret != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_POLICY_BLOCKED, "Failed mm_sound_release_focus() err:0x%x", ret);
            }
            AUDIO_IO_LOGD("Focus released successfully [id:%d]", __mId);
        }
    }
}

void CAudioSessionHandler::disableSessionHandler() throw (CAudioError) {
    CAudioSessionHandler::updateStop();
    CAudioSessionHandler::unregisterSound();

    CAudioSessionHandler::__mUseFocus = false;
}

/**
 * class IAudioSessionEventListener
 */
IAudioSessionEventListener::EInterruptCode IAudioSessionEventListener::convertInterruptedCode(int code, const char *reason_for_change) {
    EInterruptCode e = EInterruptCode::INTERRUPT_COMPLETED;

    switch (code)
    {
    case FOCUS_IS_ACQUIRED:
        e = EInterruptCode::INTERRUPT_COMPLETED;
        break;

    case FOCUS_IS_RELEASED:
        if (!strcmp(reason_for_change, "media"))              e = EInterruptCode::INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "radio"))              e = EInterruptCode::INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "loopback"))           e = EInterruptCode::INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "system"))             e = EInterruptCode::INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "alarm"))              e = EInterruptCode::INTERRUPT_BY_ALARM;
        if (!strcmp(reason_for_change, "notification"))       e = EInterruptCode::INTERRUPT_BY_NOTIFICATION;
        if (!strcmp(reason_for_change, "emergency"))          e = EInterruptCode::INTERRUPT_BY_EMERGENCY;
        if (!strcmp(reason_for_change, "voice-information"))  e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "voice-recognition"))  e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "ringtone-voip"))      e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "ringtone-call"))      e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "voip"))               e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "call-voice"))         e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "call-video"))         e = EInterruptCode::INTERRUPT_BY_MEDIA;  //for what?
        break;
    }

    return e;
}
