#include "BREngine.h"

#include "BRController.h"
#include "BRMixFile.h"
#include "BRVQAReader.h"

#include <assert.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_MIX_FILES 16

struct BREngine
{
	BOOL           isInitialized;
	BRMixFileRef   mixFiles[MAX_MIX_FILES];
	BRVQAReaderRef vqaReader;
	int            nextVQAFrame;
	int            nextResource;

	char          *datapath;

	BRControllerRef controller;
};

BOOL _BREngineSetDataPath(BREngineRef aEngine, const char *aPath);
BOOL _BREngineOpenMixFile(BREngineRef aEngine, const char *aPath);

BOOL _BREngineReadGameInfo(BREngineRef aEngine);

BREngineRef BREngineCreate()
{
	BREngineRef aEngine = calloc(1, sizeof(struct BREngine));

	_BREngineSetDataPath(aEngine, "");

	return aEngine;
}

BOOL _BREngineOpenMixFile(BREngineRef aEngine, const char *aPath)
{
	int i;
	for (i = 0; i != MAX_MIX_FILES; ++i)
		if (aEngine->mixFiles[i] == 0)
			break;
	if (i == 16)
		return NO;

#ifdef _WIN32
	char path[MAX_PATH] = "";

	strcat(path, aEngine->datapath);
	if (strlen(path) > 0)
		strcat(path, "\\");
	strcat(path, aPath);

	BRMixFileRef mixFile = BRMixFileOpen(path);
#else
	BRMixFileRef mixFile = BRMixFileOpen(aPath);
#endif
	if (!mixFile)
		return NO;

	aEngine->mixFiles[i] = mixFile;

	return YES;
}

BOOL _BREngineSetDataPath(BREngineRef aEngine, const char *aPath)
{
	BOOL rc = YES;
#ifdef _WIN32
	char path[MAX_PATH];

	struct stat s;
	if (access(aPath, 0) == 0 && stat(aPath, &s) == 0)
		rc = s.st_mode & S_IFDIR;

	if (!rc)
	{
		puts("Datapath is not a valid directory.");
		return NO;
	}

	aEngine->datapath = strdup(aPath);

	size_t len = strlen(aEngine->datapath);
	if (len > 0 && aEngine->datapath[len - 1] == '\\')
		aEngine->datapath[len - 1] = '\0';

#endif
	return rc;
}

BOOL _BREngineReadGameInfo(BREngineRef aEngine)
{
	BRPtrRangeRef r = BREngineGetResource(aEngine, "GAMEINFO.DAT");
	if (!r)
	{
		puts("GAMEINFO.DAT not found");
		return NO;
	}
	//hexdump(BRPtrRangeGetBegin(r), BRPtrRangeGetDistance(r));

	free(r);
	return YES;
}

BRPtrRangeRef BREngineGetResource(BREngineRef aEngine, const char *name)
{
	int i;
	for (i = 0; i != MAX_MIX_FILES; ++i)
	{
		if (aEngine->mixFiles[i] == 0)
			break;

		BRPtrRangeRef r = BRMixFileGetResourceRangeByName(aEngine->mixFiles[i], name);
		if (!r)
			continue;

		printf("Resource '%s' found in MIX file '%s'\n", name, BRMixFileGetName(aEngine->mixFiles[i]));
		return r;
	}

	printf("Resource '%s' not found.\n", name);
	return 0;
}

BOOL _BREngineInit(BREngineRef aEngine)
{
	assert(!aEngine->isInitialized);
	aEngine->isInitialized = YES;


	_BREngineOpenMixFile(aEngine, "BASE\\STARTUP.MIX");
	_BREngineOpenMixFile(aEngine, "BASE\\MUSIC.MIX");
	_BREngineOpenMixFile(aEngine, "BASE\\MODE.MIX");

	_BREngineOpenMixFile(aEngine, "CD1\\VQA1.MIX");
	_BREngineOpenMixFile(aEngine, "CD2\\VQA2.MIX");
	_BREngineOpenMixFile(aEngine, "CD3\\VQA3.MIX");

	_BREngineOpenMixFile(aEngine, "CD1\\OUTTAKE1.MIX");
	_BREngineOpenMixFile(aEngine, "CD2\\OUTTAKE2.MIX");
	_BREngineOpenMixFile(aEngine, "CD3\\OUTTAKE3.MIX");
	_BREngineOpenMixFile(aEngine, "CD4\\OUTTAKE4.MIX");

	_BREngineReadGameInfo(aEngine);

	aEngine->controller = BRControllerSequenceCreate(aEngine);

	BRControllerRef c;

	//c = BRControllerCreateVQAPlayer(aEngine, "ESPER.VQA");
	//BRControllerSequenceAddController(aEngine->controller, c);

	c = BRControllerCreateVQAPlayer(aEngine, "WSTLGO_E.VQA");
	BRControllerSequenceAddController(aEngine->controller, c);

	c = BRControllerCreateVQAPlayer(aEngine, "BRLOGO_E.VQA");
	BRControllerSequenceAddController(aEngine->controller, c);

	c = BRControllerCreateVQAPlayer(aEngine, "INTRO_E.VQA");
	BRControllerSequenceAddController(aEngine->controller, c);

	c = BRControllerCreateVQAPlayer(aEngine, "DSCENT_E.VQA");
	BRControllerSequenceAddController(aEngine->controller, c);

	c = BRControllerCreateVQAPlayer(aEngine, "RC01.VQA");
	BRControllerVQAPlayerSetLeadInLoop(c, 0);
	BRControllerVQAPlayerSetMainLoop(c, 1);

	BRControllerSequenceAddController(aEngine->controller, c);

	return YES;
}

BRAVFrame BREngineGetFrame(BREngineRef aEngine)
{
	if (!aEngine->isInitialized)
		_BREngineInit(aEngine);

	BRAVFrame avFrame;
	BOOL hasFrame = BRControllerGetAVFrame(aEngine->controller, &avFrame);
	if (!hasFrame)
		exit(1);

	return avFrame;
}
