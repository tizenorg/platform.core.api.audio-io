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


#include <stdio.h>
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CAudioInfo
 */
CAudioInfo::CAudioInfo()
    :  mSampleRate(MAX_SYSTEM_SAMPLERATE), mChannel(CHANNEL_MONO),
       mSampleType(SAMPLE_TYPE_U8), mAudioType(AUDIO_IN_TYPE_MEDIA),
       mAudioIndex(-1) {
}

CAudioInfo::CAudioInfo(unsigned int sampleRate, EChannel channel, ESampleType sampleType, EAudioType audioType, int audioIndex) throw (CAudioError)
    :  mSampleRate(sampleRate), mChannel(channel), mSampleType(sampleType), mAudioType(audioType), mAudioIndex(audioIndex) {
    // Check to invalid AudioInfo
    if (sampleRate < CAudioInfo::MIN_SYSTEM_SAMPLERATE || sampleRate > CAudioInfo::MAX_SYSTEM_SAMPLERATE) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "The sampleRate is invalid [sampleRate:%d]", sampleRate);
    }

    if (channel < CAudioInfo::CHANNEL_MONO || channel >= CAudioInfo::CHANNEL_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "The channel is invalid [channel:%d]", channel);
    }

    if (sampleType < CAudioInfo::SAMPLE_TYPE_U8 || sampleType >= CAudioInfo::SAMPLE_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "The sampleType is invalid [sampleType:%d]", sampleType);
    }

    if (audioType < CAudioInfo::AUDIO_IN_TYPE_MEDIA || audioType >= CAudioInfo::AUDIO_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_INVALID_ARGUMENT, "The audioType is invalid [audioType:%d]", audioType);
    }
}

unsigned int CAudioInfo::getSampleRate() {
    return mSampleRate;
}

CAudioInfo::EChannel CAudioInfo::getChannel() {
    return mChannel;
}

CAudioInfo::ESampleType CAudioInfo::getSampleType() {
    return mSampleType;
}

CAudioInfo::EAudioType CAudioInfo::getAudioType() {
    return mAudioType;
}

void CAudioInfo::setAudioType(CAudioInfo::EAudioType AudioType) {
    mAudioType = AudioType;
    return;
}

int CAudioInfo::getAudioIndex() {
    return mAudioIndex;
}

void CAudioInfo::setAudioIndex(int AudioIndex) {
    mAudioIndex = AudioIndex;
    return;
}

void CAudioInfo::convertAudioType2StreamType (CAudioInfo::EAudioType audioType, char **streamType)
{
    if (audioType < CAudioInfo::AUDIO_IN_TYPE_MEDIA || audioType >= CAudioInfo::AUDIO_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_SUPPORTED_TYPE, "The audioType is not supported [audioType:%d]", audioType);
    }
    *streamType = (char *)StreamTypeTable[audioType];
    return;
}

void CAudioInfo::convertInputStreamType2AudioType (char *streamType, CAudioInfo::EAudioType *audioType)
{
    unsigned int i;
    for (i = CAudioInfo::AUDIO_IN_TYPE_MEDIA ; i < CAudioInfo::AUDIO_OUT_TYPE_MEDIA ; i++) {
        if (!strcmp((char *)StreamTypeTable[i], streamType)) {
            break;
        }
    }
    if (i >= CAudioInfo::AUDIO_OUT_TYPE_MEDIA) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_SUPPORTED_TYPE, "The streamType is not supported [streamType:%s]", streamType);
    }
    *audioType = (CAudioInfo::EAudioType)i;
    return;
}

void CAudioInfo::convertOutputStreamType2AudioType (char *streamType, CAudioInfo::EAudioType *audioType)
{
    unsigned int i;
    for (i = CAudioInfo::AUDIO_OUT_TYPE_MEDIA ; i < CAudioInfo::AUDIO_TYPE_MAX ; i++) {
        if (!strcmp((char *)StreamTypeTable[i], streamType)) {
            break;
        }
    }
    if (i >= CAudioInfo::AUDIO_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::ERROR_NOT_SUPPORTED_TYPE, "The streamType is not supported [streamType:%s]", streamType);
    }
    *audioType = (CAudioInfo::EAudioType)i;
    return;
}

