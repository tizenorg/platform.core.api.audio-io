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

#ifndef __TIZEN_MEDIA_AUDIO_IO_CPULSESTREAM_SPEC_H__
#define __TIZEN_MEDIA_AUDIO_IO_CPULSESTREAM_SPEC_H__


#ifdef __cplusplus


#include <pulse/pulseaudio.h>


namespace tizen_media_audio {


    /**
     * class CPulseStreamSpec
     */
    class CPulseStreamSpec {
    public:
        /* Enums */
        enum class EStreamLatency : unsigned int {
            STREAM_LATENCY_INPUT_LOW,
            STREAM_LATENCY_INPUT_MID,
            STREAM_LATENCY_INPUT_HIGH,
            STREAM_LATENCY_INPUT_VOIP,
            STREAM_LATENCY_OUTPUT_LOW,
            STREAM_LATENCY_OUTPUT_MID,
            STREAM_LATENCY_OUTPUT_HIGH,
            STREAM_LATENCY_OUTPUT_VOIP,
            STREAM_LATENCY_MAX
        };

        /* Constructor & Destructor */
        CPulseStreamSpec() throw(CAudioError);
        CPulseStreamSpec(EStreamLatency latency, CAudioInfo& audioInfo) throw(CAudioError);
        CPulseStreamSpec(EStreamLatency latency, CAudioInfo& audioInfo, int customLatency) throw(CAudioError);
        ~CPulseStreamSpec();

        /* Setter & Getter */
        EStreamLatency getStreamLatency();
        const char*    getStreamLatencyToString();
        CAudioInfo&    getAudioInfo();
        pa_sample_spec getSampleSpec();
        pa_channel_map getChannelMap();
        const char*    getStreamName();

    private:
        /* Private Methods */
        void __adjustSpec() throw(CAudioError);

        /* Members */
        EStreamLatency __mLatency;
        CAudioInfo     __mAudioInfo;
        pa_sample_spec __mSampleSpec;
        pa_channel_map __mChannelMap;
        const char*    __mStreamName;
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_CPULSESTREAM_SPEC_H__ */
