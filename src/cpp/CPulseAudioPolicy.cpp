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
 * class CPulseAudioPolicy
 */
CPulseAudioPolicy::CPulseAudioPolicy() :
    mPolicy(POLICY_DEFAULT) {
}

CPulseAudioPolicy::CPulseAudioPolicy(EPolicy policy) :
    mPolicy(policy) {
}

CPulseAudioPolicy::~CPulseAudioPolicy() {
}

void CPulseAudioPolicy::setPolicy(EPolicy policy) throw (CAudioError) {
    if (policy < POLICY_DEFAULT || policy >= POLICY_MAX) {
        THROW_ERROR_MSG(CAudioError::ERROR_INVALID_ARGUMENT, "The argument is out of range");
    }

    mPolicy = policy;
}

CPulseAudioPolicy::EPolicy CPulseAudioPolicy::getPolicy() {
    return mPolicy;
}

bool CPulseAudioPolicy::operator != (const EPolicy policy) {
    return (mPolicy != policy);
}

bool CPulseAudioPolicy::operator == (const EPolicy policy) {
    return (mPolicy == policy);
}
#endif
