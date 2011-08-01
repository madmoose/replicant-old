#include "BRCommon.h"
#include "BRUtils.h"
#include "BRMixFile.h"
#include "BRVQAReader.h"

#include <stdio.h>
#include <stdlib.h>

#define kFORM 0x464f524d
#define kSet0 0x53657430

/*
#include <string.h>
void DumpSet0(BRPtrRangeRef r)
{
	BRPtrRangeAdvance(r, 8);
	uint32_t unk1 = BRPtrRangeReadLE32AndAdvance(r);
	printf("%d\n", unk1);
	char name[21] = {0};

	uint32_t i;
	for (i = 0; i != unk1; ++i)
	{
		memcpy(name, BRPtrRangeGetBegin(r), 20);
		BRPtrRangeAdvance(r, 20);
		printf("name: %s\n", name);
		hexdump(BRPtrRangeGetBegin(r), 30);
		BRPtrRangeAdvance(r, 30);
		putchar('\n');
	}

	uint32_t unk2 = BRPtrRangeReadLE32AndAdvance(r);
	printf("%d\n", unk2);

	for (i = 0; i != unk2; ++i)
	{
		memcpy(name, BRPtrRangeGetBegin(r), 20);
		BRPtrRangeAdvance(r, 20);
		printf("name: %s\n", name);
		hexdump(BRPtrRangeGetBegin(r), 40);
		BRPtrRangeAdvance(r, 40);
		putchar('\n');
	}

	if (BRPtrRangeGetDistance(r) == 0)
		return;

	puts("--- Lights");
	BRPtrRangeAdvance(r, 12);
	uint32_t unk3 = BRPtrRangeReadLE32AndAdvance(r);
	printf("%d\n", unk3);

	for (i = 0; i != unk3; ++i)
	{
		uint32_t unk4 = BRPtrRangeReadLE32AndAdvance(r);
		uint32_t unk5 = BRPtrRangeReadLE32AndAdvance(r);

		printf("unk4: %d  unk5: %d\n", unk4, unk5);

		memcpy(name, BRPtrRangeGetBegin(r), 20);
		printf("name: %s\n", name);

		hexdump(BRPtrRangeGetBegin(r), unk5);

		BRPtrRangeAdvance(r, unk5 - 8);
		putchar('\n');
	}

	puts("Remainder:");
	hexdump(BRPtrRangeGetBegin(r), BRPtrRangeGetDistance(r));
}
*/

int main(int argc, char const *argv[])
{
	if (argc != 2)
		return 0;

	const char *mixFilename = argv[1];

	BRMixFileRef mixFile = BRMixFileOpen(mixFilename);

	if (!mixFile)
	{
		printf("Attempt to open MIX file '%s' failed.\n", mixFilename);
		goto cleanup;
	}

	int i, end = BRMixFileGetResourceCount(mixFile);
	for (i = 0; i != end; ++i)
	{
		printf("\nResource %d\n", i);
		BRPtrRangeRef r = BRMixFileGetResourceRangeByIndex(mixFile, i);

		uint32_t tag = BRPtrRangeReadBE32(r);

		if (tag == kFORM)
		{
			BRVQAReaderRef vqa = BRVQAReaderOpen(r);
			BRVQAReaderClose(vqa);
		}
		/*
		else if (tag == kSet0)
		{
			hexdump(BRPtrRangeGetBegin(r), BRPtrRangeGetDistance(r));
			DumpSet0(r);
		}
		*/
		else
		{
			hexdump(BRPtrRangeGetBegin(r), MIN(BRPtrRangeGetDistance(r), 16));
		}
		free(r);
	}

cleanup:
	BRMixFileClose(mixFile);

	return 0;
}
