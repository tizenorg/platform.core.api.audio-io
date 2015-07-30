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
#include "CAudioIODef.h"


using namespace std;
using namespace tizen_media_audio;


/**
 * class CPulseAudioVolume
 */
CPulseAudioVolume/::CPulseAudioVolume() : mVolume(VOLUME_MEDIA), mVolumeGain(VOLUME_GAIN_DEFAULT) {
}

CPulseAudioVolume::CPulseAudioVolume(EVolume volume, EVolumeGain gain) :
    mVolume(volume),
    mVolumeGain(gain) {
}

CPulseAudioVolume::~CPulseAudioVolume() {
}

void CPulseAudioVolume::setVolume(EVolume volume) {
    mVolume = volume;
}

CPulseAudioVolume::EVolume CPulseAudioVolume::getVolume() {
    return mVolume;
}

void CPulseAudioVolume::setVolumeGain(EVolumeGain volumeGain) {
    mVolumeGain = volumeGain;
}

CPulseAudioVolume::EVolumeGain CPulseAudioVolume::getVolumeGain() {
    return mVolumeGain;
}
#endif
