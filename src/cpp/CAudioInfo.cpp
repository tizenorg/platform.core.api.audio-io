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
CAudioInfo::CAudioInfo() :
    __mSampleRate(MAX_SYSTEM_SAMPLERATE),
    __mChannel(EChannel::CHANNEL_MONO),
    __mSampleType(ESampleType::SAMPLE_TYPE_U8),
    __mAudioType(EAudioType::AUDIO_IN_TYPE_MEDIA),
    __mAudioIndex(-1) {
}

CAudioInfo::CAudioInfo(unsigned int sampleRate, EChannel channel, ESampleType sampleType, EAudioType audioType, int audioIndex) throw (CAudioError) :
    __mSampleRate(sampleRate),
    __mChannel(channel),
    __mSampleType(sampleType),
    __mAudioType(audioType),
    __mAudioIndex(audioIndex) {
    // Check to invalid AudioInfo
    if (sampleRate < CAudioInfo::MIN_SYSTEM_SAMPLERATE || sampleRate > CAudioInfo::MAX_SYSTEM_SAMPLERATE) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The sampleRate is invalid [sampleRate:%d]", sampleRate);
    }

    if (channel < CAudioInfo::EChannel::CHANNEL_MONO || channel >= CAudioInfo::EChannel::CHANNEL_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The channel is invalid [channel:%d]", channel);
    }

    if (sampleType < CAudioInfo::ESampleType::SAMPLE_TYPE_U8 || sampleType >= CAudioInfo::ESampleType::SAMPLE_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The sampleType is invalid [sampleType:%d]", sampleType);
    }

    if (audioType < CAudioInfo::EAudioType::AUDIO_IN_TYPE_MEDIA || audioType >= CAudioInfo::EAudioType::AUDIO_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_INVALID_ARGUMENT, "The audioType is invalid [audioType:%d]", audioType);
    }
}

unsigned int CAudioInfo::getSampleRate() {
    return __mSampleRate;
}

CAudioInfo::EChannel CAudioInfo::getChannel() {
    return __mChannel;
}

CAudioInfo::ESampleType CAudioInfo::getSampleType() {
    return __mSampleType;
}

CAudioInfo::EAudioType CAudioInfo::getAudioType() {
    return __mAudioType;
}

void CAudioInfo::setAudioType(CAudioInfo::EAudioType audioType) {
    __mAudioType = audioType;
    return;
}

int CAudioInfo::getAudioIndex() {
    return __mAudioIndex;
}

void CAudioInfo::setAudioIndex(int audioIndex) {
    __mAudioIndex = audioIndex;
    return;
}

void CAudioInfo::convertAudioType2StreamType (CAudioInfo::EAudioType audioType, char **streamType) {
    if (audioType < CAudioInfo::EAudioType::AUDIO_IN_TYPE_MEDIA || audioType >= CAudioInfo::EAudioType::AUDIO_TYPE_MAX) {
        THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_SUPPORTED_TYPE, "The audioType is not supported [audioType:%d]", audioType);
    }
    *streamType = (char *)__STREAM_TYPE_TABLE[(unsigned int)audioType];
    return;
}

void CAudioInfo::convertInputStreamType2AudioType (char *streamType, CAudioInfo::EAudioType *audioType) {
    for (unsigned int i = (unsigned int)CAudioInfo::EAudioType::AUDIO_IN_TYPE_MEDIA ; i < (unsigned int)CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA ; i++) {
        if (!strcmp((char *)__STREAM_TYPE_TABLE[i], streamType)) {
            *audioType = (CAudioInfo::EAudioType)i;
            return;
        }
    }
    THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_SUPPORTED_TYPE, "The streamType of input is not supported [streamType:%s]", streamType);
    return;
}

void CAudioInfo::convertOutputStreamType2AudioType (char *streamType, CAudioInfo::EAudioType *audioType) {
    for (unsigned int i = (unsigned int)CAudioInfo::EAudioType::AUDIO_OUT_TYPE_MEDIA ; i < (unsigned int)CAudioInfo::EAudioType::AUDIO_TYPE_MAX ; i++) {
        if (!strcmp((char *)__STREAM_TYPE_TABLE[i], streamType)) {
            *audioType = (CAudioInfo::EAudioType)i;
            return;
        }
    }
    THROW_ERROR_MSG_FORMAT(CAudioError::EError::ERROR_NOT_SUPPORTED_TYPE, "The streamType of output is not supported [streamType:%s]", streamType);
    return;
}

