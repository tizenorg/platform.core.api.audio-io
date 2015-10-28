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

#ifndef __TIZEN_MEDIA_AUDIO_IO_CAUDIO_INPUT_H__
#define __TIZEN_MEDIA_AUDIO_IO_CAUDIO_INPUT_H__


#ifdef __cplusplus


namespace tizen_media_audio {


    /**
     * A class CAudioInput that inherited from CAudioIO
     */
    class CAudioInput : public CAudioIO {
    public:
        /* Constructor & Destructor */
        CAudioInput(CAudioInfo& info);
        CAudioInput(
                    unsigned int            sampleRate,
                    CAudioInfo::EChannel    channel,
                    CAudioInfo::ESampleType sampleType,
                    CAudioInfo::EAudioType  audioType);
        ~CAudioInput();

        /* Overridden Handler */
        virtual void onStream(CPulseAudioClient* pClient, size_t length);
        virtual void onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info);
        virtual void onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value);

        /* Implemented Methods */
        virtual void initialize() throw (CAudioError);
        virtual void finalize();

        virtual void prepare() throw (CAudioError);
        virtual void unprepare() throw (CAudioError);

        virtual void pause() throw (CAudioError);
        virtual void resume() throw (CAudioError);

        virtual void drain() throw (CAudioError);
        virtual void flush() throw (CAudioError);

        virtual int  getBufferSize() throw (CAudioError);

        /* Overridden Methods */
        virtual void setStreamCallback(SStreamCallback callback) throw (CAudioError);

        /* Methods */
        size_t read(void* buffer, size_t length) throw (CAudioError);
        int peek(const void** buffer, size_t* length) throw (CAudioError);
        int drop() throw (CAudioError);

    private:
        /* Private Methods */
        void __setInit(bool flag);
        bool __IsInit();
        bool __IsReady();

        bool __mIsUsedSyncRead;
        bool __mIsInit;
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_CAUDIO_INPUT_H__ */
