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


#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


static const char* STREAM_NAME_INPUT               = "CAPTURE";
static const char* STREAM_NAME_INPUT_LOW_LATENCY   = "LOW LATENCY CAPTURE";
static const char* STREAM_NAME_INPUT_HIGH_LATENCY  = "HIGH LATENCY CAPTURE";
static const char* STREAM_NAME_INPUT_VOIP          = "VOIP CAPTURE";

static const char* STREAM_NAME_OUTPUT              = "PLAYBACK";
static const char* STREAM_NAME_OUTPUT_LOW_LATENCY  = "LOW LATENCY PLAYBACK";
static const char* STREAM_NAME_OUTPUT_HIGH_LATENCY = "HIGH LATENCY PLAYBACK";
static const char* STREAM_NAME_OUTPUT_VOIP         = "VOIP PLAYBACK";

static const char* STREAM_LATENCY_LOW  = "low";
static const char* STREAM_LATENCY_MID  = "mid";
static const char* STREAM_LATENCY_HIGH = "high";
static const char* STREAM_LATENCY_VOIP = "voip";


CPulseStreamSpec::CPulseStreamSpec() throw (CAudioError) :
    __mLatency(EStreamLatency::STREAM_LATENCY_INPUT_MID),
    __mStreamName(NULL) {
    __adjustSpec();
}

CPulseStreamSpec::CPulseStreamSpec(EStreamLatency latency, CAudioInfo& audioInfo) throw (CAudioError) :
    __mLatency(latency),
    __mAudioInfo(audioInfo),
    __mStreamName(NULL) {
    __adjustSpec();
}

CPulseStreamSpec::CPulseStreamSpec(EStreamLatency latency, CAudioInfo& audioInfo, int customLatency) throw (CAudioError) :
    __mLatency(latency),
    __mAudioInfo(audioInfo),
    __mStreamName(NULL) {
    __adjustSpec();
}

CPulseStreamSpec::~CPulseStreamSpec() {
}

void CPulseStreamSpec::__adjustSpec() throw (CAudioError) {
    // Sets a sampleRate
    __mSampleSpec.rate = __mAudioInfo.getSampleRate();

    // Convert channels for PA
    switch (__mAudioInfo.getChannel()) {
    case CAudioInfo::EChannel::CHANNEL_MONO:
        __mSampleSpec.channels = 1;
        break;

    case CAudioInfo::EChannel::CHANNEL_STEREO:
    default:
        __mSampleSpec.channels = 2;
        break;
    }

    // Convert format for PA
    switch (__mAudioInfo.getSampleType()) {
    case CAudioInfo::ESampleType::SAMPLE_TYPE_U8:
        __mSampleSpec.format = PA_SAMPLE_U8;
        break;

    case CAudioInfo::ESampleType::SAMPLE_TYPE_S16_LE:
    default:
        __mSampleSpec.format = PA_SAMPLE_S16LE;
        break;
    }

    // Sets channelmap
    pa_channel_map_init_auto(&__mChannelMap, __mSampleSpec.channels, PA_CHANNEL_MAP_ALSA);

    // Sets stream name
    switch (__mLatency) {
    case EStreamLatency::STREAM_LATENCY_OUTPUT_MID:
        __mStreamName = STREAM_NAME_OUTPUT;
        break;

    case EStreamLatency::STREAM_LATENCY_OUTPUT_HIGH:
        __mStreamName = STREAM_NAME_OUTPUT_HIGH_LATENCY;
        break;

    case EStreamLatency::STREAM_LATENCY_OUTPUT_LOW:
        __mStreamName = STREAM_NAME_OUTPUT_LOW_LATENCY;
        break;

    case EStreamLatency::STREAM_LATENCY_OUTPUT_VOIP:
        __mStreamName = STREAM_NAME_OUTPUT_VOIP;
        break;

    case EStreamLatency::STREAM_LATENCY_INPUT_HIGH:
        __mStreamName = STREAM_NAME_INPUT_HIGH_LATENCY;
        break;

    case EStreamLatency::STREAM_LATENCY_INPUT_LOW:
        __mStreamName = STREAM_NAME_INPUT_LOW_LATENCY;
        break;

    case EStreamLatency::STREAM_LATENCY_INPUT_VOIP:
        __mStreamName = STREAM_NAME_INPUT_VOIP;
        break;

    case EStreamLatency::STREAM_LATENCY_INPUT_MID:
    default:
        __mStreamName = STREAM_NAME_INPUT;
        break;
    }
}

CPulseStreamSpec::EStreamLatency CPulseStreamSpec::getStreamLatency() {
    return __mLatency;
}

const char* CPulseStreamSpec::getStreamLatencyToString() {
    const char* latency;

    switch (__mLatency) {
    case EStreamLatency::STREAM_LATENCY_INPUT_LOW:
    case EStreamLatency::STREAM_LATENCY_OUTPUT_LOW:
        latency = STREAM_LATENCY_LOW;
        break;
    case EStreamLatency::STREAM_LATENCY_INPUT_MID:
    case EStreamLatency::STREAM_LATENCY_OUTPUT_MID:
        latency = STREAM_LATENCY_MID;
        break;
    case EStreamLatency::STREAM_LATENCY_INPUT_HIGH:
    case EStreamLatency::STREAM_LATENCY_OUTPUT_HIGH:
        latency = STREAM_LATENCY_HIGH;
        break;
    case EStreamLatency::STREAM_LATENCY_INPUT_VOIP:
    case EStreamLatency::STREAM_LATENCY_OUTPUT_VOIP:
        latency = STREAM_LATENCY_VOIP;
        break;
    default:
        latency = STREAM_LATENCY_MID;
        break;
    }

    return latency;
}

CAudioInfo CPulseStreamSpec::getAudioInfo() {
    return __mAudioInfo;
}

pa_sample_spec CPulseStreamSpec::getSampleSpec() {
    return __mSampleSpec;
}

pa_channel_map CPulseStreamSpec::getChannelMap() {
    return __mChannelMap;
}

const char* CPulseStreamSpec::getStreamName() {
    return __mStreamName;
}
