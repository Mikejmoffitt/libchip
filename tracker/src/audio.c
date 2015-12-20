#include "audio.h"
#include <stdio.h>

static int audio_is_init;

void audio_init(void)
{
	if (audio_is_init)
	{
		fprintf(stderr, "%s: Audio is already init!\n",__PRETTY_FUNCTION__);
	}
	audio_is_init = 1;
}

void audio_shutdown(void)
{
}
