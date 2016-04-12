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

#ifndef __TIZEN_MEDIA_CPP_AUDIO_IO_H__
#define __TIZEN_MEDIA_CPP_AUDIO_IO_H__

#include "audio_io.h"


#ifdef __cplusplus


extern "C" {
#endif

int cpp_audio_in_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type, audio_in_h *input);
int cpp_audio_in_create_loopback(int sample_rate, audio_channel_e channel, audio_sample_type_e type , audio_in_h* input);
int cpp_audio_in_destroy(audio_in_h input);
int cpp_audio_in_set_stream_info(audio_in_h input, sound_stream_info_h stream_info);
int cpp_audio_in_prepare(audio_in_h input);
int cpp_audio_in_unprepare(audio_in_h input);
int cpp_audio_in_pause(audio_in_h input);
int cpp_audio_in_resume(audio_in_h input);
int cpp_audio_in_drain(audio_in_h input);
int cpp_audio_in_flush(audio_in_h input);
int cpp_audio_in_read(audio_in_h input, void *buffer, unsigned int length);
int cpp_audio_in_get_buffer_size(audio_in_h input, int *size);
int cpp_audio_in_get_sample_rate(audio_in_h input, int *sample_rate);
int cpp_audio_in_get_channel(audio_in_h input, audio_channel_e *channel);
int cpp_audio_in_get_sample_type(audio_in_h input, audio_sample_type_e *type);
int cpp_audio_in_set_interrupted_cb(audio_in_h input, audio_io_interrupted_cb callback, void *user_data);
int cpp_audio_in_unset_interrupted_cb(audio_in_h input);
int cpp_audio_in_ignore_session(audio_in_h input);
int cpp_audio_in_set_stream_cb(audio_in_h input, audio_in_stream_cb callback, void* user_data);
int cpp_audio_in_unset_stream_cb(audio_in_h input);
int cpp_audio_in_peek(audio_in_h input, const void **buffer, unsigned int *length);
int cpp_audio_in_drop(audio_in_h input);
int cpp_audio_in_set_state_changed_cb(audio_in_h input, audio_in_state_changed_cb callback, void* user_data);
int cpp_audio_in_unset_state_changed_cb(audio_in_h input);


int cpp_audio_out_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type, sound_type_e sound_type, audio_out_h *output);
int cpp_audio_out_create_new(int sample_rate, audio_channel_e channel, audio_sample_type_e type, audio_out_h *output);
int cpp_audio_out_destroy(audio_out_h output);
int cpp_audio_out_set_stream_info(audio_out_h output, sound_stream_info_h stream_info);
int cpp_audio_out_prepare(audio_out_h output);
int cpp_audio_out_unprepare(audio_out_h output);
int cpp_audio_out_pause(audio_out_h output);
int cpp_audio_out_resume(audio_out_h output);
int cpp_audio_out_drain(audio_out_h output);
int cpp_audio_out_flush(audio_out_h output);
int cpp_audio_out_write(audio_out_h output, void *buffer, unsigned int length);
int cpp_audio_out_get_buffer_size(audio_out_h output, int *size);
int cpp_audio_out_get_sample_rate(audio_out_h output, int *sample_rate);
int cpp_audio_out_get_channel(audio_out_h output, audio_channel_e *channel);
int cpp_audio_out_get_sample_type(audio_out_h output, audio_sample_type_e *type);
int cpp_audio_out_get_sound_type(audio_out_h output, sound_type_e *type);
int cpp_audio_out_set_interrupted_cb(audio_out_h output, audio_io_interrupted_cb callback, void *user_data);
int cpp_audio_out_unset_interrupted_cb(audio_out_h output);
int cpp_audio_out_ignore_session(audio_out_h output);
int cpp_audio_out_set_stream_cb(audio_out_h output, audio_out_stream_cb callback, void* user_data);
int cpp_audio_out_unset_stream_cb(audio_out_h output);
int cpp_audio_out_set_state_changed_cb(audio_out_h output, audio_in_state_changed_cb callback, void* user_data);
int cpp_audio_out_unset_state_changed_cb(audio_out_h output);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_CPP_AUDIO_IO_H__ */
