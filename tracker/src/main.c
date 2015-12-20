#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>

#include "audio.h"

int main(int argc, char **argv)
{
	audio_init();
	audio_shutdown();
	return 0;
}
