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

#ifndef __TIZEN_MEDIA_AUDIO_IO_IAUDIO_SESSION_EVENT_LISTENER_H__
#define __TIZEN_MEDIA_AUDIO_IO_IAUDIO_SESSION_EVENT_LISTENER_H__


#ifdef __cplusplus


#include <mm_sound.h>
#include <mm_sound_focus.h>


namespace tizen_media_audio {


    /**
     * Called by ASM Thread
     */
    class CAudioSessionHandler;
    class IAudioSessionEventListener {
    public:
        enum class EInterruptCode : unsigned int {
            INTERRUPT_COMPLETED = 0,         /**< Interrupt completed */
            INTERRUPT_BY_MEDIA,              /**< Interrupted by a media application */
            INTERRUPT_BY_CALL,               /**< Interrupted by an incoming call */
            INTERRUPT_BY_EARJACK_UNPLUG,     /**< Interrupted by unplugging headphones */
            INTERRUPT_BY_RESOURCE_CONFLICT,  /**< Interrupted by a resource conflict */
            INTERRUPT_BY_ALARM,              /**< Interrupted by an alarm */
            INTERRUPT_BY_EMERGENCY,          /**< Interrupted by an emergency */
            INTERRUPT_BY_NOTIFICATION,       /**< Interrupted by a notification */
            INTERRUPT_MAX
        };

        static EInterruptCode convertInterruptedCode(int code, const char *reason_for_change);
        virtual void onInterrupt(CAudioSessionHandler* pHandler, int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info) = 0;
        virtual void onSignal(CAudioSessionHandler* pHandler, mm_sound_signal_name_t signal, int value) = 0;
    };


} /* namespace tizen_media_audio */

#endif
#endif /* __TIZEN_MEDIA_AUDIO_IO_IAUDIO_SESSION_EVENT_LISTENER_H__ */
