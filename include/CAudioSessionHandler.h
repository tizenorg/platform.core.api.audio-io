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

#ifndef __TIZEN_MEDIA_AUDIOIO_IO_CAUDIO_SESSION_HANDLER_H__
#define __TIZEN_MEDIA_AUDIOIO_IO_CAUDIO_SESSION_HANDLER_H__


#ifdef __cplusplus


#include <mm_session.h>
#include <mm_session_private.h>
#include <mm_sound.h>
#include <mm_sound_focus.h>

namespace tizen_media_audio {


    /**
     * ASM Thread
     */
    class CAudioSessionHandler {
    public:
        enum class EAudioSessionType : unsigned int {
            AUDIO_SESSION_TYPE_CAPTURE,
            AUDIO_SESSION_TYPE_PLAYBACK
        };

        /* Constructor & Destructor */
        CAudioSessionHandler(EAudioSessionType sessionType, CAudioInfo& audioInfo, IAudioSessionEventListener* listener);
        virtual ~CAudioSessionHandler();

        /* Methods */
        virtual void initialize() throw(CAudioError);
        virtual void finalize();

        bool isSkipSessionEvent() throw(CAudioError);

        void registerSound() throw(CAudioError);
        void unregisterSound() throw(CAudioError);

        void updatePlaying() throw(CAudioError);
        void updateStop() throw(CAudioError);
        void disableSessionHandler() throw(CAudioError);

        /* Setter & Getter */
        int getId();
        int getOptions();
        EAudioSessionType  getAudioSession();
        MMSessionType      getMultimediaSession();
        int getSubscribeId();
        CAudioInfo getAudioInfo();

    private:
        struct __streamTypeTable {
            const char* name;
            MMSessionType type;
        };
        const struct __streamTypeTable __STREAM_TYPE_TABLE_IN[MM_SESSION_TYPE_NUM] = {
            {"media",        MM_SESSION_TYPE_MEDIA},
            {"media",        MM_SESSION_TYPE_MEDIA_RECORD},
            {"media",        MM_SESSION_TYPE_ALARM},
            {"media",        MM_SESSION_TYPE_NOTIFY},
            {"media",        MM_SESSION_TYPE_EMERGENCY},
            {"media",        MM_SESSION_TYPE_CALL},
            {"media",        MM_SESSION_TYPE_VIDEOCALL},
            {"voip",         MM_SESSION_TYPE_VOIP},
            {"media",        MM_SESSION_TYPE_VOICE_RECOGNITION},
            {"media",        MM_SESSION_TYPE_RECORD_AUDIO},
            {"media",        MM_SESSION_TYPE_RECORD_VIDEO}
        };
        const struct __streamTypeTable __STREAM_TYPE_TABLE_OUT[MM_SESSION_TYPE_NUM] = {
            {"media",        MM_SESSION_TYPE_MEDIA},
            {"media",        MM_SESSION_TYPE_MEDIA_RECORD},
            {"alarm",        MM_SESSION_TYPE_ALARM},
            {"notification", MM_SESSION_TYPE_NOTIFY},
            {"emergency",    MM_SESSION_TYPE_EMERGENCY},
            {"media",        MM_SESSION_TYPE_CALL},
            {"media",        MM_SESSION_TYPE_VIDEOCALL},
            {"voip",         MM_SESSION_TYPE_VOIP},
            {"media",        MM_SESSION_TYPE_VOICE_RECOGNITION},
            {"media",        MM_SESSION_TYPE_RECORD_AUDIO},
            {"media",        MM_SESSION_TYPE_RECORD_VIDEO}
        };

        /* Private Static Methods */
        static int __pcmCaptureCountInc();
        static int __pcmCaptureCountDec();
        static int __pcmCaptureCountGet();
        static int __focusIdCountInc();
        static int __focusIdCountDec();
        static int __focusIdCountGet();

        static void __sound_pcm_signal_cb(mm_sound_signal_name_t signal, int value, void *user_data);
        static void __sound_pcm_focus_cb(int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, int option, const char *additional_info, void *user_data);
        static void __sound_pcm_focus_watch_cb(int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data);

        /* Private Method */
        CAudioError __convertStreamType(EAudioSessionType type1, MMSessionType type2, int *index);
        CAudioError __getAsmInformation(MMSessionType *type, int *options);
        bool __isFocusRequired(MMSessionType type, int options);

        /* Static Member */
        static int                  __sCaptureRef;
        static int                  __sFocusRef;

        /* Members */
        int                         __mId;
        int                         __mOptions;

        EAudioSessionType           __mAudioSession;
        MMSessionType               __mMultimediaSession;

        CAudioInfo                  __mAudioInfo;        /* Referenced from CAudioIO */

        IAudioSessionEventListener* __mpEventListener;

        bool                        __mIsInit;
        int                         __mSubscribeId;

        bool                        __mUseFocus;
        mm_sound_focus_type_e       __mFocusType;        /* For audio focus */
        mm_sound_focus_state_e      __mState;            /* For audio focus */
        char*                       __mReasonForChange;  /* For audio focus */
        char*                       __mAdditionalInfo;   /* For audio focus */
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIOIO_IO_CAUDIO_SESSION_HANDLER_H__ */
