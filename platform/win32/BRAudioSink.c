#include "BRAudioSink.h"

#include "BRCommon.h"
#include "BRData.h"
#include "BRQueue.h"

#include <SDL/SDL.h>

#include <assert.h>
#include <stdlib.h>

struct BRAudioSink
{
	BRQueueRef audioQueue;

	// Total data in queue
	int inAudioQueue;

	// How much data remains in the queue head
	// if we've eaten partially in to it?
	int headRemainder;
};

void _BRAudioSinkSDLCallback(void *, uint8_t *data, int length);

BRAudioSinkRef BRAudioSinkCreate()
{
	BRAudioSinkRef audioSink = calloc(1, sizeof(struct BRAudioSink));
	cleanup_if_not(audioSink);

	audioSink->audioQueue = BRQueueCreate();
	cleanup_if_not(audioSink);


	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		goto cleanup;
	}
	atexit(SDL_Quit);

	SDL_AudioSpec desired, obtained;

	desired.freq     = 22050;
	desired.format   = AUDIO_S16LSB;
	desired.channels = 1;
	desired.samples  = 1470; // One frame's worth at 15 FPS
	desired.callback = _BRAudioSinkSDLCallback;
	desired.userdata = audioSink;

	if (SDL_OpenAudio(&desired, &obtained) < 0)
	{
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		goto cleanup;
	}

	return audioSink;
cleanup:
	if (audioSink)
		free(audioSink->audioQueue);
	free(audioSink);
	return 0;
}

void BRAudioSinkStart(BRAudioSinkRef aAudioSink)
{
	SDL_PauseAudio(0);
}

void BRAudioSinkStop(BRAudioSinkRef aAudioSink)
{
	SDL_PauseAudio(1);
}

BOOL BRAudioSinkEnqueueAudio(BRAudioSinkRef aAudioSink, BRDataRef aAudioData)
{
	assert(aAudioSink);
	assert(aAudioData);
	assert(BRDataGetBytes(aAudioData));

	SDL_LockAudio();
	aAudioSink->inAudioQueue += BRDataGetSize(aAudioData);
	BOOL rc = BRQueueEnqueue(aAudioSink->audioQueue, aAudioData);
	SDL_UnlockAudio();

	return rc;
}

void _BRAudioSinkSDLCallback(void *aAudioSink, uint8_t *stream, int requestLength)
{
	BRAudioSinkRef audioSink = (BRAudioSinkRef)aAudioSink;
	audioSink->inAudioQueue -= MIN(requestLength, audioSink->inAudioQueue);

	if (audioSink->headRemainder)
	{
		BRDataRef head = (BRDataRef)BRQueueGetHead(audioSink->audioQueue);
		assert(head);

		int headSize   = BRDataGetSize(head);

		int grab = MIN(requestLength, audioSink->headRemainder);

		memcpy(stream,
		       BRDataGetBytes(head) + (BRDataGetSize(head) - audioSink->headRemainder),
		       grab);

		stream += grab;
		requestLength -= grab;
		audioSink->headRemainder -= grab;

		if (audioSink->headRemainder == 0)
			BRQueueDequeue(audioSink->audioQueue);
	}

	while (requestLength)
	{
		BRDataRef head = (BRDataRef)BRQueueGetHead(audioSink->audioQueue);

		if (!head)
		{
			puts("Audio buffer underflow!");
			memset(stream, 0, requestLength);
			break;
		}

		int headSize = BRDataGetSize(head);

		int grab     = MIN(requestLength, headSize);

		memcpy(stream, BRDataGetBytes(head), grab);

		stream += grab;
		requestLength -= grab;
		audioSink->headRemainder = headSize - grab;

		if (audioSink->headRemainder == 0)
			BRQueueDequeue(audioSink->audioQueue);
	}
}
