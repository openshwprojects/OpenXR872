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

#ifdef __CONFIG_XPLAYER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cedarx/xplayer/include/xplayer.h"
#include "driver/chip/hal_snd_card.h"
#include "audio/manager/audio_manager.h"
#include "kernel/os/os.h"
#include "util/atomic.h"
#include "sys/defs.h"
#include "common/framework/sys_ctrl/sys_ctrl.h"
#include "player_app.h"

#define USER_AGENT "Mozilla/5.0 (FreeRTOS; OS 8.2.3) Xradio Xradio/1.0"

#define PLAYER_LOGD(msg, arg...)      //printf("[PLAYER_DBG] <%s : %d> " msg "\n", __func__, __LINE__, ##arg)
#define PLAYER_LOGI(msg, arg...)      printf("[PLAYER_INFO] <%s : %d> " msg "\n", __func__, __LINE__, ##arg)
#define PLAYER_LOGW(msg, arg...)      printf("[PLAYER_WRN] <%s : %d> " msg "\n", __func__, __LINE__, ##arg)
#define PLAYER_LOGE(msg, arg...)      printf("[PLAYER_ERR] <%s : %d> " msg "\n", __func__, __LINE__, ##arg)

#define APLAYER_ID_AND_STATE(id, state)     (((id) << 16) | (state))
#define APLAYER_GET_ID(id_state)            (((id_state) >> 16) & 0xFFFF)
#define APLAYER_GET_STATE(id_state)         ((id_state) & 0xFFFF)

typedef struct player_info {
	char *url;
	uint32_t size;
} player_info;

typedef struct app_player {
	player_base base;
	XPlayer *xplayer;
	SoundCtrl *sound;
	CdxKeyedVectorT *pHeaders;
	aplayer_states state;
	app_player_callback cb;
	player_info info;
	OS_Mutex_t lock;
	uint8_t mute;
	int vol;
	uint16_t id;
	void *arg;
} app_player;

struct player_handler_param {
	app_player *impl;
	uint32_t id_state;
};

static void player_handler(event_msg *msg);

static void *wrap_realloc(void *p, uint32_t *osize, uint32_t nsize)
{
	if (p == NULL) {
		*osize = nsize;
		return malloc(nsize);
	}
	if (*osize >= nsize)
		return p;
	free(p);
	PLAYER_LOGD("free %d, malloc %d", *osize, nsize);
	*osize = nsize;
	return malloc(nsize);
}

static int set_url(app_player *impl, char *pUrl)
{
	struct player_handler_param *handler_param;

	/* set url to the AwPlayer. */
	if (XPlayerSetDataSourceUrl(impl->xplayer, (const char *)pUrl, NULL, impl->pHeaders) != 0) {
		PLAYER_LOGE("setDataSource() return fail.");
		return -1;
	}

	if ((!strncmp(pUrl, "http://", 7)) || (!strncmp(pUrl, "https://", 8)) ||
	    (!strncmp(pUrl, "fifo://", 7))) {
		if (XPlayerPrepareAsync(impl->xplayer) != 0) {
			PLAYER_LOGE("prepareAsync() return fail.");
			return -1;
		}
	} else {
		handler_param = malloc(sizeof(*handler_param));
		if (handler_param == NULL) {
			PLAYER_LOGE("memory malloc fail.");
			return -1;
		}
		handler_param->impl = impl;
		handler_param->id_state = APLAYER_ID_AND_STATE(impl->id, PLAYER_EVENTS_MEDIA_PREPARED);
		sys_handler_send(player_handler, (uint32_t)handler_param, 10000);
	}
	return 0;
}

static int play(app_player *impl)
{
	if (XPlayerStart(impl->xplayer) != 0) {
		PLAYER_LOGE("start() return fail.");
		return -1;
	}
	PLAYER_LOGD("playing");
	return 0;
}

static int reset(app_player *impl)
{
	if (XPlayerReset(impl->xplayer) != 0) {
		PLAYER_LOGE("reset() return fail.");
		return -1;
	}
	impl->id++;
	return 0;
}

static void player_handler_preprocess(app_player *impl, player_events evt)
{
	PLAYER_LOGI("event: %d", (int)evt);

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);

	switch (evt) {
	case PLAYER_EVENTS_MEDIA_PREPARED:
		impl->cb(evt, NULL, impl->arg);
		if (impl->state != APLAYER_STATES_PAUSE) {
			play(impl);
		}
		impl->state = APLAYER_STATES_PLAYING;

		break;
	case PLAYER_EVENTS_MEDIA_PLAYBACK_COMPLETE:
	case PLAYER_EVENTS_MEDIA_ERROR:
		reset(impl);
		impl->state = APLAYER_STATES_STOPPED;
		break;
	default:
		break;
	}

	OS_RecursiveMutexUnlock(&impl->lock);

	if (evt != PLAYER_EVENTS_MEDIA_PREPARED) {
		impl->cb(evt, NULL, impl->arg);
	}

	return;
}

static void player_handler(event_msg *msg)
{
	struct player_handler_param *handler_param;
	app_player *impl;
	player_events state;
	uint16_t id;

	handler_param = (struct player_handler_param *)msg->data;
	impl = handler_param->impl;
	state = APLAYER_GET_STATE(handler_param->id_state);
	id = APLAYER_GET_ID(handler_param->id_state);
	free((void *)msg->data);

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	if (id != impl->id) {
		OS_RecursiveMutexUnlock(&impl->lock);
		return;
	}
	OS_RecursiveMutexUnlock(&impl->lock);

	player_handler_preprocess(impl, state);
}

static int player_callback(void *pUserData, int msg, int ext1, void *param)
{
	app_player *impl = (app_player *)pUserData;
	struct player_handler_param *handler_param;

	switch (msg) {
	case AWPLAYER_MEDIA_INFO:
		switch (ext1) {
		case AW_MEDIA_INFO_NOT_SEEKABLE:
			PLAYER_LOGI("media source is unseekable.");
			break;
		default:
			break;
		}
		break;

	case AWPLAYER_MEDIA_ERROR:
		PLAYER_LOGW("open media source fail.\n"
		        "reason: maybe the network is bad, or the music file is not good.");
		handler_param = malloc(sizeof(*handler_param));
		if (handler_param == NULL) {
			PLAYER_LOGE("memory malloc fail.");
			break;
		}
		handler_param->impl = impl;
		handler_param->id_state = APLAYER_ID_AND_STATE(impl->id, PLAYER_EVENTS_MEDIA_ERROR);
		sys_handler_send(player_handler, (uint32_t)handler_param, 10000);
		break;

	case AWPLAYER_MEDIA_PREPARED:
		handler_param = malloc(sizeof(*handler_param));
		if (handler_param == NULL) {
			PLAYER_LOGE("memory malloc fail.");
			break;
		}
		handler_param->impl = impl;
		handler_param->id_state = APLAYER_ID_AND_STATE(impl->id, PLAYER_EVENTS_MEDIA_PREPARED);
		sys_handler_send(player_handler, (uint32_t)handler_param, 10000);
		break;

	case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
		PLAYER_LOGI("playback complete.");
		handler_param = malloc(sizeof(*handler_param));
		if (handler_param == NULL) {
			PLAYER_LOGE("memory malloc fail.");
			break;
		}
		handler_param->impl = impl;
		handler_param->id_state = APLAYER_ID_AND_STATE(impl->id, PLAYER_EVENTS_MEDIA_PLAYBACK_COMPLETE);
		sys_handler_send(player_handler, (uint32_t)handler_param, 10000);
		break;

	case AWPLAYER_MEDIA_SEEK_COMPLETE:
		PLAYER_LOGI("seek ok.");
		break;

	default:
		PLAYER_LOGD("unknown callback from AwPlayer");
		break;
	}

	PLAYER_LOGI("cedarx cb complete.");
	return 0;
}

static int player_stop(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	struct player_handler_param *handler_param;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	reset(impl);
	impl->state = APLAYER_STATES_STOPPED;

	handler_param = malloc(sizeof(*handler_param));
	if (handler_param == NULL) {
		PLAYER_LOGE("memory malloc fail.");
		OS_RecursiveMutexUnlock(&impl->lock);
		return -1;
	}
	handler_param->impl = impl;
	handler_param->id_state = APLAYER_ID_AND_STATE(impl->id, PLAYER_EVENTS_MEDIA_STOPPED);
	sys_handler_send(player_handler, (uint32_t)handler_param, 10000);

	OS_RecursiveMutexUnlock(&impl->lock);
	OS_MSleep(10);

	return 0;
}

static int player_pause(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	int ret = 0;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	ret = XPlayerPause(impl->xplayer);
	impl->state = APLAYER_STATES_PAUSE;
	OS_RecursiveMutexUnlock(&impl->lock);

	return ret;
}

static int player_resume(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	int ret = 0;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	ret = play(impl);
	impl->state = APLAYER_STATES_PLAYING;
	OS_RecursiveMutexUnlock(&impl->lock);

	return ret;
}

static int player_seek(player_base *base, int ms)
{
	app_player *impl = container_of(base, app_player, base);
	int ret = 0;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	if (XPlayerSeekTo(impl->xplayer, ms) != 0) {
		PLAYER_LOGE("seek() return fail.");
		OS_RecursiveMutexUnlock(&impl->lock);
		return -1;
	}
	PLAYER_LOGI("seek to %d ms.", ms);
	OS_RecursiveMutexUnlock(&impl->lock);

	return ret;
}

static int player_seturl(player_base *base, const char *url)
{
	int ret = 0;
	app_player *impl = container_of(base, app_player, base);
	char *play_url = (char *)url;

	if (url == NULL)
		return -1;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);

	impl->state = APLAYER_STATES_PREPARING;

	PLAYER_LOGI("request to play : %s", url);

	impl->info.url = wrap_realloc(impl->info.url, &impl->info.size, strlen(play_url) + 1);
	memcpy(impl->info.url, play_url, strlen(play_url) + 1);
	ret = set_url(impl, impl->info.url);
	if (ret) {
		reset(impl);
		impl->state = APLAYER_STATES_STOPPED;
	}
	OS_RecursiveMutexUnlock(&impl->lock);

	return ret;
}

static int player_tell(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	int ms;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	if (XPlayerGetCurrentPosition(impl->xplayer, &ms) != 0) {
		PLAYER_LOGW("tell() return fail.");
		OS_RecursiveMutexUnlock(&impl->lock);
		return 0;
	}
	PLAYER_LOGI("tell to %d ms.", ms);
	OS_RecursiveMutexUnlock(&impl->lock);
	return ms;
}

static int player_size(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	int ms;

	OS_RecursiveMutexLock(&impl->lock, OS_WAIT_FOREVER);
	if (XPlayerGetDuration(impl->xplayer, &ms) != 0) {
		PLAYER_LOGW("size() return fail.");
		OS_RecursiveMutexUnlock(&impl->lock);
		return 0;
	}
	PLAYER_LOGI("size to %d ms.", ms);
	OS_RecursiveMutexUnlock(&impl->lock);
	return ms;
}

static int player_setvol(player_base *base, int vol)
{
	app_player *impl = container_of(base, app_player, base);

	if (vol > 31) {
		PLAYER_LOGW("set vol %d larger than 31", vol);
		vol = 31;
	} else if (vol < 0) {
		PLAYER_LOGW("set vol %d lesser than 0", vol);
		vol = 0;
	}

	impl->vol = vol;
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_OUT_DEV_SPK, vol);
	return 0;
}

static int player_getvol(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	return impl->vol;
}

static int player_mute(player_base *base, bool is_mute)
{
	app_player *impl = container_of(base, app_player, base);
	impl->mute = (uint8_t)is_mute;
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_MUTE, AUDIO_OUT_DEV_SPK, is_mute);
	return 0;
}

static int player_is_mute(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);
	return impl->mute;
}

static aplayer_states player_get_states(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);

	return impl->state;
}

static int player_control(player_base *base, player_cmd command, void *data)
{
	app_player *impl = container_of(base, app_player, base);
	switch (command) {
	case PLAYER_CMD_SET_OUTPUT_CONFIG:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_SET_OUTPUT_CONFIG, data);
		break;
	case PLAYER_CMD_ADD_OUTPUT_CONFIG:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_ADD_OUTPUT_CONFIG, data);
		break;
	case PLAYER_CMD_CLEAR_OUTPUT_CONFIG:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_CLEAR_OUTPUT_CONFIG, data);
		break;
	case PLAYER_CMD_SET_EQ_MODE:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_SET_EQ_MODE, data);
		break;
	case PLAYER_CMD_CLEAR_EQ_MODE:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_CLEAR_EQ_MODE, data);
		break;
	case PLAYER_CMD_SET_DRC_MODE:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_SET_DRC_MODE, data);
		break;
	case PLAYER_CMD_CLEAR_DRC_MODE:
		SoundDeviceControl(impl->sound, SOUND_CONTROL_CLEAR_DRC_MODE, data);
		break;
	default:
		break;
	}
	return 0;
}

static void player_null_callback(player_events event, void *data, void *arg)
{
	PLAYER_LOGI("cb event:%d", event);
}

static void player_setcb(player_base *base, app_player_callback cb, void *arg)
{
	app_player *impl = container_of(base, app_player, base);
	if (!cb)
		impl->cb = player_null_callback;
	else
		impl->cb = cb;
	impl->arg = arg;
}

player_base *player_create()
{
	OS_Status status;
	app_player *impl = NULL;
#if (__CONFIG_CEDARX_HEAP_MODE == 1)
	XPlayerBufferConfig xplayerConfig;
	HttpStreamBufferConfig httpConfig;
#endif

	impl = malloc(sizeof(*impl));
	if (impl == NULL) {
		return NULL;
	}
	memset(impl, 0, sizeof(*impl));

	impl->xplayer = XPlayerCreate();
	if (impl->xplayer == NULL) {
		goto err0;
	}

#if (__CONFIG_CEDARX_HEAP_MODE == 1)
	memset(&xplayerConfig, 0, sizeof(XPlayerBufferConfig));
	xplayerConfig.maxMovStcoBufferSize = 8 * 1024;
	xplayerConfig.maxMovStszBufferSize = 64 * 1024;
	XPlayerSetBuffer(impl->xplayer, &xplayerConfig);

	memset(&httpConfig, 0, sizeof(HttpStreamBufferConfig));
	httpConfig.maxBufferSize = (128 * 1024);
	httpConfig.maxProtectAreaSize = (32 * 1024);
	XPlayerSetHttpBuffer(impl->xplayer, &httpConfig);
#endif

	/* set callback to player. */
	XPlayerSetNotifyCallback(impl->xplayer, player_callback, (void *)impl);

	/* check if the player work. */
	if (XPlayerInitCheck(impl->xplayer) != 0) {
		goto err1;
	}

	SoundCtrl *SoundDeviceCreate();
	impl->sound = SoundDeviceCreate();
	if (impl->sound == NULL) {
		goto err1;
	}
	XPlayerSetAudioSink(impl->xplayer, (void *)impl->sound);

	impl->pHeaders = malloc(sizeof(CdxKeyedVectorT) + 1 * sizeof(KeyValuePairT));
	if (impl->pHeaders) {
		impl->pHeaders->size = 1;
		impl->pHeaders->item[0].key = "User-Agent";
		impl->pHeaders->item[0].val = USER_AGENT;
	}

	impl->base.play         = player_seturl;
	impl->base.stop         = player_stop;
	impl->base.pause        = player_pause;
	impl->base.resume       = player_resume;
	impl->base.seek         = player_seek;
	impl->base.tell         = player_tell;
	impl->base.size         = player_size;
	impl->base.setvol       = player_setvol;
	impl->base.getvol       = player_getvol;
	impl->base.mute         = player_mute;
	impl->base.is_mute      = player_is_mute;
	impl->base.control      = player_control;
	impl->base.set_callback = player_setcb;
	impl->base.get_status   = player_get_states;
	impl->cb                = player_null_callback;

	status = OS_RecursiveMutexCreate(&impl->lock);
	if (status != OS_OK) {
		goto err1;
	}

	impl->state = APLAYER_STATES_INIT;

	return &impl->base;

	PLAYER_LOGE("create player failed, quit.");
err1:
	/* it will destroy SoundDevice too */
	XPlayerDestroy(impl->xplayer);
	free(impl->pHeaders);
err0:
	free(impl);
	return NULL;
}

void player_destroy(player_base *base)
{
	app_player *impl = container_of(base, app_player, base);

	PLAYER_LOGI("destroy AwPlayer.");
	player_stop(base);

	OS_RecursiveMutexDelete(&impl->lock);
	if (impl->xplayer != NULL)
		XPlayerDestroy(impl->xplayer);
	if (impl->info.url)
		free(impl->info.url);
	if (impl->pHeaders)
		free(impl->pHeaders);
	if (impl != NULL)
		free(impl);
}

#endif

