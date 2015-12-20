// LibChip waveform definition example
// 2015 Michael Moffitt
// https://github.com/mikejmoffitt/libchip

#include <stdio.h>
#include <stdlib.h>
#include <libchip.h>

#define EX_RATE 44100
#define EX_BUFFER_LEN 256
#define EX_NUM_BUFFERS 8
#define EX_OVERSAMPLE 8

#define NOTE_A 

static uint16_t wave_tri[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8,
	0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
};

static uint16_t wave_50[] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF
};

static uint16_t wave_saw[] = {
	0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,
	8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15
};

static uint16_t wave_funk[] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0xF,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF,
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF
};

// Song state
unsigned int frame_counter;
unsigned int amp;// Note lookup table
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

void playnote(int chan, int note, int oct)
{
	chip_set_freq(chan, freqs[note] * (1 << oct));
}

// Engine callback function, responsible for repeating 2Hz volume envelope
void printme(void)
{
	chip_set_amp(0,0xF * (1.0 - (amp / 30.0)), 0xF * (1.0 - (amp / 30.0)));
	amp++;
	if (amp == 30)
	{
		amp = 0;
	}
	frame_counter++;
}

int main(int argc, char **argv)
{
	chip_init(EX_RATE, 1, EX_BUFFER_LEN, EX_NUM_BUFFERS, EX_OVERSAMPLE);
	chip_set_engine_ptr(&printme,0);
	chip_set_wave(0, wave_tri, 32, 1);
	chip_set_amp(0,0xF,0xF);
	chip_set_freq(0,110);
	char c = 0;
	chip_start();
	printf("LibChip waveforms example\n");
	printf("Enter a letter for the corresponding choice and strike enter.\n");
	printf(" [t] /\\/\\/\\/\\ \n");
	printf(" [p] --__--__--__\n");
	printf(" [s] /_/_/_/_\n");
	printf(" [f] __-_____-------\n");
	printf(" [n] (noise)\n");
	printf(" [q] Quit\n");
	while(c != 'q')
	{
		printf(">");
		c = getchar();
		printf("\n");
		switch (c | 0x20) // Case insensitive; make it uppercase
		{
			case 't':
				chip_set_wave(0, wave_tri, 32, 1);
	//			chip_set_wave(1, wave_tri, 32, 1);
				chip_set_noise(0,0);
	//			chip_set_noise(1,0);
				break;
			case 'p':
				chip_set_wave(0, wave_50, 32, 1);
	//			chip_set_wave(1, wave_50, 32, 1);
				chip_set_noise(0,0);
	//			chip_set_noise(1,0);
				break;
			case 's':
				chip_set_wave(0, wave_saw, 32, 1);
	//			chip_set_wave(1, wave_saw, 32, 1);
				chip_set_noise(0,0);
	//			chip_set_noise(1,0);
				break;
			case 'f':
				chip_set_wave(0, wave_funk, 64, 1);
	//			chip_set_wave(1, wave_funk, 32, 1);
				chip_set_noise(0,0);
	//			chip_set_noise(1,0);
				break;
			case 'n':
				chip_set_noise(0,1);
	//			chip_set_noise(1,1);
				break;
			default:
				break;
		}
	}
	chip_shutdown();
	printf("Finished after %f seconds.\n",frame_counter / 60.0);
	return 0;
}
