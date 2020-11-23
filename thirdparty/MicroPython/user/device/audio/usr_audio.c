#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <os_device.h>
#include <drv_cfg.h>
#include <vfs_posix.h>
#include <stdio.h>
#include "user_audio.h"
#include <audio/audio.h>

#define BUFSZ   			1024   
#define AUDIO_TASK_SIZE  	1024
#define AUDIO_TASK_PRIORITY	7

os_device_t *G_snd_dev = OS_NULL;

volatile int G_player_state = AUDIO_PLAYER_IDLE_CMD;

static void start_player(void);


static int audio_init(void *dev, uint16_t oflag)
{
	audio_dev_t *audio = (audio_dev_t *)dev;
	G_snd_dev = os_device_find(MICROPYTHON_DEVICE_AUDIO_NAME);
	if(G_snd_dev == OS_NULL){
		mp_raise_ValueError("Couldn't find deep device(audio0)! \n");
		return -1;
	}
	os_device_open(G_snd_dev, OS_DEVICE_OFLAG_WRONLY);
	
	audio->device->open_flag = AUDIO_INIT_FLAG;
	G_player_state = AUDIO_PLAYER_IDLE_CMD;
    return 0;
}

static int audio_deinit(void *dev)
{
	audio_dev_t *audio = (audio_dev_t *)dev;
	
	if (NULL == audio)
    {
        mp_raise_ValueError("audio device is NULL!\n");
    }
	audio->device->open_flag = AUDIO_DEINIT_FLAG;
	os_device_close(G_snd_dev);
	G_player_state= AUDIO_DEINIT_FLAG;
	return 0;
}



static void stop_player(void)
{
	G_player_state = AUDIO_PLAYER_STOP_CMD;
}

static void start_player(void)
{
	G_player_state = AUDIO_PLAYER_CONTINUE_CMD;
}

static int audio_player_task(void *data)
{
	if(G_snd_dev == OS_NULL || !data){
		mp_raise_ValueError("Couldn't operate deep device(audio0)! Please open deep!\n");
		return -1;
	}
		
	int fd = -1;
    uint8_t *buf = NULL;
    struct wav_info *info = NULL;
    struct os_audio_caps caps = {0};
    audio_dev_t *audio_file = data;
	os_device_t *snd_dev;

    fd = open(audio_file->file, O_RDONLY);
    if (fd < 0)
    {
        os_kprintf("failed to open %s file !\n", audio_file->file);
        goto __exit;
    }

    buf = os_malloc(BUFSZ);
    if (buf == OS_NULL)
        goto __exit;

    info = (struct wav_info *) os_malloc(sizeof * info);
    if (info == OS_NULL)
        goto __exit;

    if (read(fd, &(info->header), sizeof(struct RIFF_HEADER_DEF)) <= 0)    
        goto __exit;
    if (read(fd, &(info->fmt_block),  sizeof(struct FMT_BLOCK_DEF)) <= 0)
        goto __exit;
    if (read(fd, &(info->data_block), sizeof(struct DATA_BLOCK_DEF)) <= 0)
        goto __exit;

    os_kprintf("wav information:\n");
    os_kprintf("samplerate %d\n", info->fmt_block.wav_format.SamplesPerSec);
    os_kprintf("channel %d\n", info->fmt_block.wav_format.Channels);

	snd_dev = os_device_find("audio0");
    if(snd_dev == OS_NULL){
		mp_raise_ValueError("Couldn't operate deep device(audio0)! Please open deep!\n");
		return -1;
	}
	os_device_open(snd_dev, OS_DEVICE_OFLAG_WRONLY);
	
    /* parameter settings */                         
    caps.config_type = AUDIO_PARAM_CMD; 
    caps.udata.config.samplerate = info->fmt_block.wav_format.SamplesPerSec;    
    caps.udata.config.channels   = info->fmt_block.wav_format.Channels;                                                  
    os_device_control(snd_dev, AUDIO_CTL_CONFIGURE, &caps);      
    caps.config_type = AUDIO_VOLUME_CMD;
    caps.udata.value = audio_file->volume;
	
	/*save parameters*/
	audio_file->channels = caps.udata.config.channels;
	audio_file->samplerate = caps.udata.config.samplerate;
	
	
    os_device_control(snd_dev, AUDIO_CTL_CONFIGURE, &caps);                                    
	int length;
	start_player();
    while (1)
    {
		if (G_player_state == AUDIO_PLAYER_CONTINUE_CMD){
			length = read(fd, buf, BUFSZ);  
			if (length <= 0){
				break;
			}
			os_device_write(snd_dev, 0, buf, length);
		} else if(G_player_state == AUDIO_DEINIT_FLAG){
			break;
		} else {
			os_kprintf("G_player_state=%d\n", G_player_state);
			os_task_msleep(1);
		}
    }
	os_device_close(snd_dev);
__exit:

    if (fd >= 0)
        close(fd);

    if (buf)
        os_free(buf);

    if (info)
        os_free(info);
	G_player_state = AUDIO_PLAYER_IDLE_CMD;		// 方便下次播放
	return 0;
}

static void audio_player_entry(void *audio_file)
{
	if (!audio_player_task(audio_file))
	{
		//os_kprintf("player song failed!\n");
	}
}


static int audio_player(void *dev, uint32_t offset, void *buffer, uint32_t bufsize)
{	
	os_task_t  *audio_task;
	audio_dev_t *audio_file = (audio_dev_t *)dev;
	
	audio_file->file = buffer;
	if (G_player_state != AUDIO_PLAYER_IDLE_CMD){
		mp_raise_ValueError("player is busy!\n");
		return 0;
	}
	if (NULL == audio_file->device){
        mp_raise_ValueError("audio device is NULL!\n");
		return 0;
    }
	os_kprintf("file:%s", audio_file->file);
	audio_task = os_task_create("audio_player", audio_player_entry, audio_file, AUDIO_TASK_SIZE, AUDIO_TASK_PRIORITY, 10);
    if (audio_task != OS_NULL)
    {
        os_task_startup(audio_task);
    }
    else
    {
        os_kprintf("create audio_player task failed!\n");
    }

    return 0;
}

static int audio_ioctl(void *dev, int cmd, void *arg)
{

	struct os_audio_caps caps={0};
	
	audio_dev_t * audio = (audio_dev_t *)dev;

    if (NULL == audio)
    {
        mp_raise_ValueError("audio device is NULL \n");
        return -1;
    }
	if(G_snd_dev == OS_NULL){
		mp_raise_ValueError("Couldn't operate deep device(audio0)! Please open deep!\n");
		return -1;
	}
	
    switch(cmd)
    {	
		case AUDIO_WRITE_VOLUME_CMD: 
			caps.config_type = AUDIO_VOLUME_CMD;
			caps.udata.value = audio->volume;
			os_kprintf("volume %d\n", audio->volume);
			os_device_control(G_snd_dev, AUDIO_CTL_CONFIGURE, &caps);       
			return 0;
		case AUDIO_WRITE_PARAM_CMD:
		{
			caps.config_type = AUDIO_PARAM_CMD;
			caps.udata.config.samplerate = audio->samplerate;
			caps.udata.config.channels = audio->channels;
			os_kprintf("samplerate %d\n", caps.udata.config.samplerate);
			os_kprintf("channel %d\n", caps.udata.config.channels);
			os_device_control(G_snd_dev, AUDIO_CTL_CONFIGURE, &caps);
			return 0;
		}
		case AUDIO_PLAYER_STOP_CMD:
			stop_player();
			return 0;
		
		case AUDIO_PLAYER_CONTINUE_CMD:
			G_player_state = AUDIO_PLAYER_CONTINUE_CMD;
			return 0;
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}



static int audio_register(void)
{
	device_info_t * audio = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == audio)
	{
		os_kprintf("mpycall_audio_register malloc mem failed!");
		return -1;
	}
    memset(audio, 0, sizeof(device_info_t));
	

	audio->owner.name = "audio";
	audio->owner.type = DEV_AUDIO;
	
	audio->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == audio->ops)
	{
		os_kprintf("mpycall_audio_register malloc mem failed!"); 
		return -1;
	}
    memset(audio->ops, 0, sizeof(struct operate));
	
	audio->ops->open =  audio_init;
	audio->ops->ioctl = audio_ioctl;
	audio->ops->write = audio_player;
	audio->ops->close = audio_deinit;
    

	mpycall_device_add(audio);

	return 0;
}

OS_CMPOENT_INIT(audio_register);




