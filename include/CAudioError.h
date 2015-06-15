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

#ifndef __TIZEN_MEDIA_AUDIO_IO_CAUDIOERROR_H__
#define __TIZEN_MEDIA_AUDIO_IO_CAUDIOERROR_H__


#ifdef __cplusplus


namespace tizen_media_audio {


    /**
     * Audio Error
     */
    class CAudioError {
    public:
        /* Constants Definition */
        static const unsigned int MSG_LENGTH = 512;

        enum EError {
            ERROR_NONE,

            ERROR_INVALID_ARGUMENT,
            ERROR_INVALID_HANDLE,
            ERROR_INVALID_SAMPLERATE,
            ERROR_INVALID_CHANNEL,
            ERROR_INVALID_FORMAT,
            ERROR_INVALID_POINTER,
            ERROR_INVALID_OPERATION,

            ERROR_NOT_INITIALIZED,
            ERROR_NOT_SUPPORTED,

            ERROR_PERMISSION_DENIED,

            ERROR_DEVICE_NOT_OPENED,
            ERROR_DEVICE_NOT_CLOSED,

            ERROR_OUT_OF_MEMORY,
            ERROR_INTERNAL_OPERATION,
            ERROR_FAILED_OPERATION,

            ERROR_POLICY_BLOCKED,
            ERROR_POLICY_INTERRUPTED,
            ERROR_POLICY_DUPLICATED,

            ERROR_MAX
        };

    private:
        /* Members */
        static EError mLastError;
        static char   mLastErrorMsg[MSG_LENGTH];

        EError        mError;
        char          mErrorMsg[MSG_LENGTH];

        const char* _convertErrorToString(EError err);

    public:
        /* Constructor & Destructor */
        CAudioError(EError err);
        CAudioError(EError err, const char* fileName, const char* parentFunc, int lineNum);
        CAudioError(EError err, const char* msg, const char* fileName, const char* parentFunc, int lineNum);
        //CAudioError(CAudioError& err);
        ~CAudioError();

        /* Static Methods */
        static EError getLastError();
        static const char* getLastErrorMsg();

        /* Setter & Getter */
        EError getError();
        const char* getErrorMsg();

        /* Overrided Operation */
        CAudioError& operator =  (const EError err);
        CAudioError& operator =  (const CAudioError& err);
        bool         operator != (const EError err);
        bool         operator == (const EError err);
//        friend bool operator == (const CAudioError& src, const EError& err);
    };

} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_CPP_OBJECTS_IO_H__ */
