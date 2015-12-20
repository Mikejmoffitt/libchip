#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "audio.h"
#include "engine.h"

int main(int argc, char **argv)
{
	audio_init();
	engine_attach_callback();
	while(1);
	audio_shutdown();
	return 0;
}
