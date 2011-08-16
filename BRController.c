#include "BRController.h"

#include "BREngine.h"
#include "BRVQAReader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

struct BRController
{
	enum {
		BR_CONTROLLER_KIND_NONE = 0,
		BR_CONTROLLER_KIND_SEQUENCE,
		BR_CONTROLLER_KIND_VQA_PLAYER
	} kind;

	union {
		struct {
			char *name;
			BRVQAReaderRef vqaReader;
			int frameNumber;
			int frameEnd;
			int frameCount;
			int leadInLoop;
			int mainLoop;
			enum {
				NO_LOOP = 0,
				LEAD_IN_LOOP,
				MAIN_LOOP
			} loopStatus;
		} vqa_player;
		struct {
			struct _BRControllerSequenceItem *head;
			struct _BRControllerSequenceItem *tail;
		} sequence;
	};

	BREngineRef engine;

	struct _BRControllerSequenceItem {
		BRControllerRef controller;
		struct _BRControllerSequenceItem *next;
	};
};

BRControllerRef BRControllerCreateVQAPlayer(BREngineRef aEngine, const char *aVQAName)
{
	BRControllerRef c = calloc(1, sizeof(struct BRController));

	c->engine = aEngine;
	c->kind = BR_CONTROLLER_KIND_VQA_PLAYER;
	c->vqa_player.name = strdup(aVQAName);
	c->vqa_player.leadInLoop = -1;
	c->vqa_player.mainLoop = -1;

	return c;
}

void BRControllerVQAPlayerSetLeadInLoop(BRControllerRef aController, int aLeadInLoop)
{
	aController->vqa_player.leadInLoop = aLeadInLoop;
}

void BRControllerVQAPlayerSetMainLoop(BRControllerRef aController, int aMainLoop)
{
	aController->vqa_player.mainLoop = aMainLoop;
}

BRControllerRef BRControllerSequenceCreate(BREngineRef aEngine)
{
	BRControllerRef c = calloc(1, sizeof(struct BRController));

	c->engine = aEngine;
	c->kind = BR_CONTROLLER_KIND_SEQUENCE;

	return c;
}

BOOL BRControllerSequenceAddController(BRControllerRef aSequence, BRControllerRef aController)
{
	struct _BRControllerSequenceItem *n = calloc(1, sizeof(struct _BRControllerSequenceItem));
	if (!n)
		return NO;

	n->controller = aController;

	if (!aSequence->sequence.head)
	{
		aSequence->sequence.head = aSequence->sequence.tail = n;
		return YES;
	}

	aSequence->sequence.tail->next = n;
	aSequence->sequence.tail = n;
	return YES;
}

void _BRControllerInitialize(BRControllerRef aController)
{
	if (aController->kind == BR_CONTROLLER_KIND_VQA_PLAYER)
	{
		if (!aController->vqa_player.vqaReader)
		{
			BRPtrRangeRef r = BREngineGetResource(aController->engine, aController->vqa_player.name);
			aController->vqa_player.vqaReader = BRVQAReaderOpen(r);
			aController->vqa_player.frameNumber = 0;
			aController->vqa_player.frameEnd = 0;
			aController->vqa_player.frameCount = BRVQAReaderGetFrameCount(aController->vqa_player.vqaReader);
		}
	}
}

BOOL _BRControllerHasFrame(BRControllerRef aController)
{
	_BRControllerInitialize(aController);

	if (aController->kind == BR_CONTROLLER_KIND_VQA_PLAYER)
	{
		if (aController->vqa_player.frameNumber < aController->vqa_player.frameEnd)
			return YES;

		if (aController->vqa_player.loopStatus == NO_LOOP && aController->vqa_player.leadInLoop != -1)
		{
			aController->vqa_player.loopStatus = LEAD_IN_LOOP;
			BRVQAReaderGetLoop(aController->vqa_player.vqaReader,
			                   aController->vqa_player.leadInLoop,
			                   &aController->vqa_player.frameNumber,
			                   &aController->vqa_player.frameEnd);
			assert(aController->vqa_player.frameNumber < aController->vqa_player.frameEnd);
			return YES;
		}
		if (aController->vqa_player.loopStatus != NO_LOOP && aController->vqa_player.mainLoop != -1)
		{
			aController->vqa_player.loopStatus = MAIN_LOOP;
			BRVQAReaderGetLoop(aController->vqa_player.vqaReader,
			                   aController->vqa_player.mainLoop,
			                   &aController->vqa_player.frameNumber,
			                   &aController->vqa_player.frameEnd);
			assert(aController->vqa_player.frameNumber < aController->vqa_player.frameEnd);
			return YES;
		}

		return aController->vqa_player.frameNumber < aController->vqa_player.frameCount;
	}
	if (aController->kind == BR_CONTROLLER_KIND_SEQUENCE)
	{
		while (aController->sequence.head && !_BRControllerHasFrame(aController->sequence.head->controller))
			aController->sequence.head = aController->sequence.head->next;

		return !!aController->sequence.head;
	}
	exit(1);
}

BOOL BRControllerGetAVFrame(BRControllerRef aController, BRAVFrame *aFrame)
{
	assert(aFrame);
	BOOL hasFrame = _BRControllerHasFrame(aController);
	if (!hasFrame)
		return NO;

	if (aController->kind == BR_CONTROLLER_KIND_VQA_PLAYER)
	{
		BRVQAReaderReadFrame(aController->vqa_player.vqaReader, aController->vqa_player.frameNumber);

		aController->vqa_player.frameNumber++;

		*aFrame = BRVQAReaderGetAVFrame(aController->vqa_player.vqaReader);
		return YES;
	}
	if (aController->kind == BR_CONTROLLER_KIND_SEQUENCE)
	{
		return BRControllerGetAVFrame(aController->sequence.head->controller, aFrame);
	}
	exit(1);
}
