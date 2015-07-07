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

#ifndef __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_CLIENT_H__
#define __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_CLIENT_H__


#ifdef __cplusplus


#include <pulse/pulseaudio.h>
#include <pulse/ext-policy.h>


namespace tizen_media_audio {


    /**
     * PULSE Thread
     */
    class CPulseAudioVolume;
    class CPulseAudioPolicy;
    class CPulseStreamSpec;
    class CPulseAudioClient {
    public:
        /* Constants */
        static const char* CLIENT_NAME;

        enum EStreamDirection {
            STREAM_DIRECTION_RECORD,        /**< Record stream */
            STREAM_DIRECTION_PLAYBACK       /**< Playback stream */
        };

    private:
        /* Members */
        EStreamDirection      mDirection;
        CPulseStreamSpec      mSpec;
        IPulseStreamListener* mpListener;

        pa_threaded_mainloop* mpMainloop;
        pa_context*           mpContext;
        pa_stream*            mpStream;
        pa_proplist*          mpPropList;

        bool                  mIsInit;
        bool                  mIsOperationSuccess;

        /* Static Methods */

        /* Private Method */

        /* Private Calblack Method */
        static void _contextStateChangeCb(pa_context* c, void* user_data);
        static void _successContextCb(pa_context* c, int success, void* user_data);

        static void _streamStateChangeCb(pa_stream* s, void* user_data);
        static void _streamCaptureCb(pa_stream* s, size_t length, void* user_data);
        static void _streamPlaybackCb(pa_stream* s, size_t length, void* user_data);
        static void _streamLatencyUpdateCb(pa_stream* s, void* user_data);
        static void _successStreamCb(pa_stream* s, int success, void* user_data);

    public:
        /* Constructor & Destructor */
        CPulseAudioClient(EStreamDirection      direction,
                          CPulseStreamSpec&     spec,
                          IPulseStreamListener* listener);
        ~CPulseAudioClient();

        /* Implemented Methods */
        void initialize() throw (CAudioError);
        void finalize();

        /* Methods */
        int  peek(const void** data, size_t* length) throw (CAudioError);
        int  drop() throw (CAudioError);
        int  write(const void* data, size_t length) throw (CAudioError);

        void cork(bool cork) throw (CAudioError);
        bool isCorked() throw (CAudioError);

        bool drain() throw (CAudioError);
        bool flush() throw (CAudioError);

        void checkRunningState() throw (CAudioError);
        bool isInThread() throw (CAudioError);

        size_t getWritableSize() throw (CAudioError);
        size_t getReadableSize() throw (CAudioError);

        size_t getBufferSize() throw (CAudioError);

        pa_usec_t getLatency() throw (CAudioError);
        pa_usec_t getFinalLatency() throw (CAudioError);

        /* Setter & Getter */
        EStreamDirection getStreamDirection();
        CPulseStreamSpec getStreamSpec();
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_CPULSEAUDIO_CLIENT_H__ */
