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

#ifndef __TIZEN_MEDIA_AUDIO_IO_CAUDIO_IO_H__
#define __TIZEN_MEDIA_AUDIO_IO_CAUDIO_IO_H__


#ifdef __cplusplus


namespace tizen_media_audio {


    /**
     *  Abstract Class
     *  You can't take this object directly.
     */
    class IPulseStreamListener;
    class IAudioSessionEventListener;
    class CAudioIO : public IPulseStreamListener, public IAudioSessionEventListener {
    public:
        struct SStreamCallback {
            void* mUserData;
            void (*onStream)(size_t nbytes, void* user_data);

            SStreamCallback() : mUserData(NULL), onStream(NULL)
            {/* Empty Body */}
        };

        struct SStateChangedCallback {
            void* mUserData;
            void (*onStateChanged)(CAudioInfo::EAudioIOState state, CAudioInfo::EAudioIOState statePrev, bool byPolicy, void* user_data);

            SStateChangedCallback() : mUserData(NULL), onStateChanged(NULL)
            {/* Empty Body */}
        };

        struct SInterruptCallback {
            void* mUserData;
            void (*onInterrupt)(IAudioSessionEventListener::EInterruptCode code, void* user_data);

            SInterruptCallback() : mUserData(NULL), onInterrupt(NULL)
            {/* Empty Body */}
        };

    private:
        pthread_mutex_t       mMutex;
        pthread_cond_t        mCond;
        bool                  mIsInit;
        bool                  mForceIgnore;

    protected:
        CAudioSessionHandler* mpAudioSessionHandler;
        CPulseAudioClient*    mpPulseAudioClient;
        CAudioInfo            mAudioInfo;

        SStreamCallback       mStreamCallback;
        SStateChangedCallback mStateChangedCallback;
        SInterruptCallback    mInterruptCallback;

        CAudioInfo::EAudioIOState mState;
        CAudioInfo::EAudioIOState mStatePrev;
        bool                  mByPolicy;

        /* Protected Methods */
        virtual void setInit(bool flag);
        virtual bool isInit();
        virtual bool IsReady();

        void internalLock()   throw (CAudioError);
        void internalUnlock() throw (CAudioError);
        void internalWait()   throw (CAudioError);
        void internalSignal() throw (CAudioError);

        bool isForceIgnore();

    public:
        /* Constructor & Destructor */
        CAudioIO();
        CAudioIO(CAudioInfo& audioInfo);
        virtual ~CAudioIO();

        /* Pure Virtual Methods */
        virtual void initialize() throw (CAudioError) = 0;
        virtual void finalize() = 0;

        virtual void prepare() throw (CAudioError)   = 0;
        virtual void unprepare() throw (CAudioError) = 0;

        virtual void pause() throw (CAudioError) = 0;
        virtual void resume() throw (CAudioError) = 0;

        virtual void drain() throw (CAudioError) = 0;
        virtual void flush() throw (CAudioError) = 0;

        virtual int  getBufferSize() throw (CAudioError) = 0;

        /* Implemented Handlers */
        virtual void onStream(CPulseAudioClient* pClient, size_t length);
        virtual void onStateChanged(CAudioInfo::EAudioIOState state, bool byPolicy);
        virtual void onStateChanged(CAudioInfo::EAudioIOState state);
        virtual void onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info);
        virtual void onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value);

        /* Methods */
        CAudioInfo getAudioInfo() throw (CAudioError);

        virtual void setStreamCallback(SStreamCallback callback) throw (CAudioError);
        SStreamCallback getStreamCallback() throw (CAudioError);

        virtual void setStateChangedCallback(SStateChangedCallback callback) throw (CAudioError);
        SStateChangedCallback getStateChangedCallback() throw (CAudioError);

        void setInterruptCallback(SInterruptCallback callback) throw (CAudioError);
        SInterruptCallback getInterruptCallback() throw (CAudioError);

        void ignoreSession() throw (CAudioError);
    };

} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_CAUDIO_IO_H__ */
