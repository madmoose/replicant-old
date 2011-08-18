#include <windows.h>

#include "BRAudioSink.h"
#include "BREngine.h"
#include "BROpenGLRenderer.h"
#include "BRQueue.h"
#include "BRRetain.h"
#include "BRWindow.h"

#include <assert.h>
#include <stdio.h>

void handleInput();
int64_t queryPerformanceFrequency();
int64_t queryPerformanceCounter();

void runLoop();

BREngineRef engine;
BRAudioSinkRef audioSink;
BRWindowRef window;
BROpenGLRendererRef renderer;

int main()
{
	engine = BREngineCreate();
	assert(engine);

	audioSink = BRAudioSinkCreate();
	assert(audioSink);

	window = BRWindowCreate();
	assert(window);

	//_BROpenGLRendererGetInfo(renderer);
	DWORD WINAPI runLoopTheadProc(LPVOID lpParameter);
	HANDLE runLoopThread = CreateThread(0, 0, runLoopTheadProc, 0, 0, 0);

	handleInput();
}

DWORD WINAPI runLoopTheadProc(LPVOID lpParameter)
{
	runLoop();
}

void runLoop()
{
	renderer = BROpenGLRendererCreate(window, engine);
	assert(renderer);

	int64_t frequency = queryPerformanceFrequency();
	int64_t ticksPerFrame = frequency / 15;

	int64_t lastFrameTime = queryPerformanceCounter() - ticksPerFrame;

	int64_t currentTime;
	int64_t frameDeltaTicks = 0;

	BOOL audioStarted = NO;

	BRQueueRef videoFrameQueue = BRQueueCreate();

	for (;;)
	{
		int samplesInQueue = BRAudioSinkGetSamplesInQueue(audioSink);

		// Make sure we more than one frame queued
		if (samplesInQueue <= 1470)
		{
			while (samplesInQueue <= 1470)
			{
				BRAVFrameRef avFrame = BREngineGetFrame(engine);
				assert(avFrame);

				BRAudioFrameRef audioFrame = BRAVFrameGetAudioFrame(avFrame);
				assert(BRAudioFrameGetData(audioFrame));
				assert(BRDataGetBytes(BRAudioFrameGetData(audioFrame)));

				BRAudioSinkEnqueueAudio(audioSink, BRAudioFrameGetData(audioFrame));

				BRVideoFrameRef videoFrame = BRAVFrameGetVideoFrame(avFrame);
				assert(videoFrame);
				BRQueueEnqueue(videoFrameQueue, videoFrame);

				if (!audioStarted)
				{
					audioStarted = YES;
					BRAudioSinkStart(audioSink);
				}

				BRRelease(avFrame);

				samplesInQueue = BRAudioSinkGetSamplesInQueue(audioSink);
			}

			BRVideoFrameRef videoFrame = BRQueueGetHead(videoFrameQueue);
			BROpenGLRendererRenderFrame(renderer, videoFrame);
			BRQueueDequeue(videoFrameQueue);
		}
		Sleep(10);
	}
}

void handleInput()
{
	MSG msg;
	BOOL quit = FALSE;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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
