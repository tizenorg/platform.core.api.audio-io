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


CPulseStreamSpec::CPulseStreamSpec() throw (CAudioError)
        : mLatency(STREAM_LATENCY_INPUT_MID), mStreamName(NULL) {
    _adjustSpec();
}

CPulseStreamSpec::CPulseStreamSpec(EStreamLatency latency, CAudioInfo& audioInfo) throw (CAudioError)
        : mLatency(latency), mAudioInfo(audioInfo), mStreamName(NULL) {
    _adjustSpec();
}

CPulseStreamSpec::CPulseStreamSpec(EStreamLatency latency, CAudioInfo& audioInfo, int customLatency) throw (CAudioError)
        : mLatency(latency), mAudioInfo(audioInfo), mStreamName(NULL) {
    _adjustSpec();
}

CPulseStreamSpec::~CPulseStreamSpec() {
}

void CPulseStreamSpec::_adjustSpec() throw (CAudioError) {
    // Sets a sampleRate
    mSampleSpec.rate = mAudioInfo.getSampleRate();

    // Convert channels for PA
    switch (mAudioInfo.getChannel()) {
    case CAudioInfo::CHANNEL_MONO:
        mSampleSpec.channels = 1;
        break;

    case CAudioInfo::CHANNEL_STEREO:
    default:
        mSampleSpec.channels = 2;
        break;
    }

    // Convert format for PA
    switch (mAudioInfo.getSampleType()) {
    case CAudioInfo::SAMPLE_TYPE_U8:
        mSampleSpec.format = PA_SAMPLE_U8;
        break;

    case CAudioInfo::SAMPLE_TYPE_S16_LE:
    default:
        mSampleSpec.format = PA_SAMPLE_S16LE;
        break;
    }

    // Sets channelmap
    pa_channel_map_init_auto(&mChannelMap, mSampleSpec.channels, PA_CHANNEL_MAP_ALSA);

    // Sets stream name
    switch (mLatency) {
    case STREAM_LATENCY_OUTPUT_MID:
        mStreamName = STREAM_NAME_OUTPUT;
        break;

    case STREAM_LATENCY_OUTPUT_HIGH:
        mStreamName = STREAM_NAME_OUTPUT_HIGH_LATENCY;
        break;

    case STREAM_LATENCY_OUTPUT_LOW:
        mStreamName = STREAM_NAME_OUTPUT_LOW_LATENCY;
        break;

    case STREAM_LATENCY_OUTPUT_VOIP:
         mStreamName = STREAM_NAME_OUTPUT_VOIP;
         break;

    case STREAM_LATENCY_INPUT_HIGH:
        mStreamName = STREAM_NAME_INPUT_HIGH_LATENCY;
        break;

    case STREAM_LATENCY_INPUT_LOW:
        mStreamName = STREAM_NAME_INPUT_LOW_LATENCY;
        break;

    case STREAM_LATENCY_INPUT_VOIP:
        mStreamName = STREAM_NAME_INPUT_VOIP;
        break;

    case STREAM_LATENCY_INPUT_MID:
    default:
        mStreamName = STREAM_NAME_INPUT;
        break;
    }
}

CPulseStreamSpec::EStreamLatency CPulseStreamSpec::getStreamLatency() {
    return mLatency;
}

CAudioInfo CPulseStreamSpec::getAudioInfo() {
    return mAudioInfo;
}

pa_sample_spec CPulseStreamSpec::getSampleSpec() {
    return mSampleSpec;
}

pa_channel_map CPulseStreamSpec::getChannelMap() {
    return mChannelMap;
}

const char* CPulseStreamSpec::getStreamName() {
    return mStreamName;
}
