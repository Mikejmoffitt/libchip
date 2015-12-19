#include "libchip.h"
#include "chipkernel.h"

// User functions
void chip_shutdown(void)
{	
	chip_num_channels = 0;
	chip_is_init = 0;
	if (chip_thread)
	{
		al_set_thread_should_stop(chip_thread);
		al_destroy_thread(chip_thread);
		chip_thread = NULL;
	}
	if (chip_queue)
	{
		al_destroy_event_queue(chip_queue);
		chip_queue = NULL;
	}
	if (chip_stream)
	{
		al_destroy_audio_stream(chip_stream);
		chip_stream = NULL;
	}
	if (chip_voice)
	{
		al_destroy_voice(chip_voice);
		chip_voice = NULL;
	}
	if (chip_mixer)
	{
		al_destroy_mixer(chip_mixer);
		chip_mixer = NULL;
	}
	if (chip_channels)
	{
		for (unsigned int i = 0; i < chip_num_channels; i++)
		{
			chip_channel *ch = &chip_channels[i];
			al_unlock_mutex(ch->mutex);
			// Release waves if the channel owns it
			if (ch->own_wave)
			{
				free(ch->wave_data);
				al_destroy_mutex(ch->mutex);
			}
		}
		free(chip_channels);
		chip_channels = NULL;
	}
}

static int chip_allegro_setup(void)
{
	if (!al_is_system_installed())
	{
		if (!al_init())
		{
			fprintf(stderr,"[audio] Error: Could not initialize Allegro.\n");
			return 0;
		}
	}
	printf("[audio] Allegro is installed\n");
	if (!al_is_audio_installed())
	{
	
		if (!al_install_audio())
		{
			fprintf(stderr,"[audio] Error: Could not install audio addon.\n");
			return 0;
		}
	}
	printf("[audio] Audio addon is installed\n");
	// Voice
	chip_voice = al_create_voice(chip_rate,
		CHIP_DEPTH,
		CHIP_CHAN);
	if (!chip_voice)
	{
		fprintf(stderr,"[audio] Error: Failed to create voice.\n");
		return 0;
	}
	printf("[audio] Created voice at %X\n",(unsigned int)chip_voice);

	// Mixer
	chip_mixer = al_create_mixer(chip_rate,
		CHIP_DEPTH,
		CHIP_CHAN);
	if (!chip_mixer)
	{
		fprintf(stderr,"[audio] Error: Failed to create mixer.\n");
		return 0;
	}
	printf("[audio] Created mixer at %X\n",(unsigned int)chip_mixer);

	if (!al_attach_mixer_to_voice(chip_mixer, chip_voice))
	{
		fprintf(stderr,"[audio] Error: Failed to attach mixer to voice.\n");
		return 0;
	}
	printf("[audio] Attached mixer to voice\n");

	al_set_default_mixer(chip_mixer);
	al_reserve_samples(chip_frag_num);

	// Build stream
	chip_stream = al_create_audio_stream(
		chip_frag_num,
		chip_frag_size,
		chip_rate,
		CHIP_DEPTH,
		CHIP_CHAN);
	printf("[audio] Created stream at %X\n",(unsigned int)chip_stream);
	if (!al_attach_audio_stream_to_mixer(chip_stream, al_get_default_mixer()))
	{
		printf("[audio] Error: Couldn't attach stream to mixer.\n");
		return 0;
	}
	printf("[audio] Attached stream to mixer.\n");

	// Set up event source for the audio thread
	chip_queue = al_create_event_queue();
	printf("[audio] Created queue at %X\n",(unsigned int)chip_queue);
	al_register_event_source(chip_queue, 
		al_get_audio_stream_event_source(chip_stream));
	printf("[audio] Registered audio event source with queue.\n");

	return 1;

}

static int chip_arg_sanity(void)
{
	if (!chip_rate)
	{
		fprintf(stderr,"[audio] Error: Invalid sample rate specified.\n");
		return 0;
	}
	printf("[audio] Sampling rate: %dHz\n",chip_rate);
	if (!chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: At least one channel must be created.\n");
		return 0;
	}
	printf("[audio] Using %d channels\n",chip_num_channels);
	if (!chip_frag_size)
	{
		fprintf(stderr,"[audio] Warning: No fragment size given. Defaulting to 1024.\n");
		chip_frag_size = CHIP_SIZE_FRAGMENT;
	}
	printf("[audio] Using %d for fragment size\n",chip_frag_size);
	if (!chip_frag_num)
	{
		fprintf(stderr,"[audio] Warning: No fragment number given. Defaulting to 4.\n");
		chip_frag_num = CHIP_NUM_FRAGMENTS;
	}
	printf("[audio] Using %d fragments\n",chip_frag_num);
	if (!chip_rate_mul)
	{
		chip_rate_mul = 1;
	}
	printf("[audio] Rate multiplier is %d\n",chip_rate_mul);
	return 1;
}

static int chip_channel_init(void)
{
	// Set up channel state
	chip_channels = (chip_channel *)calloc(chip_num_channels,sizeof(chip_channel));
	if (!chip_channels)
	{
		fprintf(stderr,"[audio] Couldn't malloc for channel states. Maybe too many have been requested?\n");
		return 0;
	}
	for (int i = 0; i < chip_num_channels; i++)
	{
		chip_channel *ch = &chip_channels[i];
		ch->period = 1;
		ch->mutex = al_create_mutex();
		ch->wave_data = (uint16_t *)malloc(sizeof(uint16_t));
		ch->wave_len = 1;
		ch->own_wave = 1;
		ch->noise_tap = 7;
		ch->noise_state = 0x0001;
		al_unlock_mutex(ch->mutex);
	}

	printf("[audio] Created channel states at %X\n",(uint16_t)chip_channels);
	return 1;
}

void chip_init(unsigned int rate, unsigned int num_channels, unsigned int frag_size, unsigned int frag_num, unsigned int rate_mul)
{
	chip_shutdown();

	chip_rate = rate;
	chip_num_channels = num_channels;
	chip_frag_size = frag_size;
	chip_frag_num = frag_num;
	chip_rate_mul = rate_mul;
	
	if (!chip_arg_sanity())
	{
		return;
	}
	if (!chip_allegro_setup())
	{
		return;
	}
	if (!chip_channel_init())
	{
		return;
	}

	// Set up defaults for audio engine pointer
	chip_engine_ptr = NULL;
	chip_engine_cnt = 0;
	chip_engine_period = (unsigned int)(chip_rate / 60.00); // Default to 60Hz

	// Build the thread
	chip_thread = al_create_thread(chip_func, NULL);
	printf("[audio] Created audio thread.\n");

	chip_is_init = 1;
}

void chip_start(void)
{
	if (!chip_is_init)
	{
		fprintf(stderr, "[audio] Error: LibChip has not been initialized.\n");
		return;
	}
	al_start_thread(chip_thread);
	printf("[audio] Started audio thread.\n");
}

void chip_set_engine_ptr(void *ptr, unsigned int eng_period)
{
	chip_engine_cnt = 0;
	chip_engine_ptr = ptr;
	if (eng_period)
	{
		chip_engine_period = eng_period;
	}
}

void *chip_get_engine_ptr(void)
{
	return chip_engine_ptr;
}

/* External control fuctions */
void chip_set_freq(unsigned int channel, float f)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
// Resulting frequency: (rate_mul * rate) / (wave_len * period)
	unsigned int set_p = (unsigned int)((chip_rate_mul * chip_rate) / (ch->wave_len * f));
	if (set_p < 1)
	{
		set_p = 1;
	}
	ch->period = set_p;
	printf("[audio] Set channel %d period to %d\n",channel,set_p);
}

void chip_set_period_direct(unsigned int channel, unsigned int period)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	if (period < 1)
	{
		period = 1;
	}
	ch->period = period;
}

void chip_set_amp(unsigned int channel, unsigned int amp_l, unsigned int amp_r)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	ch->amplitude[0] = amp_l;
	ch->amplitude[1] = amp_r;
}

void chip_set_noise(unsigned int channel, unsigned int noise_en)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	ch->noise_en = noise_en;
}

void chip_set_loop(unsigned int channel, unsigned int loop_en)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	ch->loop_en = loop_en;
}

// Point to user-owned wave data
void chip_set_wave(unsigned int channel, uint16_t *wave_data, unsigned int len, unsigned int loop_en)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	al_lock_mutex(ch->mutex);
	// Clear out the previous wave if we own it
	if (ch->own_wave && ch->wave_data)
	{
		free(ch->wave_data);
		ch->wave_data = NULL;
	}
	ch->wave_data = wave_data;
	ch->wave_len = len;
	ch->loop_en = loop_en;
	ch->own_wave = 0;
	al_unlock_mutex(ch->mutex);
}

// Create a buffer for wave data owned by the library
void chip_create_wave(unsigned int channel, unsigned int len, unsigned int loop_en)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	al_lock_mutex(ch->mutex);
	if (!len)
	{
		fprintf(stderr,"[audio] Error: Wave length of 0 specified. The engine may crash.\n");
		return;
	}
	// Clear out the previous wave if we own it
	if (ch->own_wave && ch->wave_data)
	{
		free(ch->wave_data);
		ch->wave_data = NULL;
	}
	ch->wave_data = (uint16_t *)calloc(len,sizeof(uint16_t));
	ch->own_wave = 1;
	ch->wave_len = len;
	ch->loop_en = loop_en;
	al_unlock_mutex(ch->mutex);
}

void chip_set_wave_pos(unsigned int channel, unsigned int pos)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	ch->wave_pos = pos;
}

void chip_set_noise_tap(unsigned int channel, unsigned int tap)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return;
	}
	chip_channel *ch = &chip_channels[channel];
	if (tap > 15)
	{
		tap = 0;
	}
	ch->noise_tap = tap;
}

unsigned int chip_get_period(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->period;
}
unsigned int chip_get_amp(unsigned int channel, unsigned int side)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->amplitude[side % 2];
}

unsigned int chip_get_noise(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->noise_en;
}

unsigned int chip_get_loop(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->loop_en;
}

uint16_t *chip_get_wave(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return NULL;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->wave_data;
}

unsigned int get_is_user_wave(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return !ch->own_wave;
}

unsigned int chip_get_wave_len(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->wave_len;
}

chip_channel *chip_get_channel(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	return &chip_channels[channel];
}

unsigned int chip_get_wave_pos(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->wave_pos;
}

unsigned int chip_get_noise_tap(unsigned int channel)
{
	if (channel >= chip_num_channels)
	{
		fprintf(stderr,"[audio] Error: Channel out of range (%d > %d)\n",channel,chip_num_channels);
		return 0;
	}
	chip_channel *ch = &chip_channels[channel];
	return ch->noise_tap;
	
}
