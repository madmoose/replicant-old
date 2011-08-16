#include <windows.h>

#include "BRAudioSink.h"
#include "BREngine.h"
#include "BROpenGLRenderer.h"
#include "BRRetain.h"
#include "BRWindow.h"

#include <assert.h>
#include <stdio.h>

BOOL handleInput();
int64_t queryPerformanceFrequency();
int64_t queryPerformanceCounter();

int main()
{
	BREngineRef engine = BREngineCreate();
	assert(engine);

	BRAudioSinkRef audioSink = BRAudioSinkCreate();
	assert(audioSink);

	BRWindowRef window = BRWindowCreate();
	assert(window);

	BROpenGLRendererRef renderer = BROpenGLRendererCreate(window, engine);
	assert(renderer);

	//_BROpenGLRendererGetInfo(renderer);


	int64_t frequency = queryPerformanceFrequency();
	int64_t ticksPerFrame = frequency / 15;

	int64_t lastFrameTime = queryPerformanceCounter() - ticksPerFrame;

	int64_t currentTime;

	int64_t frameDeltaTicks = 0;

	BRAVFrame avFrame;
	avFrame.audio.data = 0;
	avFrame.video.data = 0;

	BRAudioSinkStart(audioSink);
	while (handleInput())
	{
		currentTime = queryPerformanceCounter();

		frameDeltaTicks += currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		if (frameDeltaTicks > ticksPerFrame)
		{
			if (frameDeltaTicks / ticksPerFrame > 1)
				puts("Catching up!");

			while (frameDeltaTicks > ticksPerFrame)
			{
				frameDeltaTicks -= ticksPerFrame;

				if (avFrame.video.data)
				{
					BRRelease(avFrame.video.data);
					avFrame.video.data = 0;
				}

				avFrame = BREngineGetFrame(engine);

				if (!avFrame.audio.data)
				{
					BRAudioSinkStop(audioSink);
					puts("NO Audio frame!");
				}
				else
					BRAudioSinkEnqueueAudio(audioSink, avFrame.audio.data);
				avFrame.audio.data = 0;
			}

			BROpenGLRendererRenderFrame(renderer, avFrame.video);

			BRRelease(avFrame.video.data);
			avFrame.video.data = 0;
		}
		Sleep(10);
	}
}

BOOL handleInput()
{
	MSG msg;
	BOOL quit = FALSE;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			quit = TRUE;
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return !quit;
}

int64_t queryPerformanceFrequency()
{
	LARGE_INTEGER v;
	QueryPerformanceFrequency(&v);
	int64_t r;
	memcpy(&r, &v, sizeof(uint64_t));
	return r;
}

int64_t queryPerformanceCounter()
{
	LARGE_INTEGER v;
	QueryPerformanceCounter(&v);
	int64_t r;
	memcpy(&r, &v, sizeof(uint64_t));
	return r;
}
