#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "audio.h"
#include "engine.h"

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		audio_set_rate(atoi(argv[1]));	
	}
	if (argc > 2)
	{
		audio_set_buffer_len(atoi(argv[2]));	
	}
	if (argc > 3)
	{
		audio_set_num_buffers(atoi(argv[3]));	
	}
	audio_init();

	engine_attach_callback();
	while(1);
	audio_shutdown();
	return 0;
}
