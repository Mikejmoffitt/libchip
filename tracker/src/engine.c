#include "engine.h"
#include "audio.h"
#include <stdint.h>
#include <libchip.h>

// Test stuff
#define REST 64
#define SPD 9

unsigned int step_cnt;
unsigned int play_pos;
unsigned int amp[2];

static uint16_t wave1[] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x9, 0xC,
	0xF, 0xF, 0xF, 0xf, 0xF, 0xF, 0xF, 0x0,
	0x0, 0xF, 0xF, 0xf, 0xF, 0xC, 0x9, 0x3
};

static uint16_t wave2[] = {
	0x0, 0xF, 0x0, 0xF, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xF, 0xF, 0xF, 0xf, 0xF, 0xF, 0xF, 0xF,
	0xF, 0xF, 0xF, 0xf, 0xF, 0xF, 0xF, 0xF
};

static const unsigned int notes1[] = {
	5, REST, REST, REST, REST, 7, 9, 10, 
	9, 5, REST, 0, REST, 5, REST, 0,
	3, REST, 5, REST, 3, 2, REST, 0,
	REST, REST, REST, REST, REST, REST, REST, 0
};

static const unsigned int notes2[] = {
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	3, 3, 3, 3, 3, 3, 3, 3, 
	0, 0, 2, 2, 3, 3, 4, 4
};

static const unsigned int octs2[] = {
	2, 2, 3, 3, 2, 2, 3, 3, 
	2, 2, 3, 3, 2, 2, 3, 3, 
	2, 2, 3, 3, 2, 2, 3, 3, 
	2, 3, 2, 3, 2, 3, 2, 3
};

static const float freqs[] = {
	16.351,
	17.324,
	18.354,
	19.445,
	20.601,
	21.827,
	23.124,
	24.499,
	25.956,
	27.500,
	29.135,
	30.868
};

static void engine_set_note(unsigned int chan, unsigned int note, unsigned int oct)
{
	chip_set_freq(chan, freqs[note] * (1 << oct));
}

// Very simple playback test
static void engine_func(void)
{
	step_cnt++;
	if (step_cnt == SPD)
	{
		step_cnt = 0;
		// Sequence playback
		if (notes1[play_pos] != REST)
		{
			engine_set_note(0, notes1[play_pos], 4);
			amp[0] = 0xF;
		}
		else
		{
			amp[0] = 0;
		}
		if (notes2[play_pos] != REST)
		{
			engine_set_note(1, notes2[play_pos], octs2[play_pos]);
			amp[1] = 0xF;
		}
		else
		{
			amp[1] = 0;
		}
		play_pos++;
		if (play_pos == 32)
		{
			play_pos = 0;
		}
	}

	// Envelope generation
	if (amp[0])
	{
		amp[0]--;
	}
	if (amp[1])
	{
		amp[1]--;
	}
	chip_set_amp(0, amp[0], amp[0]);
	chip_set_amp(1, amp[1], amp[1]);
}

void engine_attach_callback(void)
{
	chip_set_engine_ptr((void *)engine_func, (unsigned int)(audio_get_rate() / 60.00)); // 60Hz
	chip_set_wave(0, &wave1[0], 32, 1);
	chip_set_wave(1, &wave2[0], 32, 1);
}
