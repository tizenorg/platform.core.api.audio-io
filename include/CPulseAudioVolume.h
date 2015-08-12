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


#if 0
#ifndef __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_VOLUME_H__
#define __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_VOLUME_H__


#ifdef __cplusplus


namespace tizen_media_audio {


    /**
     * Audio Volume
     */
    class CPulseAudioVolume {
    public:
        /* Enums */
        enum EVolume {
            VOLUME_SYSTEM,                         /**< System volume type */
            VOLUME_NOTIFICATION,                   /**< Notification volume type */
            VOLUME_ALARM,                          /**< Alarm volume type */
            VOLUME_RINGTONE,                       /**< Ringtone volume type */
            VOLUME_MEDIA,                          /**< Media volume type */
            VOLUME_CALL,                           /**< Call volume type */
            VOLUME_VOIP,                           /**< VOIP volume type */
            VOLUME_VOICE,                          /**< VOICE volume type */
            VOLUME_FIXED,                          /**< Volume type for fixed acoustic level */
            VOLUME_EXT_ANDROID = VOLUME_FIXED,     /**< External system volume for Android */
            VOLUME_MAX                             /**< Volume type count */
        };

        enum EVolumeGain {
            VOLUME_GAIN_DEFAULT,
            VOLUME_GAIN_DIALER,
            VOLUME_GAIN_TOUCH,
            VOLUME_GAIN_AF,
            VOLUME_GAIN_SHUTTER1,
            VOLUME_GAIN_SHUTTER2,
            VOLUME_GAIN_CAMCORDING,
            VOLUME_GAIN_MIDI,
            VOLUME_GAIN_BOOTING,
            VOLUME_GAIN_VIDEO,
            VOLUME_GAIN_TTS,
            VOLUME_GAIN_MAX
        };

        /* Constructor & Destructor */
        CPulseAudioVolume();
        CPulseAudioVolume(EVolume volume, EVolumeGain gain);
        ~CPulseAudioVolume();

        /* Methods */

        /* Setter & Getter */
        void setVolume(EVolume volume);
        EVolume getVolume();

        void setVolumeGain(EVolumeGain volumeGain);
        EVolumeGain getVolumeGain();

    private:
        /* Members */
        EVolume     __mVolume;
        EVolumeGain __mVolumeGain;
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_VOLUME_H__ */
#endif
