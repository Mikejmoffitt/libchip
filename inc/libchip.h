/*

libChip interface.
By Michael Moffitt 2015
-----------------------

*/
#ifndef CHIP_H
#define CHIP_H

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#define CHIP_NUM_FRAGMENTS 4
#define CHIP_SIZE_FRAGMENT 1024
#define CHIP_RATE 44100
#define CHIP_DEPTH ALLEGRO_AUDIO_DEPTH_INT16
#define CHIP_CHAN ALLEGRO_CHANNEL_CONF_2

typedef struct chip_channel chip_channel;
struct chip_channel
{
	ALLEGRO_MUTEX *mutex; // Lock for reading/writing wave data
	uint16_t *wave_data; // Pointer to wave nybbles array
	unsigned int period; // Division of sample rate / rate multiplier.
	unsigned int counter; // Countdown until wave pos increment
	unsigned int amplitude[2]; // Left and right amplitude;
	unsigned int own_wave; // Mark if it should be destroyed
	unsigned int wave_len; // Number of samples in the wave
	unsigned int wave_pos; // Pointer within wave
	unsigned int loop_en; // Will the wave loop at the end or stop playing?
	unsigned int noise_en; // When nonzero, make LSFR noise like NES APU
	unsigned int noise_state;
	unsigned int noise_tap;
};

/* Setup Functions */

// Initialize libchip, allocating resources for specified channels / settings.
void chip_init(unsigned int rate, unsigned int num_channels, unsigned int frag_size, unsigned int frag_num, unsigned int rate_mul);

// Begin the sound generation thread.
void chip_start(void);

// Call to terminate the libChip thread and deallocate resources.
void chip_shutdown(void);

// Set a function pointer, called every n samples. Intended to progress a sound
// engine for consistent timing. This can be called before or after init.
void chip_set_engine_ptr(void *ptr, unsigned int n);

// Get the engine callback function pointer.
void *chip_get_engine_ptr(void);

/* Post-init generator setters */

// Set a channel's period by specifying the desired effective frequency. This is
// the only libChip function that will utilize floating point operations.
void chip_set_freq(unsigned int channel, float f);

// Set a channel's period directly.
void chip_set_period_direct(unsigned int channel, unsigned int period);

// Set the amplitude of a channel, from 0 (off) to 15 (full), with separate left
// and right coefficients. 
void chip_set_amp(unsigned int channel, unsigned int amp_l, unsigned int amp_r);

// Enable the noise generator on a channel, replacing wave generation. This will
// not destroy the channel's state, so it can be used temporarily and switched
// off to resume normal wave playback. Disabling the noise generator will halt
// it, not just silence it.
void chip_set_noise(unsigned int channel, unsigned int noise_en);

// Enable or disable sample looping for a channel's wave.
void chip_set_loop(unsigned int channel, unsigned int loop_en);

// Configure a channel to point to a user-owned wave buffer. The user must be
// responsible for allocating and deallocating the buffer, and the length must
// be within bounds because libChip can not perform bounds checks. 
void chip_set_wave(unsigned int channel, uint16_t *wave_data, unsigned int len, unsigned int loop_en);

// Configure a channel with a libChip-owned buffer of size [len] samples. The
// user does not need to (and should not) deallocate the channel's buffer, as
// libChip will do this as needed when a new wave is selected or when the engine
// is shut down.
void chip_create_wave(unsigned int channel, unsigned int len, unsigned int loop_en);

// Manually reset the wave playback phase to a specified position.
void chip_set_wave_pos(unsigned int channel, unsigned int pos);

// Configure the noise channel's LSFR tap bit. The default  will match the 
// Nintendo 2A03's tap configuration without user intervention.
void chip_set_noise_tap(unsigned int channel, unsigned int tap);

/* Post-init generator getters */

// Get the period of a channel directly. 
unsigned int chip_get_period(uint32_t channel);

// Get the amplitude for the specified side; zero for left, nonzero for right.
unsigned int chip_get_amp(unsigned int channel, unsigned int side);

// Determine whether or not the channel has the noise generator enabled. 
unsigned int chip_get_noise(unsigned int channel);

// Retrieve whether or not sample looping is enabled for the given channel.
unsigned int chip_get_loop(unsigned int channel);

// Get a pointer to the channel's sample buffer.
uint16_t *chip_get_wave(unsigned int channel);

// Retrieve whether or not the wave buffer is user owned.
unsigned int chip_get_is_user_wave(unsigned int channel);

// Get the length of the loaded sample buffer (or zero if there is none).
unsigned int chip_get_wave_len(unsigned int channel);

// Get a pointer to the entire channel's state. You may choose to do this,
// and manipulate channels by modifying struct members instead of using the
// utility functions. 
chip_channel *chip_get_channel(unsigned int channel);

// Get the current wave playback position. 
unsigned int chip_get_wave_pos(unsigned int channel);

// Get the current tap bit specified for the noise generator. 
unsigned int chip_get_noise_tap(unsigned int channel);

#endif
