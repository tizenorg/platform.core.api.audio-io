/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __TIZEN_MEDIA_AUDIO_IO_H__
#define __TIZEN_MEDIA_AUDIO_IO_H__

#include <tizen.h>
#include <sound_manager.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define AUDIO_IO_ERROR_CLASS          TIZEN_ERROR_MULTIMEDIA_CLASS | 0x40

/**
 * @file audio_io.h
 * @brief This file contains the Audio Input and Output API.
 */

/**
 * @addtogroup CAPI_MEDIA_AUDIO_IN_MODULE
 * @{
*/

/**
 * @brief Audio input handle type.
 */
typedef struct audio_in_s *audio_in_h;

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_AUDIO_OUT_MODULE
 * @{
 */
 
/**
 * @brief Audio output handle type.
 */
typedef struct audio_out_s *audio_out_h;

 /**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_AUDIO_IO_MODULE
 * @{
 */

/**
 * @brief Enumerations of audio sample type with bit depth
 */
typedef enum
{
    AUDIO_SAMPLE_TYPE_U8 = 0x70,   /**< Unsigned 8-bit audio samples */
    AUDIO_SAMPLE_TYPE_S16_LE,   /**< Signed 16-bit audio samples */
} audio_sample_type_e;

/**
 * @brief Enumerations of audio channel
 */
typedef enum {
    AUDIO_CHANNEL_MONO = 0x80,    /**< 1 channel, mono */
    AUDIO_CHANNEL_STEREO,      /**< 2 channel, stereo */
} audio_channel_e;

/**
 * @brief Enumerations of audio input and output error code
 */
typedef enum{
    AUDIO_IO_ERROR_NONE                = TIZEN_ERROR_NONE,                /**<Successful */
    AUDIO_IO_ERROR_OUT_OF_MEMORY       = TIZEN_ERROR_OUT_OF_MEMORY,     /**< Out of memory */
    AUDIO_IO_ERROR_INVALID_PARAMETER   = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
    AUDIO_IO_ERROR_INVALID_OPERATION   = TIZEN_ERROR_INVALID_OPERATION, /**< Invalid operation */
    AUDIO_IO_ERROR_DEVICE_NOT_OPENED   = AUDIO_IO_ERROR_CLASS | 0x01, /**< Device open error */
    AUDIO_IO_ERROR_DEVICE_NOT_CLOSED   = AUDIO_IO_ERROR_CLASS | 0x02, /**< Device close error */
    AUDIO_IO_ERROR_INVALID_BUFFER      = AUDIO_IO_ERROR_CLASS | 0x03, /**< Invalid buffer pointer */
    AUDIO_IO_ERROR_SOUND_POLICY        = AUDIO_IO_ERROR_CLASS | 0x04, /**< Sound policy error */
} audio_io_error_e;


/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_AUDIO_IN_MODULE
 * @{
*/


//
//AUDIO INPUT
//


/**
 * @brief    Creates an audio device instance and returns an input handle to record PCM (pulse-code modulation) data
 * @details  This function is used for audio input initialization.
 *
 * @remarks @a input must be release audio_in_destroy() by you.
 *
 * @param[in]  sample_rate	The audio sample rate in 8000[Hz] ~ 44100[Hz]
 * @param[in]  channel	The audio channel type, mono, or stereo
 * @param[in]  type	The type of audio sample (8- or 16-bit)
 * @param[out] input	An audio input handle will be created, if successful
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #AUDIO_IO_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #AUDIO_IO_ERROR_DEVICE_NOT_OPENED Device not opened
 * @retval #AUDIO_IO_ERROR_SOUND_POLICY    Sound policy error
 * @see audio_in_destroy()
 */
int audio_in_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type , audio_in_h *input);



/**
 * @brief    Releases the audio input handle and all its resources associated with an audio stream
 *
 * @param[in]	input	The handle to the audio input to destroy
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #AUDIO_IO_ERROR_DEVICE_NOT_CLOSED Device not closed
 * @see audio_in_create()
 */
int audio_in_destroy(audio_in_h input);



/**
 * @brief    Starts reading and buffering the audio data from the device
 *
 * @param[in]	input	The handle to the audio input
 * @param[out]	buffer	The PCM buffer address
 * @param[in]	length	The length of PCM data buffer (in bytes)
 *
 * @return  Number of read bytes on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval  #AUDIO_IO_ERROR_INVALID_BUFFER  Invalid buffer pointer
 * @retval  #AUDIO_IO_ERROR_SOUND_POLICY    Sound policy error
*/
int audio_in_read(audio_in_h input, void *buffer, unsigned int length);



/**
 * @brief    Gets the size to be allocated for audio input buffer
 * @param[in]   input	The handle to the audio input
 * @param[out]  size	The buffer size (in bytes). \n The maximum size is 1 MB.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_NONE Successful
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @see audio_in_read() 
*/
int audio_in_get_buffer_size(audio_in_h input, int *size);



/**
 * @brief    Gets the sample rate of the audio input data stream
 *
 * @param[in]   input	The handle to the audio input
 * @param[out]  sample_rate  The audio sample rate in Hertz (8000 ~ 44100)
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_NONE Successful
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_in_get_sample_rate(audio_in_h input, int *sample_rate);



/**
 * @brief    Gets the channel type of audio input data stream
 *
 * @details  The audio channel type defines whether the audio is mono or stereo.
 *
 * @param[in]   input   The handle to the audio input
 * @param[out]  channel The audio channel type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_in_get_channel(audio_in_h input, audio_channel_e *channel);



/**
 * @brief    Gets the sample audio format (8-bit or 16-bit) of audio input data stream
 *
 * @param[in]  input    The handle to the audio input
 * @param[out] type     The audio sample type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_in_get_sample_type(audio_in_h input, audio_sample_type_e *type);




//
//  AUDIO OUTPUT
//

/**
 * @}
*/

/**
 * @addtogroup CAPI_MEDIA_AUDIO_OUT_MODULE
 * @{
 */

/**
 * @brief    Creates an audio device instance and returns an output handle to play PCM (pulse-code modulation) data
 * @details  This function is used for audio output initialization. 
 * @remarks @a output must be released audio_out_destroy() by you.
 *
 * @param[in]  sample_rate  The audio sample rate in 8000[Hz] ~ 44100[Hz]
 * @param[in]  channel      The audio channel type, mono, or stereo
 * @param[in]  type         The type of audio sample (8- or 16-bit)
 * @param[in]  sound_type   The type of sound (#sound_type_e)
 * @param[out] output       An audio output handle will be created, if successful
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #AUDIO_IO_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #AUDIO_IO_ERROR_DEVICE_NOT_OPENED Device not opened
 * @retval #AUDIO_IO_ERROR_SOUND_POLICY    Sound policy error
 *
 * @see audio_out_destroy()
*/
int audio_out_create(int sample_rate, audio_channel_e channel, audio_sample_type_e type, sound_type_e sound_type,  audio_out_h *output);



/**
 * @brief    Releases the audio output handle, along with all its resources
 *
 * @param[in]	output The handle to the audio output to destroy
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_NONE Successful
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval  #AUDIO_IO_ERROR_OUT_OF_MEMORY Out of memory
 * @retval  #AUDIO_IO_ERROR_DEVICE_NOT_CLOSED Device not closed
 *
 * @see audio_out_create()
*/
int audio_out_destroy(audio_out_h output);



/**
 * @brief    Starts writing the audio data to the device
 *
 * @param[in]       output  The handle to the audio output
 * @param[in,out]   buffer  The PCM buffer address
 * @param[in]       length  The length of PCM buffer (in bytes)
 *
 * @return  Written data size on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval  #AUDIO_IO_ERROR_INVALID_BUFFER  Invalid buffer pointer
 * @retval  #AUDIO_IO_ERROR_SOUND_POLICY    Sound policy error
*/
int audio_out_write(audio_out_h output, void *buffer, unsigned int length);



/**
 * @brief    Gets the size to be allocated for audio output buffer
 * @param[in]     output  The handle to the audio output
 * @param[out]    size    The suggested buffer size (in bytes). \n The maximum size is 1 MB.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_NONE Successful
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
 * @see audio_out_write()
 *
*/
int audio_out_get_buffer_size(audio_out_h output, int *size);



/**
 * @brief    Gets the sample rate of audio output data stream
 *
 * @param[in]   output       The handle to the audio output
 * @param[out]  sample_rate  The audio sample rate in Hertz (8000 ~ 44100)
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #AUDIO_IO_ERROR_NONE Successful
 * @retval  #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_out_get_sample_rate(audio_out_h output, int *sample_rate);



/**
 * @brief    Gets the channel type of audio output data stream
 *
 * @details  The audio channel type defines whether the audio is mono or stereo.
 *
 * @param[in]   output  The handle to the audio output
 * @param[out]  channel The audio channel type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_out_get_channel(audio_out_h output, audio_channel_e *channel);



/**
 * @brief    Gets the sample audio format (8-bit or 16-bit) of audio output data stream
 *
 * @param[in]   output  The handle to the audio output
 * @param[out]  type    The audio sample type
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_out_get_sample_type(audio_out_h output, audio_sample_type_e *type);



/**
 * @brief    Gets the sound type supported by the audio output device
 * @param[in]  output   The handle to the audio output
 * @param[out] type     The sound type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #AUDIO_IO_ERROR_NONE Successful
 * @retval #AUDIO_IO_ERROR_INVALID_PARAMETER Invalid parameter
*/
int audio_out_get_sound_type(audio_out_h output, sound_type_e *type);



/**
 * @}
*/

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_AUDIO_IO_H__ */
