/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "kernel/os/os.h"
#include "audio_player.h"
#include "audio/manager/audio_manager.h"
#include "play_list.h"
#include "common/apps/player_app.h"

#define AUDIO_LOGE(fmt, arg...)  printf("[AUDIO_ERR][F:%s][L:%d] " fmt, __func__, __LINE__, ##arg)
#define AUDIO_LOGW(fmt, arg...)  printf("[AUDIO_WRN][F:%s][L:%d] " fmt, __func__, __LINE__, ##arg)
#define AUDIO_LOGI(fmt, arg...)  printf("[AUDIO_INFO][F:%s][L:%d] " fmt, __func__, __LINE__, ##arg)
#define AUDIO_LOGD(fmt, arg...)  printf("[AUDIO_DBG][F:%s][L:%d] " fmt, __func__, __LINE__, ##arg)
#define AUDIO_LOGT(fmt, arg...)  //printf("[AUDIO_TRC][F:%s][L:%d] " fmt, __func__, __LINE__, ##arg)

#define BUTTON_CTRL_ENABLE    1

#if BUTTON_CTRL_ENABLE
#include "audio_buttons.h"
#endif

#define PLAYER_TASK_SLEEP_DURATION_MS   100

#define PLAYER_THREAD_STACK_SIZE    (1024 * 3)

typedef enum {
	PLAYER_PLAYING,
	PLAYER_PAUSED,
} PLAYER_STATUS;

typedef struct DemoPlayerContext {
	player_base *mbase;
} DemoPlayerContext;

static DemoPlayerContext demoPlayer;
static uint8_t play_song_flag;

static OS_Thread_t player_task_thread;
static uint8_t player_task_run;

static void audio_player_callback(player_events event, void *data, void *arg)
{
	AUDIO_LOGD("player event:%d\n", event);

	switch (event) {
	case PLAYER_EVENTS_MEDIA_PREPARED:
		break;
	case PLAYER_EVENTS_MEDIA_STOPPED:
		break;
	case PLAYER_EVENTS_MEDIA_ERROR:
		break;
	case PLAYER_EVENTS_MEDIA_PLAYBACK_COMPLETE:
		play_song_flag = 0;
		break;
	default:
		break;
	}
}

static int audio_player_init(void)
{
	int ret;

	ret = play_list_init();
	if (ret != 0) {
		return -1;
	}

	memset(&demoPlayer, 0, sizeof(DemoPlayerContext));

	demoPlayer.mbase = player_create();
	if (demoPlayer.mbase == NULL) {
		AUDIO_LOGE("can not create Player, quit.\n");
		return -1;
	}

	AUDIO_LOGD("create player success\n");
	return 0;
}

static void audio_player_deinit(void)
{
	if (demoPlayer.mbase != NULL) {
		player_destroy(demoPlayer.mbase);
		demoPlayer.mbase = NULL;
	}

	play_list_deinit();
}

static void play_songs(char *song_name)
{
	demoPlayer.mbase->stop(demoPlayer.mbase);
	demoPlayer.mbase->set_callback(demoPlayer.mbase, audio_player_callback, NULL);
	play_song_flag = 1;
	demoPlayer.mbase->play(demoPlayer.mbase, song_name);
}

static void player_pause(void)
{
	demoPlayer.mbase->pause(demoPlayer.mbase);
}

static void player_resume(void)
{
	demoPlayer.mbase->resume(demoPlayer.mbase);
}

static void player_volume_ctrl(int volume)
{
	int current_dev = audio_manager_get_current_dev(AUDIO_SND_CARD_DEFAULT);
	if (current_dev < 0) {
		AUDIO_LOGE("Invalid snd card num [%d]\n", AUDIO_SND_CARD_DEFAULT);
		return;
	}

	if (current_dev & AUDIO_OUT_DEV_SPK) {
		audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_OUT_DEV_SPK, volume);
	} else if (current_dev & AUDIO_OUT_DEV_HP) {
		audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_OUT_DEV_HP, volume);
	} else { /* no current dev before opening audio driver, so we set all device */
		audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_OUT_DEV_SPK, volume);
		//audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_OUT_DEV_HP, volume);
	}
}

#if BUTTON_CTRL_ENABLE

PLAYER_CMD player_button_cmd = CMD_PLAYER_NULL;

static PLAYER_CMD player_button_ctrl_cmd(void)
{
	PLAYER_CMD cmd = CMD_PLAYER_NULL;

	if (player_button_cmd != CMD_PLAYER_NULL) {
		cmd = player_button_cmd;
		player_button_cmd = CMD_PLAYER_NULL;
	}

	return cmd;
}

#endif

static PLAYER_CMD player_get_ctrl_cmd(void)
{
#if BUTTON_CTRL_ENABLE
	PLAYER_CMD cmd = player_button_ctrl_cmd();
	if (cmd != CMD_PLAYER_NULL) {
		return cmd;
	}
#endif

	if (!play_song_flag) {
		return CMD_PLAYER_NEXT;
	}
	return CMD_PLAYER_NULL;
}

static void player_task(void *arg)
{
	int ret;
	int volume = 8;
	char read_songs_buf[255];
	PLAYER_STATUS pause_ctrl = PLAYER_PLAYING;

	ret = audio_player_init();
	if (ret) {
		goto exit;
	}
	player_volume_ctrl(volume);

	while (player_task_run) {
		PLAYER_CMD cmd = player_get_ctrl_cmd();
		switch (cmd) {
		case CMD_PLAYER_NEXT:
			pause_ctrl = PLAYER_PLAYING;
			player_read_song(PLAYER_NEXT, read_songs_buf);
			play_songs(read_songs_buf);
			break;
		case CMD_PLAYER_PERV:
			pause_ctrl = PLAYER_PLAYING;
			player_read_song(PLAYER_PREV, read_songs_buf);
			play_songs(read_songs_buf);
			break;
		case CMD_PLAYER_VOLUME_UP:
			volume++;
			if (volume > VOLUME_MAX_LEVEL) {
				volume = VOLUME_MAX_LEVEL;
			}

			player_volume_ctrl(volume);
			break;
		case CMD_PLAYER_VOLUME_DOWN:
			volume--;
			if (volume < 0) {
				volume = 0;
			}

			player_volume_ctrl(volume);
			break;
		case CMD_PLAYER_PAUSE:
			if (pause_ctrl == PLAYER_PLAYING) {
				pause_ctrl = PLAYER_PAUSED;
				player_pause();
			} else {
				pause_ctrl = PLAYER_PLAYING;
				player_resume();
			}
			break;
		default:
			break;
		}

		OS_MSleep(PLAYER_TASK_SLEEP_DURATION_MS);
	}

	audio_player_deinit();

exit:
	OS_ThreadDelete(&player_task_thread);
}

int player_task_init(void)
{
	player_task_run = 1;
	if (OS_ThreadCreate(&player_task_thread,
	                    "player_task",
	                    player_task,
	                    NULL,
	                    OS_THREAD_PRIO_APP,
	                    PLAYER_THREAD_STACK_SIZE) != OS_OK) {
		AUDIO_LOGE("thread create error\n");
		return -1;
	}

#if BUTTON_CTRL_ENABLE
	audio_buttons_init();
#endif

	return 0;
}

int player_task_deinit(void)
{
	player_task_run = 0;
	while ((OS_ThreadIsValid(&player_task_thread)))
		OS_MSleep(10);

#if BUTTON_CTRL_ENABLE
	audio_buttons_deinit();
#endif

	return 0;
}

