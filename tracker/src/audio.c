#include "audio.h"
#include <libchip.h>
#include <stdio.h>

#define AUDIO_RATE 44100
#define AUDIO_BUFFER_LEN 64
#define AUDIO_NUM_BUFFERS 32
#define AUDIO_OVERSAMPLE 128
#define AUDIO_CHANNEL_COUNT 8

static int audio_is_init;
static int audio_rate;
static int audio_buffer_len;
static int audio_num_buffers;
static int audio_oversample;
static int audio_channel_count;

void audio_init(void)
{
	if (audio_is_init)
	{
		fprintf(stderr, "[%s]: Audio is already init!\n",__PRETTY_FUNCTION__);
	}
	// Set configuration to defaults if user has not specified preferences
	if (!audio_rate)
	{
		audio_rate = AUDIO_RATE;
	}
	if (!audio_buffer_len)
	{
		audio_buffer_len = AUDIO_BUFFER_LEN;
	}
	if (!audio_num_buffers)
	{
		audio_num_buffers = AUDIO_NUM_BUFFERS;
	}
	if (!audio_oversample)
	{
		audio_oversample = AUDIO_OVERSAMPLE;
	}
	if (!audio_channel_count)
	{
		audio_channel_count = AUDIO_CHANNEL_COUNT;
	}
	printf("[%s]: Initializing audio:\n", __PRETTY_FUNCTION__);
	printf("\tSampling rate: %dHz\n", audio_rate);
	printf("\tBuffer length: %d samples\n", audio_buffer_len);
	printf("\tBuffer count: %d buffers\n", audio_num_buffers);
	printf("\tOversample: %d\n", audio_oversample);
	printf("\t%d channels\n\n", audio_channel_count);

	// Set up libchip
	chip_init(audio_rate,
	          audio_channel_count, 
	          audio_buffer_len, 
	          audio_num_buffers, 
	          audio_oversample);

	audio_is_init = 1;

	// libchip is set up; begin audio thread
	chip_start();
}

void audio_shutdown(void)
{
	chip_shutdown();
}

// Boilerplate
void audio_set_rate(unsigned int r)
{
	audio_rate = r;
}

void audio_set_buffer_len(unsigned int b)
{
	audio_buffer_len = b;
}

void audio_set_num_buffers(unsigned int n)
{
	audio_num_buffers = n;
}

void audio_set_oversample(unsigned int o)
{
	audio_oversample = o;
}

void audio_set_channel_count(unsigned int c)
{
	audio_channel_count = c;
}

unsigned int audio_get_rate(void)
{
	return audio_rate;
}

unsigned int audio_get_buffer_len(void)
{
	return audio_buffer_len;
}

unsigned int audio_get_num_buffers(void)
{
	return audio_num_buffers;
}

unsigned int audio_get_oversample(void)
{
	return audio_oversample;
}

unsigned int audio_get_channel_count(void)
{
	return audio_channel_count;
}

