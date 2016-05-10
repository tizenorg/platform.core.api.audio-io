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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sound_manager.h>
#include <audio_io.h>

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)
typedef struct {
	float sine[TABLE_SIZE];
	int left_channel;
	int right_channel;
} test_wav_t;
test_wav_t test_wav;

static int ch_table[3] = { 0, AUDIO_CHANNEL_MONO, AUDIO_CHANNEL_STEREO };

void play_file(char *file, int length, int ch)
{
	audio_out_h output;
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		printf("fopen failed\n");
		return;
	}

	char *buf = malloc(length);
	if (buf == NULL) {
		printf("malloc failed\n");
		fclose(fp);
		return;
	}

	printf("start to play [%s][%d][%d]\n", file, length, ch);
	audio_out_create_new(44100, ch_table[ch], AUDIO_SAMPLE_TYPE_S16_LE, &output);
	if (fread(buf, 1, length, fp) != length)
		printf("error!!!!\n");

	audio_out_prepare(output);
	audio_out_write(output, buf, length);
	audio_out_unprepare(output);

	audio_out_destroy(output);

	fclose(fp);

	printf("play done\n");
}

#define DUMP_FILE "/root/test.raw"

void play_file_sample(char *file, int frequency, int ch, int type)
{
	audio_out_h output;
	int file_size = 0;
	int read_bytes = 0;
	int buffer_size = 0;
	char *buf = NULL;

	if (ch < 0 || ch > 2)
		ch = 0;

	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		printf("open failed\n");
		return;
	}
	/* Get the size */
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	printf("start to play [%s] of size [%d] with [%d][%d][%d]\n", file, file_size, frequency, ch, type);
	if (type)
		audio_out_create_new(frequency, ch_table[ch], AUDIO_SAMPLE_TYPE_S16_LE, &output);
	else
		audio_out_create_new(frequency, ch_table[ch], AUDIO_SAMPLE_TYPE_U8, &output);

	audio_out_prepare(output);
	audio_out_get_buffer_size(output, &buffer_size);

	buf = (char *)malloc(buffer_size);
	if (buf == NULL) {
		printf("malloc failed\n");
		audio_out_unprepare(output);
		audio_out_destroy(output);
		fclose(fp);
		return;
	}

	while (file_size > 0) {
		read_bytes = fread(buf, 1, buffer_size, fp);
		printf("Read %d Requested - %d\n", read_bytes, buffer_size);
		audio_out_write(output, buf, read_bytes);
		file_size = file_size - read_bytes;
	}

	audio_out_unprepare(output);
	audio_out_destroy(output);

	free(buf);
	fclose(fp);
	printf("play done\n");
}

int audio_io_test(int length, int num, int ch)
{
	int ret, size, i;
	audio_in_h input;
	if ((ret = audio_in_create(44100, ch_table[ch], AUDIO_SAMPLE_TYPE_S16_LE, &input)) == AUDIO_IO_ERROR_NONE) {
		ret = audio_in_ignore_session(input);
		if (ret != 0) {
			printf("ERROR, set session mix\n");
			audio_in_destroy(input);
			return 0;
		}

		ret = audio_in_prepare(input);
		if (ret != 0) {
			printf("ERROR, prepare\n");
			audio_in_destroy(input);
			return 0;
		}

		FILE *fp = fopen(DUMP_FILE, "wb+");

		if (fp == NULL) {
			printf("ERROR, file open failed\n");
			audio_in_destroy(input);
			return 0;
		}

		if ((ret = audio_in_get_buffer_size(input, &size)) == AUDIO_IO_ERROR_NONE) {
			size = length;
			char *buffer = alloca(size);

			for (i = 0; i < num; i++) {
				printf("### loop = %d ============== \n", i);
				if ((ret = audio_in_read(input, (void *)buffer, size)) > AUDIO_IO_ERROR_NONE) {
					fwrite(buffer, size, sizeof(char), fp);
					printf("PASS, size=%d, ret=%d\n", size, ret);
				} else {
					printf("FAIL, size=%d, ret=%d\n", size, ret);
				}
			}
		}

		fclose(fp);

		audio_in_destroy(input);
	}

	play_file(DUMP_FILE, length * num, ch);

	return 1;
}

int audio_io_loopback_in_test()
{
	int ret, size;
	audio_in_h input;
	FILE *fp = fopen("/tmp/dump_test.raw", "wb+");

	if (fp == NULL) {
		printf("open failed \n");
		return 0;
	}

	if ((ret = audio_in_create(16000, AUDIO_CHANNEL_MONO, AUDIO_SAMPLE_TYPE_S16_LE, &input)) == AUDIO_IO_ERROR_NONE) {
		ret = audio_in_ignore_session(input);
		if (ret != 0) {
			printf("ERROR, set session mix\n");
			goto exit;
		}

		ret = audio_in_prepare(input);
		if (ret != 0) {
			printf("ERROR, prepare\n");
			goto exit;
		}

		ret = audio_in_get_buffer_size(input, &size);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_in_get_buffer_size failed.\n");
			goto exit;
		}

		while (1) {
			char *buffer = alloca(size);
			if ((ret = audio_in_read(input, (void *)buffer, size)) > AUDIO_IO_ERROR_NONE) {
				fwrite(buffer, size, sizeof(char), fp);
				printf("PASS, size=%d, ret=%d\n", size, ret);
			} else {
				printf("FAIL, size=%d, ret=%d\n", size, ret);
			}
		}
	}

 exit:
	audio_in_destroy(input);

	fclose(fp);

	return ret;

}

int audio_io_loopback_test()
{
	int ret, size;
	audio_in_h input;
	audio_out_h output;
	char *buffer = NULL;

	ret = audio_in_create(16000, AUDIO_CHANNEL_MONO, AUDIO_SAMPLE_TYPE_S16_LE, &input);
	if (ret != AUDIO_IO_ERROR_NONE) {
		printf("audio_in_create_ex failed. \n");
		return 0;
	}
	ret = audio_out_create_new(16000, AUDIO_CHANNEL_MONO, AUDIO_SAMPLE_TYPE_S16_LE, &output);
	if (ret != AUDIO_IO_ERROR_NONE) {
		printf("audio_out_create failed. \n");
		return 0;
	}

	ret = audio_in_prepare(input);
	if (ret != 0) {
		printf("audio_in_prepare failed.\n");
		audio_in_destroy(input);
		return 0;
	} else {
		ret = audio_in_get_buffer_size(input, &size);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_in_get_buffer_size failed.\n");
			return 0;
		} else {
			printf("size(%d)\n", size);
			buffer = alloca(size);
		}
	}

	ret = audio_out_prepare(output);
	if (ret != 0) {
		printf("audio_out_prepare failed.\n");
		audio_out_destroy(output);
		return 0;
	}

	if (buffer == NULL) {
		printf("buffer is null\n");
		return 0;
	}

	while (1) {
		ret = audio_in_read(input, (void *)buffer, size);
		if (ret > AUDIO_IO_ERROR_NONE) {
			ret = audio_out_write(output, buffer, size);
			if (ret > AUDIO_IO_ERROR_NONE)
				printf("audio read/write success. buffer(%p), size(%d)\n", buffer, size);
			else
				printf("audio read success, write failed. buffer(%p), size(%d)\n", buffer, size);
		} else
			printf("audio read/write failed. buffer(%p), size(%d)\n", buffer, size);
	}

}

audio_in_h input;
audio_out_h output;

FILE *fp_w = NULL;

sound_stream_info_h g_stream_info_read_h = NULL;
sound_stream_info_h g_stream_info_write_h = NULL;

static void focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	int ret = 0;
	sound_stream_focus_state_e playback_focus_state;
	sound_stream_focus_state_e recording_focus_state;
	printf("*** focus_callback_read is called, stream_info(%p, read(%p)/write(%p)) ***\n", stream_info, g_stream_info_read_h, g_stream_info_write_h);
	printf(" - reason_for_change(%d), additional_info(%s), user_data(%p)\n", reason_for_change, additional_info, user_data);
	ret = sound_manager_get_focus_state(stream_info, &playback_focus_state, &recording_focus_state);
	if (!ret)
		printf(" - focus_state(playback_focus:%d, recording_focus:%d)\n", playback_focus_state, recording_focus_state);

	return;
}

static void _audio_io_stream_read_cb(audio_in_h handle, size_t nbytes, void *user_data)
{
	const void *buffer = NULL;
	unsigned int len = (unsigned int)nbytes;

	if (len > 0) {
		audio_in_peek(handle, &buffer, &len);
		if (fp_w)
			fwrite(buffer, sizeof(char), len, fp_w);
		audio_in_drop(handle);
	}
}

static void _audio_io_stream_write_cb(audio_out_h handle, size_t nbytes, void *user_data)
{
	short *buffer = NULL;
	int i = 0;

	if (nbytes > 0) {
		buffer = (short *)malloc(nbytes);
		if (buffer == NULL) {
			printf("malloc failed\n");
			return;
		}
		memset(buffer, 0, nbytes);

		for (i = 0; i < nbytes / 2; i += 2) {
			buffer[i] = (short)32768 *test_wav.sine[test_wav.left_channel];	/* left */
			buffer[i + 1] = (short)32768 *test_wav.sine[test_wav.right_channel];	/* right */
			test_wav.left_channel += 1;
			if (test_wav.left_channel >= TABLE_SIZE)
				test_wav.left_channel -= TABLE_SIZE;
			test_wav.right_channel += 3;
			if (test_wav.right_channel >= TABLE_SIZE)
				test_wav.right_channel -= TABLE_SIZE;
		}

		audio_out_write(handle, buffer, nbytes);

		free(buffer);
	}
}

static void _audio_in_state_cb(audio_in_h handle, audio_io_state_e previous, audio_io_state_e current, bool by_policy, void *user_data)
{
	printf(">>> _audio_in_state_cb() : handle(%p), current(%d), previous(%d), by_policy(%d), user_data(%p)\n", handle, current, previous, by_policy, user_data);
}

static void _audio_out_state_cb(audio_in_h handle, audio_io_state_e previous, audio_io_state_e current, bool by_policy, void *user_data)
{
	printf(">>> _audio_out_state_cb() : handle(%p), current(%d), previous(%d), by_policy(%d), user_data(%p)\n", handle, current, previous, by_policy, user_data);
}

int _convert_cmd_and_run(char cmd, int mode)
{
	int ret = 0;
	switch (cmd) {
	case 'P':
		if (mode & 0x01)
			ret = audio_out_prepare(output);
		if (mode & 0x02)
			ret = audio_in_prepare(input);
		break;
	case 'u':
		if (mode & 0x01)
			ret = audio_out_unprepare(output);
		if (mode & 0x02)
			ret = audio_in_unprepare(input);
		break;
	case 'p':
		if (mode & 0x01)
			ret = audio_out_pause(output);
		if (mode & 0x02)
			ret = audio_in_pause(input);
		break;
	case 'r':
		if (mode & 0x01)
			ret = audio_out_resume(output);
		if (mode & 0x02)
			ret = audio_in_resume(input);
		break;
	case 'd':
		if (mode & 0x01)
			ret = audio_out_drain(output);
		break;
	case 'f':
		if (mode & 0x01)
			ret = audio_out_flush(output);
		if (mode & 0x02)
			ret = audio_in_flush(input);
		break;
	case 'i':
		ret = sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, focus_callback, NULL, &g_stream_info_write_h);
		if (ret)
			printf("fail to sound_manager_create_stream_information(), ret(0x%x)\n", ret);
		break;
	case 'q':					/* quit */
		ret = 1;
		break;
	default:
		ret = 1;
		break;
	}
	return ret;
}

int audio_io_async_test(int mode)
{
	int ret, size;
	char *buffer = NULL;
	int i = 0;

	char cmd = 0;
	int cmd_ret;

	int write_mode = (mode & 0x01);
	int read_mode = (mode & 0x02);

	sound_stream_focus_state_e playback_focus_state;
	sound_stream_focus_state_e recording_focus_state;

	if ((write_mode == 0) && (read_mode == 0)) {
		printf("not vaild mode.\n");
		return 0;
	}

	if (read_mode) {

		printf("audio_in_create\n");
		ret = audio_in_create(44100, AUDIO_CHANNEL_STEREO, AUDIO_SAMPLE_TYPE_S16_LE, &input);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_in_create_ex failed. \n");
			return 0;
		}
		printf("audio_in_create success!!! [%p]\n", input);

		ret = audio_in_set_stream_cb(input, _audio_io_stream_read_cb, NULL);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_in_set_stream_cb failed. \n");
			goto EXIT;
		}
		printf("audio_in_set_stream_cb success!!! [%p]\n", input);

		ret = audio_in_set_state_changed_cb(input, _audio_in_state_cb, NULL);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_out_set_state_changed_cb failed. \n");
			goto EXIT;
		}
		printf("audio_out_set_state_changed_cb success!!! [%p]\n", input);

		ret = sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, focus_callback, NULL, &g_stream_info_read_h);
		if (ret) {
			printf("fail to sound_manager_create_stream_information(), ret(0x%x)\n", ret);
			goto EXIT;
		}
		ret = audio_in_set_stream_info(input, g_stream_info_read_h);
		if (ret)
			printf("fail to audio_in_set_stream_info(), ret(0x%x)\n", ret);

		ret = sound_manager_acquire_focus(g_stream_info_read_h, SOUND_STREAM_FOCUS_FOR_RECORDING, NULL);
		if (ret) {
			printf("fail to sound_manager_acquire_focus() for RECORDING, ret(0x%x)\n", ret);
			goto EXIT;
		}

		fp_w = fopen("/tmp/pcm_w.raw", "w");
	}

	if (write_mode) {
		printf("before audio_out_create\n");
		getchar();

		printf("audio_out_create\n");
		ret = audio_out_create_new(44100, AUDIO_CHANNEL_STEREO, AUDIO_SAMPLE_TYPE_S16_LE, &output);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_out_create failed. \n");
			goto EXIT;
		}
		printf("audio_out_create success!!! [%p]\n", output);

		ret = audio_out_set_stream_cb(output, _audio_io_stream_write_cb, NULL);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_out_set_stream_cb failed. \n");
			goto EXIT;
		}
		printf("audio_out_set_stream_cb success!!! [%p]\n", output);

		ret = audio_out_set_state_changed_cb(output, _audio_out_state_cb, NULL);
		if (ret != AUDIO_IO_ERROR_NONE) {
			printf("audio_out_set_state_changed_cb failed. \n");
			goto EXIT;
		}
		printf("audio_out_set_state_changed_cb success!!! [%p]\n", output);

		ret = sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, focus_callback, NULL, &g_stream_info_write_h);
		if (ret) {
			printf("fail to sound_manager_create_stream_information(), ret(0x%x)\n", ret);
			goto EXIT;
		}
		ret = audio_out_set_stream_info(output, g_stream_info_write_h);
		if (ret)
			printf("fail to audio_out_set_stream_info(), ret(0x%x)\n", ret);

		ret = sound_manager_acquire_focus(g_stream_info_write_h, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (ret) {
			printf("fail to sound_manager_acquire_focus() for PLAYBACK, ret(0x%x)\n", ret);
			goto EXIT;
		}

		/* generate wave data */
		for (i = 0; i < TABLE_SIZE; i++)
			test_wav.sine[i] = 0.9 * (float)sin(((double)i / (double)TABLE_SIZE) * M_PI * 2.);
		test_wav.left_channel = test_wav.right_channel = 0;
	}

	if (read_mode) {
		printf("before audio_in_prepare\n");
		getchar();
		printf("audio_in_prepare\n");
		ret = audio_in_prepare(input);
		if (ret != 0) {
			printf("audio_in_prepare failed.\n");
			audio_in_destroy(input);
			goto EXIT;
		} else {
			ret = audio_in_get_buffer_size(input, &size);
			if (ret != AUDIO_IO_ERROR_NONE) {
				printf("audio_in_get_buffer_size failed.\n");
				goto EXIT;
			} else {
				printf("size(%d)\n", size);
				buffer = alloca(size);
			}
		}

		if (buffer == NULL) {
			printf("buffer is null\n");
			goto EXIT;
		}
	}

	if (write_mode) {
		printf("before audio_out_prepare\n");
		getchar();
		printf("audio_out_prepare\n");
		ret = audio_out_prepare(output);
		if (ret != 0) {
			printf("audio_out_prepare failed.\n");
			audio_out_destroy(output);
			goto EXIT;
		}
	}

	do {
		int gotchar;
		printf("command(q:quit) : ");
		gotchar = getchar();
		if (gotchar == EOF)
			goto EXIT;
		if (cmd != '\n')
			getchar();
		cmd_ret = _convert_cmd_and_run(cmd, mode);
		printf("  - result code : %d\n", cmd_ret);
	} while (cmd != 'q');

EXIT:
	if (read_mode) {
		if (input) {
			printf("audio_in_unprepare\n");
			audio_in_unprepare(input);
			printf("audio_in_destroy\n");
			audio_in_destroy(input);
			input = NULL;
		}

		if (fp_w) {
			fclose(fp_w);
			fp_w = NULL;
		}

		if (g_stream_info_read_h) {
			ret = sound_manager_get_focus_state(g_stream_info_read_h, NULL, &recording_focus_state);
			if (recording_focus_state == SOUND_STREAM_FOCUS_STATE_ACQUIRED) {
				ret = sound_manager_release_focus(g_stream_info_read_h, SOUND_STREAM_FOCUS_FOR_RECORDING, NULL);
				if (ret)
					printf("fail to sound_manager_release_focus() for recording, ret(0x%x)\n", ret);
			}
			ret = sound_manager_destroy_stream_information(g_stream_info_read_h);
			if (ret)
				printf("fail to sound_manager_destroy_stream_information(), ret(0x%x)\n", ret);
			g_stream_info_read_h = NULL;
		}
	}

	if (write_mode) {
		if (output) {
			printf("audio_out_unprepare\n");
			audio_out_unprepare(output);
			printf("audio_out_destroy\n");
			audio_out_destroy(output);
		}

		if (g_stream_info_write_h) {
			ret = sound_manager_get_focus_state(g_stream_info_write_h, &playback_focus_state, NULL);
			if (playback_focus_state == SOUND_STREAM_FOCUS_STATE_ACQUIRED) {
				ret = sound_manager_release_focus(g_stream_info_write_h, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
				if (ret)
					printf("fail to sound_manager_release_focus() for playback, ret(0x%x)\n", ret);
			}
			ret = sound_manager_destroy_stream_information(g_stream_info_write_h);
			if (ret)
				printf("fail to sound_manager_destroy_stream_information(), ret(0x%x)\n", ret);
			g_stream_info_write_h = NULL;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc == 2 && !strcmp(argv[1], "call-forwarding-loop")) {
		audio_io_loopback_test();
	} else if (argc == 2 && !strcmp(argv[1], "call-forwarding-in")) {
		audio_io_loopback_in_test();
	} else if (argc == 3 && !strcmp(argv[1], "async")) {
		audio_io_async_test(atoi(argv[2]));
	} else if (argc == 4) {
		int channel_idx = atoi(argv[3]);
		if (channel_idx < 0 || channel_idx >2) {
			printf("Invalid channel\n");
			return 0;
		}
		printf("run with [%s][%s][%s]\n", argv[1], argv[2], argv[3]);
		audio_io_test(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	} else if (argc == 6) {
		play_file_sample(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	} else {
		printf("1. usage : audio_io_test call-forwarding-loop\n");
		printf("2. usage : audio_io_test call-forwarding-in\n");
		printf("3. usage : audio_io_test [length to read] [number of iteration] [channels]\n");
		printf("4. usage : audio_io_test async [write(1) | read(2)]\n");
		printf("5. Uasge : audio_io_test play [filename] [sample rate] [channels] [type(0:U8)]\n");
	}
	return 0;
}
