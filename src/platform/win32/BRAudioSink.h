#ifndef BR_AUDIO_SINK_H
#define BR_AUDIO_SINK_H

#include "BRCommon/BRCommon.h"

typedef struct BRAudioSink * BRAudioSinkRef;

BRAudioSinkRef BRAudioSinkCreate();

void BRAudioSinkStart(BRAudioSinkRef aAudioSink);
void BRAudioSinkStop(BRAudioSinkRef aAudioSink);

BOOL BRAudioSinkEnqueueAudio(BRAudioSinkRef aAudioSink, BRDataRef data);

int BRAudioSinkGetSamplesInQueue(BRAudioSinkRef aAudioSink);

#endif
