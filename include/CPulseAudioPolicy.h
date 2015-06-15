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
#ifndef __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_POLICY_H__
#define __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_POLICY_H__


#ifdef __cplusplus


namespace tizen_media_audio {


    /**
     * class CPulseAudioPolicy
     */
    class CPulseAudioPolicy {
    public:
        /* Constants */
        enum EPolicy {
            POLICY_DEFAULT,
            POLICY_OUT_AUTO,
            POLICY_OUT_PHONE,
            POLICY_OUT_VOIP,
            POLICY_OUT_ALL,
            POLICY_IN_VOIP,
            POLICY_IN_LOOPBACK,
            POLICY_IN_MIRRORING,
            POLICY_HIGH_LATENCY,
            POLICY_MAX
        };

    private:
        /* Members */
        EPolicy mPolicy;

    public:
        /* Constructors */
        CPulseAudioPolicy();
        CPulseAudioPolicy(EPolicy policy);
        ~CPulseAudioPolicy();

        /* getter & setter */
        void setPolicy(EPolicy policy) throw (CAudioError);
        EPolicy getPolicy();

        /* Override */
        bool operator != (const EPolicy policy);
        bool operator == (const EPolicy policy);
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_POLICY_H__ */
#endif
