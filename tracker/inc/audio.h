#ifndef AUDIO_H
#define AUDIO_H

#define AUDIO_NUM_CHANNELS 8

void audio_init(void);
void audio_shutdown(void);

void audio_set_rate(unsigned int r);
void audio_set_buffer_len(unsigned int b);
void audio_set_num_buffers(unsigned int n);
void audio_set_oversample(unsigned int o);
void audio_set_channel_count(unsigned int c);

unsigned int audio_get_rate(void);
unsigned int audio_get_buffer_len(void);
unsigned int audio_get_num_buffers(void);
unsigned int audio_get_oversample(void);
unsigned int audio_get_channel_count(void);

#endif
