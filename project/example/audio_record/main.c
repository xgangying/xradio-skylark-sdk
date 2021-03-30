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
#include "common/framework/fs_ctrl.h"
#include "fs/fatfs/ff.h"
#include "common/apps/recorder_app.h"
#include "common/framework/platform_init.h"
#include "ExampleCustomerWriter.h"
#include "kernel/os/os_time.h"
#include "audio/pcm/audio_pcm.h"
#include "audio/manager/audio_manager.h"
#include "common/apps/player_app.h"
#include "audio/eq/eq.h"

//#include "audiofifo.h"

static player_base *player;

void drv_play_pwr_en(unsigned char en)
{
    GPIO_InitParam param;
    param.driving = GPIO_DRIVING_LEVEL_1;
    param.mode = GPIOx_Pn_F1_OUTPUT;
    param.pull = GPIO_PULL_NONE;

    HAL_GPIO_Init(GPIO_PORT_B, GPIO_PIN_16, &param);

    if (en)
    {
        HAL_GPIO_WritePin(GPIO_PORT_B, GPIO_PIN_16, GPIO_PIN_LOW);
    }
    else
    {
        HAL_GPIO_WritePin(GPIO_PORT_B, GPIO_PIN_16, GPIO_PIN_HIGH);
    }
}

int play_pcm_music_()
{
    FIL fp;
    int ret;
    FRESULT result;
    unsigned int act_read;
    unsigned int pcm_buf_size;
    void *pcm_buf;
    struct pcm_config config;

    //result = f_open(&fp, "record/audio.pcm", FA_OPEN_EXISTING | FA_READ);
    //if (result != FR_OK) {
        //printf("open pcm file fail\n");
        //return -1;
    //}

    config.channels = 1;
    config.format = PCM_FORMAT_S16_LE;
    config.period_count = 2;
    config.period_size = 1024;
    config.rate = 8000;
    ret = snd_pcm_open(AUDIO_SND_CARD_DEFAULT, PCM_OUT, &config);
    if (ret != 0) {
        goto err1;
    }
    printf("snd_pcm_open_ok\n");
    return 0;
    pcm_buf_size = config.channels * config.period_count * config.period_size;
    pcm_buf = malloc(pcm_buf_size);
    if (pcm_buf == NULL) {
        goto err2;
    }

    while (1) {
        result = f_read(&fp, pcm_buf, pcm_buf_size, &act_read);
        if (result != FR_OK) {
            printf("read fail.\n");
            break;
        }

        snd_pcm_write(AUDIO_SND_CARD_DEFAULT, pcm_buf, act_read);

        if (act_read != pcm_buf_size) {
            printf("reach file end\n");
            break;
        }
    }

    free(pcm_buf);
    snd_pcm_flush(AUDIO_SND_CARD_DEFAULT);
    snd_pcm_close(AUDIO_SND_CARD_DEFAULT, PCM_OUT);
    f_close(&fp);

    return 0;

err2:
    snd_pcm_close(AUDIO_SND_CARD_DEFAULT, PCM_OUT);
err1:
    f_close(&fp);
    return -1;
}


void play_pcm(void)
{
    //if (fs_ctrl_mount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
        //printf("mount fail\n");
        //goto exit;
    //}
    //drv_play_pwr_en(1);
    player = player_create();
    if (player == NULL) {
        printf("player create fail.\n");
        return;
    }

    printf("player create success.\n");
    printf("you can use it to play, pause, resume, set volume and so on.\n");

    printf("player set volume to 8. valid volume value is from 0~31\n");
    player->setvol(player, 30);

    //while (1) 
    {
        printf("===try to play pcm by audio driver===\n");
        play_pcm_music_();
    }

    //player_destroy(player);
}


static void cedarx_record(void)
{
	recorder_base *recorder;
	rec_cfg cfg;
	char music_url[64];
	CdxWriterT *writer;

	recorder = recorder_create();
	if (recorder == NULL) {
		printf("recorder create fail, exit\n");
		return;
	}

	/* record a 15s amr media */
	cfg.type = XRECODER_AUDIO_ENCODE_AMR_TYPE;
	printf("===start record amr now, last for 15s===\n");
	recorder->start(recorder, "file://record/1.amr", &cfg);
	OS_Sleep(120);
	recorder->stop(recorder);
	printf("record amr over.\n");

	/* record a 15s pcm media */
	cfg.type = XRECODER_AUDIO_ENCODE_PCM_TYPE;
	cfg.sample_rate = 8000;
	cfg.chan_num = 1;
	cfg.bitrate = 12200;
	cfg.sampler_bits = 16;
	printf("===start record pcm now, last for 15s===\n");
	recorder->start(recorder, "file://record/1.pcm", &cfg);
	OS_Sleep(60);
	recorder->stop(recorder);
	printf("record pcm over.\n");

	/* record a 15s amr media by customer writer */
	cfg.type = XRECODER_AUDIO_ENCODE_AMR_TYPE;
	printf("===start record amr by customer writer now, last for 15s===\n");
	writer = ExampleCustomerWriterCreat();
	if (writer == NULL) {
		goto exit;
	}
	sprintf(music_url, "customer://%p", writer);
	recorder->start(recorder, music_url, &cfg);
	OS_Sleep(60);
	recorder->stop(recorder);
	printf("record amr over.\n");

exit:
	recorder_destroy(recorder);
}

#define SAVE_RECORD_DATA_DURATION_MS   60000

static void audio_driver_record(void)
{
	int ret;
	FIL file;
	void *data;
	unsigned int len;
	//unsigned int writeLen;
	struct pcm_config config;
	unsigned int tick;
	unsigned int startTime;
	unsigned int nowTime;
    //uint8_t *buff;
    //int data_len;
	//f_unlink("record/audio.pcm");
	//f_open(&file, "record/audio.pcm", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);

	config.channels = 1;
	config.format = PCM_FORMAT_S16_LE;
	config.period_count = 2;
	config.period_size = 1024;
	config.rate = 8000;
	ret = snd_pcm_open(AUDIO_SND_CARD_DEFAULT, PCM_IN, &config);
	if (ret) {
		printf("snd_pcm_open fail.\n");
		goto err;
	}

	len = config.channels * config.period_count * config.period_size;
	data = malloc(len);
	if (data == NULL) {
		goto err1;
	}

	tick = OS_GetTicks();
	startTime = OS_TicksToMSecs(tick);

	printf("===start record pcm by audio driver, last for %dms===\n", SAVE_RECORD_DATA_DURATION_MS);
	while (1) {
		ret = snd_pcm_read(AUDIO_SND_CARD_DEFAULT, data, len);
		if (ret != len) {
			printf("snd_pcm_read fail.\n");
		}
        //printf("snd_pcm_data:len=%d\n", len);
        //buff = data;
        //for(data_len = 0; data_len < len; data_len++)
        //{
            //printf("%d ", buff[data_len]);
        //}

		//f_write(&file, data, len, &writeLen);
        snd_pcm_write(AUDIO_SND_CARD_DEFAULT, data, len);
		tick = OS_GetTicks();
		nowTime = OS_TicksToMSecs(tick);
		if ((nowTime - startTime) > SAVE_RECORD_DATA_DURATION_MS) {
			break;
		}
	}
	printf("record pcm over.\n");

	free(data);

err1:
	snd_pcm_close(AUDIO_SND_CARD_DEFAULT, PCM_IN);
err:
	f_close(&file);
	return;
}

int main(void)
{
	platform_init();

	//if (fs_ctrl_mount(FS_MNT_DEV_TYPE_SDCARD, 0) != 0) {
		//printf("mount fail\n");
		//return -1;
	//}

	printf("audio record start.\n");
    drv_play_pwr_en(1);
	/* set record volume */
	audio_manager_handler(AUDIO_SND_CARD_DEFAULT, AUDIO_MANAGER_SET_VOLUME_LEVEL, AUDIO_IN_DEV_AMIC, 3);

	//printf("start to use cedarx to record amr/pcm\n");
	//cedarx_record();
    play_pcm();

	printf("start to use audio driver to record pcm\n");
	audio_driver_record();
    
	printf("audio record over.\n");
	return 0;
}
