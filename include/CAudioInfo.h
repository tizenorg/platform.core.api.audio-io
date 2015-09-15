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

#ifndef __TIZEN_MEDIA_AUDIO__IO_CAUDIO_INFO_H__
#define __TIZEN_MEDIA_AUDIO__IO_CAUDIO_INFO_H__


#ifdef __cplusplus


namespace tizen_media_audio {


    /**
     * Audio Information
     */
    class CAudioInfo {
    public:
        enum class EChannel : unsigned int {
            CHANNEL_MONO = 1,               /**< 1 channel, mono */
            CHANNEL_STEREO,                 /**< 2 channel, stereo */
            CHANNEL_MAX
        };

        enum class ESampleType : unsigned int {
            SAMPLE_TYPE_U8 = 1,             /**< Unsigned 8-bit audio samples */
            SAMPLE_TYPE_S16_LE,             /**< Signed 16-bit audio samples */
            SAMPLE_TYPE_MAX
        };

        enum class EAudioType : unsigned int {
            /* Input Type */
            AUDIO_IN_TYPE_MEDIA = 0,
            //AUDIO_IN_TYPE_SYSTEM,
            //AUDIO_IN_TYPE_ALARM,
            //AUDIO_IN_TYPE_NOTIFICATION,
            //AUDIO_IN_TYPE_EMERGENCY,
            //AUDIO_IN_TYPE_VOICE_INFORMATION,
            AUDIO_IN_TYPE_VOICE_RECOGNITION,
            //AUDIO_IN_TYPE_RINGTONE_VOIP,
            AUDIO_IN_TYPE_VOIP,
            //AUDIO_IN_TYPE_RINGTONE_CALL,
            //AUDIO_IN_TYPE_VOICE_CALL,
            //AUDIO_IN_TYPE_VIDEO_CALL,
            //AUDIO_IN_TYPE_RADIO,
            AUDIO_IN_TYPE_LOOPBACK,

            /* Output Type */
            AUDIO_OUT_TYPE_MEDIA,
            AUDIO_OUT_TYPE_SYSTEM,
            AUDIO_OUT_TYPE_ALARM,
            AUDIO_OUT_TYPE_NOTIFICATION,
            AUDIO_OUT_TYPE_EMERGENCY,
            AUDIO_OUT_TYPE_VOICE_INFORMATION,
            //AUDIO_OUT_TYPE_VOICE_RECOGNITION,
            AUDIO_OUT_TYPE_RINGTONE_VOIP,
            AUDIO_OUT_TYPE_VOIP,
            AUDIO_OUT_TYPE_RINGTONE_CALL,
            //AUDIO_OUT_TYPE_VOICE_CALL,
            //AUDIO_OUT_TYPE_VIDEO_CALL,
            //AUDIO_OUT_TYPE_RADIO,
            //AUDIO_OUT_TYPE_LOOPBACK,

            AUDIO_TYPE_MAX
        };

        enum class EAudioIOState : unsigned int {
            AUDIO_IO_STATE_NONE,      /**< Audio-io handle is not created */
            AUDIO_IO_STATE_IDLE,      /**< Audio-io handle is created, but not prepared */
            AUDIO_IO_STATE_RUNNING,   /**< Audio-io handle is ready and the stream is running */
            AUDIO_IO_STATE_PAUSED,    /**< Audio-io handle is ready and the stream is paused */
            AUDIO_IO_STATE_MAX
        };

        const static unsigned int MIN_SYSTEM_SAMPLERATE = 8000;
        const static unsigned int MAX_SYSTEM_SAMPLERATE = 48000;

        /* Constructors */
        CAudioInfo();
        CAudioInfo(unsigned int sampleRate, EChannel channel, ESampleType sampleType, EAudioType audioType, int audioIndex) throw (CAudioError);

        /* Setter & Getter */
        unsigned int getSampleRate();
        EChannel getChannel();
        ESampleType getSampleType();
        EAudioType getAudioType();
        void setAudioType(EAudioType audioType);
        int getAudioIndex();
        void setAudioIndex(int audioIndex);
        void convertAudioType2StreamType (CAudioInfo::EAudioType audioType, char **streamType);
        void convertInputStreamType2AudioType (char *streamType, CAudioInfo::EAudioType *audioType);
        void convertOutputStreamType2AudioType (char *streamType, CAudioInfo::EAudioType *audioType);

    private:
        const char *__STREAM_TYPE_TABLE[(unsigned int)EAudioType::AUDIO_TYPE_MAX] = {
            /* Input Type */
            "media",                  /**< AUDIO_IN_TYPE_MEDIA */
            //"system",                 /**< AUDIO_IN_TYPE_SYSTEM */
            //"alarm",                  /**< AUDIO_IN_TYPE_ALARM */
            //"notification",           /**< AUDIO_IN_TYPE_NOTIFICATION */
            //"emergency",              /**< AUDIO_IN_TYPE_EMERGENCY */
            //"voice-information",      /**< AUDIO_IN_TYPE_VOICE_INFORMATION */
            "voice-recognition",      /**< AUDIO_IN_TYPE_VOICE_RECOGNITION */
            //"ringtone-voip",          /**< AUDIO_IN_TYPE_RINGTONE_VOIP */
            "voip",                   /**< AUDIO_IN_TYPE_VOIP */
            //"ringtone-call",          /**< AUDIO_IN_TYPE_RINGTONE_CALL */
            //"call-voice",             /**< AUDIO_IN_TYPE_VOICE_CALL */
            //"call-video",             /**< AUDIO_IN_TYPE_VIDEO_CALL */
            //"radio",                  /**< AUDIO_IN_TYPE_RADIO */
            "loopback",               /**< AUDIO_IN_TYPE_LOOPBACK */

            /* Output Type */
            "media",                  /**< AUDIO_OUT_TYPE_MEDIA */
            "system",                 /**< AUDIO_OUT_TYPE_SYSTEM */
            "alarm",                  /**< AUDIO_OUT_TYPE_ALARM */
            "notification",           /**< AUDIO_OUT_TYPE_NOTIFICATION */
            "emergency",              /**< AUDIO_OUT_TYPE_EMERGENCY */
            "voice-information",      /**< AUDIO_OUT_TYPE_VOICE_INFORMATION */
            //"voice-recognition",      /**< AUDIO_OUT_TYPE_VOICE_RECOGNITION */
            "ringtone-voip",          /**< AUDIO_OUT_TYPE_RINGTONE_VOIP */
            "voip",                   /**< AUDIO_OUT_TYPE_VOIP */
            "ringtone-call",          /**< AUDIO_OUT_TYPE_RINGTONE_CALL */
            //"call-voice",             /**< AUDIO_OUT_TYPE_VOICE_CALL */
            //"call-video",             /**< AUDIO_OUT_TYPE_VIDEO_CALL */
            //"radio",                  /**< AUDIO_OUT_TYPE_RADIO */
            //"loopback",               /**< AUDIO_OUT_TYPE_LOOPBACK */
        };

        unsigned int __mSampleRate;
        EChannel     __mChannel;
        ESampleType  __mSampleType;
        EAudioType   __mAudioType;
        int          __mAudioIndex;
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO__IO_CAUDIO_INFO_H__ */
