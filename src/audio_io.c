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


#include <mm.h>
#include <cpp_audio_io.h>


int audio_in_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type , audio_in_h* input)
{
    return cpp_audio_in_create(sample_rate, channel, type, input);
}

int audio_in_create_loopback(int sample_rate, audio_channel_e channel, audio_sample_type_e type, audio_in_h* input)
{
    return cpp_audio_in_create_loopback(sample_rate, channel, type, input);
}

int audio_in_destroy(audio_in_h input)
{
    return cpp_audio_in_destroy(input);
}

int audio_in_set_stream_info(audio_in_h input, sound_stream_info_h stream_info)
{
    return cpp_audio_in_set_stream_info(input, stream_info);
}

int audio_in_prepare(audio_in_h input)
{
    return cpp_audio_in_prepare(input);
}

int audio_in_unprepare(audio_in_h input)
{
    return cpp_audio_in_unprepare(input);
}

int audio_in_pause(audio_in_h input)
{
    return cpp_audio_in_pause(input);
}

int audio_in_resume(audio_in_h input)
{
    return cpp_audio_in_resume(input);
}

int audio_in_flush(audio_in_h input)
{
    return cpp_audio_in_flush(input);
}

int audio_in_read(audio_in_h input, void *buffer, unsigned int length )
{
    return cpp_audio_in_read(input, buffer, length);
}

int audio_in_get_buffer_size(audio_in_h input, int *size)
{
    return cpp_audio_in_get_buffer_size(input, size);
}

int audio_in_get_sample_rate(audio_in_h input, int *sample_rate)
{
    return cpp_audio_in_get_sample_rate(input, sample_rate);
}

int audio_in_get_channel(audio_in_h input, audio_channel_e *channel)
{
    return cpp_audio_in_get_channel(input, channel);
}

int audio_in_get_sample_type(audio_in_h input, audio_sample_type_e *type)
{
    return cpp_audio_in_get_sample_type(input, type);
}

int audio_in_set_interrupted_cb(audio_in_h input, audio_io_interrupted_cb callback, void *user_data)
{
    return cpp_audio_in_set_interrupted_cb(input, callback, user_data);
}

int audio_in_unset_interrupted_cb(audio_in_h input)
{
    return cpp_audio_in_unset_interrupted_cb(input);
}

int audio_in_ignore_session(audio_in_h input)
{
    return cpp_audio_in_ignore_session(input);
}

int audio_in_set_stream_cb(audio_in_h input, audio_in_stream_cb callback, void* user_data)
{
    return cpp_audio_in_set_stream_cb(input, callback, user_data);
}

int audio_in_unset_stream_cb(audio_in_h input)
{
    return cpp_audio_in_unset_stream_cb(input);
}

int audio_in_peek(audio_in_h input, const void **buffer, unsigned int *length)
{
    return cpp_audio_in_peek(input, buffer, length);
}

int audio_in_drop(audio_in_h input)
{
    return cpp_audio_in_drop(input);
}

int audio_in_set_state_changed_cb(audio_in_h input, audio_in_state_changed_cb callback, void* user_data)
{
    return cpp_audio_in_set_state_changed_cb(input, callback, user_data);
}

int audio_in_unset_state_changed_cb(audio_in_h input)
{
    return cpp_audio_in_unset_state_changed_cb(input);
}


/* Audio Out */
int audio_out_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type, sound_type_e sound_type,  audio_out_h* output)
{
    return cpp_audio_out_create(sample_rate, channel, type, sound_type, output);
}

int audio_out_create_new(int sample_rate, audio_channel_e channel, audio_sample_type_e type, audio_out_h *output)
{
    return cpp_audio_out_create_new(sample_rate, channel, type, output);
}

int audio_out_destroy(audio_out_h output)
{
    return cpp_audio_out_destroy(output);
}

int audio_out_set_stream_info(audio_out_h output, sound_stream_info_h stream_info)
{
    return cpp_audio_out_set_stream_info(output, stream_info);
}

int audio_out_prepare(audio_out_h output)
{
    return cpp_audio_out_prepare(output);
}

int audio_out_unprepare(audio_out_h output)
{
    return cpp_audio_out_unprepare(output);
}

int audio_out_pause(audio_out_h output)
{
    return cpp_audio_out_pause(output);
}

int audio_out_resume(audio_out_h output)
{
    return cpp_audio_out_resume(output);
}

int audio_out_drain(audio_out_h output)
{
    return cpp_audio_out_drain(output);
}

int audio_out_flush(audio_out_h output)
{
    return cpp_audio_out_flush(output);
}

int audio_out_write(audio_out_h output, void* buffer, unsigned int length)
{
    return cpp_audio_out_write(output, buffer, length);
}

int audio_out_get_buffer_size(audio_out_h output, int *size)
{
    return cpp_audio_out_get_buffer_size(output, size);
}

int audio_out_get_sample_rate(audio_out_h output, int *sample_rate)
{
    return cpp_audio_out_get_sample_rate(output, sample_rate);
}

int audio_out_get_channel(audio_out_h output, audio_channel_e *channel)
{
    return cpp_audio_out_get_channel(output, channel);
}

int audio_out_get_sample_type(audio_out_h output, audio_sample_type_e *type)
{
    return cpp_audio_out_get_sample_type(output, type);
}

int audio_out_get_sound_type(audio_out_h output, sound_type_e *type)
{
    return cpp_audio_out_get_sound_type(output, type);
}

int audio_out_set_interrupted_cb(audio_out_h output, audio_io_interrupted_cb callback, void *user_data)
{
    return cpp_audio_out_set_interrupted_cb(output, callback, user_data);
}

int audio_out_unset_interrupted_cb(audio_out_h output)
{
    return cpp_audio_out_unset_interrupted_cb(output);
}

int audio_out_ignore_session(audio_out_h output)
{
    return cpp_audio_out_ignore_session(output);
}

int audio_out_set_stream_cb(audio_out_h output, audio_out_stream_cb callback, void* user_data)
{
    return cpp_audio_out_set_stream_cb(output, callback, user_data);
}

int audio_out_unset_stream_cb(audio_out_h output)
{
    return cpp_audio_out_unset_stream_cb(output);
}

int audio_out_set_state_changed_cb(audio_out_h output, audio_out_state_changed_cb callback, void* user_data)
{
    return cpp_audio_out_set_state_changed_cb(output, callback, user_data);
}

int audio_out_unset_state_changed_cb(audio_out_h output)
{
    return cpp_audio_out_unset_state_changed_cb(output);
}
