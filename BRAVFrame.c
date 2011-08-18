#include "BRAVFrame.h"

#include "BRRetain.h"

#include <stdio.h>

struct BRAudioFrame
{
	BRRetainable _;
	uint32_t  sampleCount;
	BRDataRef data;
};

struct BRVideoFrame
{
	BRRetainable _;
	BRSize    size;
	BRSize    stride;
	BRDataRef data;
};

struct BRAVFrame
{
	BRRetainable _;
	BRAudioFrameRef audioFrame;
	BRVideoFrameRef videoFrame;
};

void BRAudioFrameDealloc(BRAudioFrameRef aAudioFrame);
void BRVideoFrameDealloc(BRVideoFrameRef aVideoFrame);
void BRAVFrameDealloc(BRAVFrameRef aAVFrame);

BRAudioFrameRef BRAudioFrameCreate(uint32_t aSampleCount, BRDataRef aData)
{
	BRAudioFrameRef audioFrame = calloc(1, sizeof(struct BRAudioFrame));
	if (!audioFrame) return 0;
	//printf("BRAudioFrameCreate: %p\n", audioFrame);
	BRRetainableSetDeallocFunc(audioFrame, BRAudioFrameDealloc);

	audioFrame->sampleCount = aSampleCount;
	audioFrame->data        = BRRetain(aData);

	return audioFrame;
}

uint32_t BRAudioFrameGetSampleCount(BRAudioFrameRef aAudioFrame)
{
	return aAudioFrame->sampleCount;
}

BRDataRef BRAudioFrameGetData(BRAudioFrameRef aAudioFrame)
{
	return aAudioFrame->data;
}

void BRAudioFrameDealloc(BRAudioFrameRef aAudioFrame)
{
	//printf("BRAudioFrameDealloc %p *****\n", aAudioFrame);
	BRRelease(aAudioFrame->data);
	free(aAudioFrame);
}

BRVideoFrameRef BRVideoFrameCreate(BRSize aSize, BRSize aStride, BRDataRef aData)
{
	BRVideoFrameRef videoFrame = calloc(1, sizeof(struct BRVideoFrame));
	if (!videoFrame) return 0;
	//printf("BRVideoFrameCreate: %p\n", videoFrame);
	BRRetainableSetDeallocFunc(videoFrame, BRVideoFrameDealloc);

	videoFrame->size   = aSize;
	videoFrame->stride = aStride;
	videoFrame->data   = BRRetain(aData);

	return videoFrame;
}

void BRVideoFrameDealloc(BRVideoFrameRef aVideoFrame)
{
	//printf("BRVideoFrameDealloc %p *****\n", aVideoFrame);
	BRRelease(aVideoFrame->data);
	free(aVideoFrame);
}

BRSize BRVideoFrameGetSize(BRVideoFrameRef aVideoFrame)
{
	return aVideoFrame->size;
}

BRSize BRVideoFrameGetStride(BRVideoFrameRef aVideoFrame)
{
	return aVideoFrame->stride;
}

BRDataRef BRVideoFrameGetData(BRVideoFrameRef aVideoFrame)
{
	return aVideoFrame->data;
}

BRAVFrameRef BRAVFrameCreate(BRAudioFrameRef aAudioFrame, BRVideoFrameRef aVideoFrame)
{
	BRAVFrameRef avFrame = calloc(1, sizeof(struct BRAVFrame));
	if (!avFrame) return 0;
	//printf("BRAVFrameCreate: %p\n", avFrame);
	BRRetainableSetDeallocFunc(avFrame, BRAVFrameDealloc);

	if (aAudioFrame)
		avFrame->audioFrame = BRRetain(aAudioFrame);

	if (aVideoFrame)
		avFrame->videoFrame = BRRetain(aVideoFrame);

	return avFrame;
}

void BRAVFrameDealloc(BRAVFrameRef aAVFrame)
{
	//printf("BRAVFrameDealloc %p  *****\n", aAVFrame);
	BRRelease(aAVFrame->audioFrame);
	BRRelease(aAVFrame->videoFrame);
	free(aAVFrame);
}

BRAudioFrameRef BRAVFrameGetAudioFrame(BRAVFrameRef aAVFrame)
{
	return aAVFrame->audioFrame;
}

BRVideoFrameRef BRAVFrameGetVideoFrame(BRAVFrameRef aAVFrame)
{
	return aAVFrame->videoFrame;
}
