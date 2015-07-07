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


#include <mm_error.h>
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioSessionHandler
 */
int CAudioSessionHandler::sCaptureRef = 0;

int CAudioSessionHandler::PCM_CAPTURE_COUNT_INC() {
    int actual;
    do {
        actual = sCaptureRef;
    } while (!__sync_bool_compare_and_swap(&sCaptureRef, actual, actual + 1));
    AUDIO_IO_LOGD("CaptureRefCount+1 > [%d]", sCaptureRef);
    return sCaptureRef;
}

int CAudioSessionHandler::PCM_CAPTURE_COUNT_DEC() {
    int actual;
    do {
        actual = sCaptureRef;
    } while (!__sync_bool_compare_and_swap(&sCaptureRef, actual, actual - 1));
    AUDIO_IO_LOGD("CaptureRefCount-1 > [%d]", sCaptureRef);
    if (sCaptureRef < 0) {
        AUDIO_IO_LOGE("A CaptureRef[%d] is not valid! Something is wrong!", sCaptureRef);
        sCaptureRef = 0;
    }
    return sCaptureRef;
}

int CAudioSessionHandler::PCM_CAPTURE_COUNT_GET() {
    AUDIO_IO_LOGD("CaptureRefCount > [%d]", sCaptureRef);
    return sCaptureRef;
}

int CAudioSessionHandler::sFocusRef = 0;

int CAudioSessionHandler::FOCUS_ID_COUNT_INC() {
    int actual;
    do {
        actual = sFocusRef;
    } while (!__sync_bool_compare_and_swap(&sFocusRef, actual, actual + 1));
    AUDIO_IO_LOGD("FocusRefCount+1 > [%d]", sFocusRef);
    return sFocusRef;
}

int CAudioSessionHandler::FOCUS_ID_COUNT_DEC() {
    int actual;
    do {
        actual = sFocusRef;
    } while (!__sync_bool_compare_and_swap(&sFocusRef, actual, actual - 1));
    AUDIO_IO_LOGD("FocusRefCount-1 > [%d]", sFocusRef);
    return sFocusRef;
}

int CAudioSessionHandler::FOCUS_ID_COUNT_GET() {
    /* AUDIO_IO_LOGD("FocusRefCount > [%d]", sFocusRef); */
    return sFocusRef;
}

CAudioSessionHandler::CAudioSessionHandler(EAudioSessionType sessionType, CAudioInfo& audioInfo, IAudioSessionEventListener* listener)
: mId(-1),
  mOptions(0),
  mAudioSession(sessionType),
  mMultimediaSession(MM_SESSION_TYPE_MEDIA),
  mpEventListener(listener),
  mIsInit(false),
  mUseFocus(false),
  mSubscribeId(-1) {
    mAudioInfo = audioInfo;
}

CAudioSessionHandler::~CAudioSessionHandler() {
}

CAudioError CAudioSessionHandler::_convertStreamType(EAudioSessionType type1, MMSessionType type2, int *index) {
    unsigned int i;
    int idx = -1;

    assert(index != NULL);

    if (type1 == AUDIO_SESSION_TYPE_CAPTURE) {
        for (i = 0 ; i < sizeof(stream_type_table_in) / sizeof(stream_type_table_in[0]) ; i++) {
            if (stream_type_table_in[i].type == type2) {
                idx = i;
                break;
            }
        }
    } else {
        for (i = 0 ; i < sizeof(stream_type_table_out) / sizeof(stream_type_table_out[0]) ; i++) {
            if (stream_type_table_out[i].type == type2) {
                idx = i;
                break;
            }
        }
    }

    if (idx < 0) {
        RET_ERROR_MSG(CAudioError::ERROR_NOT_SUPPORTED, "Does not support session type.");
    }
    *index = idx;
    RET_ERROR(CAudioError::ERROR_NONE);
}

CAudioError CAudioSessionHandler::_getAsmInformation(MMSessionType *type, int *options) {
    assert(type != NULL);
    assert(options != NULL);

    MMSessionType currentSession = MM_SESSION_TYPE_MEDIA;
    int           sessionOptions = 0;

    /* Read session information */
    int ret = 0;
    if ((ret = _mm_session_util_read_information(-1, (int*)&currentSession, &sessionOptions)) < 0) {
        if(ret == (int) MM_ERROR_INVALID_HANDLE) {
            RET_ERROR_MSG(CAudioError::ERROR_INVALID_HANDLE, "Failed _mm_session_util_read_information(). Invalid handle");
        } else {
            RET_ERROR_MSG(CAudioError::ERROR_FAILED_OPERATION, "Failed _mm_session_util_read_information(). Not exist");
        }
    }

    *type    = currentSession;
    *options = sessionOptions;

    RET_ERROR(CAudioError::ERROR_NONE);
}

bool CAudioSessionHandler::_isFocusRequired(MMSessionType type, int options) {
    if((options & ASM_SESSION_OPTION_PAUSE_OTHERS)
        || ((type != MM_SESSION_TYPE_MEDIA) && (type != MM_SESSION_TYPE_MEDIA_RECORD)))
        return true;
    else
        return false;
}

int CAudioSessionHandler::getId() {
    return mId;
}

int CAudioSessionHandler::getOptions() {
    return mOptions;
}

CAudioSessionHandler::EAudioSessionType CAudioSessionHandler::getAudioSession() {
    return mAudioSession;
}

MMSessionType CAudioSessionHandler::getMultimediaSession() {
    return mMultimediaSession;
}

int CAudioSessionHandler::getSubscribeId() {
    return mSubscribeId;
}

CAudioInfo CAudioSessionHandler::getAudioInfo() {
    return mAudioInfo;
}

void CAudioSessionHandler::_sound_pcm_signal_cb(mm_sound_signal_name_t signal, int value, void *user_data) {
    assert(user_data);

    AUDIO_IO_LOGD("[signal:%d], [value:%d], [user_data:0x%x]", signal, value, user_data);

    CAudioSessionHandler* pHandler = static_cast<CAudioSessionHandler*>(user_data);
    if (pHandler->mpEventListener != NULL) {
        pHandler->mpEventListener->onSignal(pHandler, signal, value);
    }
}

void CAudioSessionHandler::initialize() throw (CAudioError) {
    AUDIO_IO_LOGD("");
    if (mIsInit == true) {
        return;
    }

    MMSessionType currentSession = MM_SESSION_TYPE_MEDIA;
    int           sessionOptions = 0;  // Mix with others by default

    CAudioError err = _getAsmInformation(&currentSession, &sessionOptions);
    if (err == CAudioError::ERROR_NONE) {
        // Session was configured before, use focus callback
        mUseFocus = true;
        AUDIO_IO_LOGD("Use audio focus concept internally!");
    } else {
        if (err == CAudioError::ERROR_INVALID_HANDLE) {
            int value = 0;
            unsigned int subscribe_id;

            int errorCode = mm_sound_get_signal_value(MM_SOUND_SIGNAL_RELEASE_INTERNAL_FOCUS, &value);
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_get_signal_value() err:0x%x", errorCode);
            }

            if (value == 1) {
                // stream_info was created or focus watch callback was configured before
                mUseFocus = false;
                AUDIO_IO_LOGD("Skip audio focus concept!");
            } else if (value == 0) {
                // No session, No stream_info, No focus watch callback before
                // Use focus watch callback with signal subscribe
                errorCode = mm_sound_subscribe_signal(MM_SOUND_SIGNAL_RELEASE_INTERNAL_FOCUS, &subscribe_id, _sound_pcm_signal_cb, static_cast<void*>(this));
                if (errorCode != MM_ERROR_NONE) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_get_signal_value() err:0x%x", errorCode);
                }

                mSubscribeId = (int)subscribe_id;
                AUDIO_IO_LOGD("Subscribed mm_sound signal");

                sessionOptions = 0;  // Mix with others by default
                mUseFocus = true;
                AUDIO_IO_LOGD("Use audio focus(watch) concept internally!");
            }
        } else {
            mUseFocus = false;
            AUDIO_IO_LOGD("Skip audio focus concept!");
        }

        if (mAudioSession == AUDIO_SESSION_TYPE_CAPTURE) {
            AUDIO_IO_LOGD("Set default \"Media_Record\" type");
            currentSession = MM_SESSION_TYPE_MEDIA_RECORD;
        } else {
            AUDIO_IO_LOGD("Set default \"Media\" type");
            currentSession = MM_SESSION_TYPE_MEDIA;
        }
    }

    // Updates session information
    mMultimediaSession = currentSession;
    mOptions           = sessionOptions;

    if (this->mAudioSession == AUDIO_SESSION_TYPE_CAPTURE) {
        PCM_CAPTURE_COUNT_INC();
    }

    mIsInit = true;
}

void CAudioSessionHandler::finalize() {
    AUDIO_IO_LOGD("");
    if (mIsInit == false) {
        return;
    }

    if (mAudioSession == AUDIO_SESSION_TYPE_CAPTURE) {
        PCM_CAPTURE_COUNT_DEC();
    }

    if (mSubscribeId >= 0) {
        mm_sound_unsubscribe_signal(mSubscribeId);
    }

    mIsInit = false;
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
        switch (mAudioInfo.getAudioType()) {
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
            int captureCount = CAudioSessionHandler::PCM_CAPTURE_COUNT_GET();
            if (captureCount == 1) {/* If this is last one */
                /* Recover session information to MEDIA */
                int sessionResult = _mm_session_util_write_information(-1, MM_SESSION_TYPE_MEDIA, mOptions);
                if (sessionResult != 0) {
                    THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INTERNAL_OPERATION, "Failed _mm_session_util_write_information() ret:%d", sessionResult);
                }
            }
        }
    }
#endif

    return ret;
}

void CAudioSessionHandler::_sound_pcm_focus_cb(int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data) {
    assert(user_data);

    AUDIO_IO_LOGD("[id:%d], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s], [user_data:0x%x]", id, focus_type, state, reason_for_change, additional_info, user_data);

    CAudioSessionHandler* pHandler = static_cast<CAudioSessionHandler*>(user_data);
    pHandler->mFocusType       = focus_type;
    pHandler->mState           = state;
    pHandler->mReasonForChange = (char *)reason_for_change;
    pHandler->mAdditionalInfo  = (char *)additional_info;

    if (pHandler->mpEventListener != NULL) {
        pHandler->mpEventListener->onInterrupt(pHandler, id, focus_type, state, reason_for_change, additional_info);
    }

    return;
}

void CAudioSessionHandler::_sound_pcm_focus_watch_cb(int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data) {
    AUDIO_IO_LOGD("[id:%d], [focus_type:%d], [state:%d], [reason_for_change:%s], [additional_info:%s], [user_data:0x%x]", id, focus_type, state, reason_for_change, additional_info, user_data);

    CAudioSessionHandler::_sound_pcm_focus_cb(-1, focus_type, state, reason_for_change, additional_info, user_data);

    return;
}

void CAudioSessionHandler::registerSound() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (mUseFocus == true) {
        if (mId >= 0) {
            THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Already registered [id:%d]", mId);
        }

        int errorCode = 0;

        if (_isFocusRequired(mMultimediaSession, mOptions)) {
            int index = 0;
            CAudioError err = _convertStreamType(mAudioSession, mMultimediaSession, &index);
            if (err != CAudioError::ERROR_NONE) {
                throw err;
            }

            errorCode = mm_sound_focus_get_id(&mId);
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_focus_get_id() err:0x%x", errorCode);
            }

            // Register focus callback
            errorCode = mm_sound_register_focus(mId,
                                                mAudioSession == AUDIO_SESSION_TYPE_CAPTURE ? stream_type_table_in[index].name : stream_type_table_out[index].name,
                                                _sound_pcm_focus_cb,
                                                static_cast<void*>(this));
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_register_focus() err:0x%x", errorCode);
            }

            FOCUS_ID_COUNT_INC();

            AUDIO_IO_LOGD("Focus callback registered successfully [id:%d]", mId);
        } else if (!(mOptions & ASM_SESSION_OPTION_UNINTERRUPTIBLE)) {
            // Register focus watch callback
            errorCode = mm_sound_set_focus_watch_callback(FOCUS_FOR_BOTH, _sound_pcm_focus_watch_cb, static_cast<void*>(this), &mId);
            if (errorCode < 0) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_set_focus_watch_callback() err:0x%x", errorCode);
            }

            FOCUS_ID_COUNT_INC();

            AUDIO_IO_LOGD("Focus watch callback registered successfully [id:%d]", mId);
        }
    }
}

void CAudioSessionHandler::unregisterSound() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (mUseFocus == true) {
        if (mId < 0) {
            THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Did not register [id:%d]", mId);
        }

        int errorCode = 0;

        if (_isFocusRequired(mMultimediaSession, mOptions)) {
            // Unregister focus callback
            errorCode = mm_sound_unregister_focus(mId);
            if (errorCode != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_unregister_focus() err:0x%x", errorCode);
            }

            FOCUS_ID_COUNT_DEC();

            AUDIO_IO_LOGD("Focus callback unregistered successfully [id:%d]", mId);
            mId = -1;
        } else if (!(mOptions & ASM_SESSION_OPTION_UNINTERRUPTIBLE)) {
            // Unregister focus watch callback.
            errorCode = mm_sound_unset_focus_watch_callback(mId);
            if (errorCode < 0) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_unset_focus_watch_callback() err:0x%x", errorCode);
            }

            FOCUS_ID_COUNT_DEC();

            AUDIO_IO_LOGD("Focus watch callback unregistered successfully [id:%d]", mId);
            mId = -1;
        }
    }
}

void CAudioSessionHandler::updatePlaying() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (mUseFocus && _isFocusRequired(mMultimediaSession, mOptions)) {
        if (mId >= 0) {
            int ret = mm_sound_acquire_focus(mId, FOCUS_FOR_BOTH, "audio-io acquire focus");
            if (ret != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_acquire_focus() err:0x%x", ret);
            }
            AUDIO_IO_LOGD("Focus acquired successfully [id:%d]", mId);
        }
    }
}

void CAudioSessionHandler::updateStop() throw (CAudioError) {
    if (mIsInit == false) {
        THROW_ERROR_MSG(CAudioError::ERROR_NOT_INITIALIZED, "Doesn't initialize CAudioSessionHandler");
    }

    if (mUseFocus && _isFocusRequired(mMultimediaSession, mOptions)) {
        if (mId >= 0) {
            int ret = mm_sound_release_focus(mId, FOCUS_FOR_BOTH, "audio-io release focus");
            if (ret != MM_ERROR_NONE) {
                THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_POLICY_BLOCKED, "Failed mm_sound_release_focus() err:0x%x", ret);
            }
            AUDIO_IO_LOGD("Focus released successfully [id:%d]", mId);
        }
    }
}

void CAudioSessionHandler::disableSessionHandler() throw (CAudioError) {
    CAudioSessionHandler::updateStop();
    CAudioSessionHandler::unregisterSound();

    CAudioSessionHandler::mUseFocus = false;
}

/**
 * class IAudioSessionEventListener
 */
IAudioSessionEventListener::EInterruptCode IAudioSessionEventListener::convertInterruptedCode(int code, const char *reason_for_change) {
    EInterruptCode e = INTERRUPT_COMPLETED;

    switch (code)
    {
    case FOCUS_IS_ACQUIRED:
        e = INTERRUPT_COMPLETED;
        break;

    case FOCUS_IS_RELEASED:
        if (!strcmp(reason_for_change, "media"))              e = INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "radio"))              e = INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "loopback"))           e = INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "system"))             e = INTERRUPT_BY_MEDIA;
        if (!strcmp(reason_for_change, "alarm"))              e = INTERRUPT_BY_ALARM;
        if (!strcmp(reason_for_change, "notification"))       e = INTERRUPT_BY_NOTIFICATION;
        if (!strcmp(reason_for_change, "emergency"))          e = INTERRUPT_BY_EMERGENCY;
        if (!strcmp(reason_for_change, "voice-information"))  e = INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "voice-recognition"))  e = INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "ringtone-voip"))      e = INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "ringtone-call"))      e = INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "voip"))               e = INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "call-voice"))         e = INTERRUPT_BY_MEDIA;  //for what?
        if (!strcmp(reason_for_change, "call-video"))         e = INTERRUPT_BY_MEDIA;  //for what?
        break;
    }

    return e;
}
