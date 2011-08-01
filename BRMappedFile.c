#include "BRMappedFile.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#endif

struct BRMappedFile
{
#ifdef _WIN32
	HANDLE hFile;
	HANDLE hMap;
	LPVOID lpMapAddress;
	DWORD  dwFileSize;
#else
	int    fd;
	void  *data;
	size_t size;
#endif
};

BRMappedFileRef BRMappedFileMapFile(const char *aFilename)
{
	BRMappedFileRef mappedFile = calloc(1, sizeof(struct BRMappedFile));
	if (!mappedFile)
		goto cleanup;

#ifdef _WIN32

	mappedFile->hFile = INVALID_HANDLE_VALUE;
	mappedFile->hMap  = NULL;
	mappedFile->dwFileSize   = 0;
	mappedFile->lpMapAddress = 0;

	mappedFile->hFile = CreateFile(aFilename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
	if (mappedFile->hFile == INVALID_HANDLE_VALUE)
		goto cleanup;

	mappedFile->dwFileSize = GetFileSize(mappedFile->hFile, NULL);

	mappedFile->hMap  = CreateFileMapping(mappedFile->hFile, 0, PAGE_READONLY, 0, mappedFile->dwFileSize, 0);
	if (mappedFile->hMap == NULL)
		goto cleanup;

	mappedFile->lpMapAddress = MapViewOfFile(mappedFile->hMap, FILE_MAP_READ, 0, 0, mappedFile->dwFileSize);
	if (mappedFile->lpMapAddress == NULL)
		goto cleanup;

#else

	mappedFile->fd   = -1;
	mappedFile->data =  MAP_FAILED;
	mappedFile->size =  0;

	mappedFile->fd = open(aFilename, O_RDONLY);
	if (mappedFile->fd == -1)
		goto cleanup;

	struct stat st;
	if (fstat(mappedFile->fd, &st) == -1)
		goto cleanup;

	mappedFile->size = (size_t)st.st_size;

	mappedFile->data = mmap(0, mappedFile->size, PROT_READ, MAP_SHARED, mappedFile->fd, 0);
	if (mappedFile->data == MAP_FAILED)
		goto cleanup;

#endif

	return mappedFile;

cleanup:
	BRMappedFileUnmapFile(mappedFile);
	return 0;
}

void BRMappedFileUnmapFile(BRMappedFileRef aMappedFile)
{
	if (!aMappedFile)
		return;
#ifdef _WIN32
	if (aMappedFile->lpMapAddress != NULL)
		UnmapViewOfFile(aMappedFile->lpMapAddress);

	if (aMappedFile->hMap != NULL)
		CloseHandle(aMappedFile->hMap);

	if (aMappedFile->hFile == INVALID_HANDLE_VALUE)
		CloseHandle(aMappedFile->hFile);
#else
	if (!aMappedFile)
		return;

	if (aMappedFile->data != MAP_FAILED)
		munmap(aMappedFile->data, aMappedFile->size);

	if (aMappedFile->fd != -1)
		close(aMappedFile->fd);
#endif
	free(aMappedFile);
}

void *BRMappedFileGetData(BRMappedFileRef aMappedFile)
{
#ifdef _WIN32
	return aMappedFile ? aMappedFile->lpMapAddress : 0;
#else
	return aMappedFile ? aMappedFile->data : 0;
#endif
}

size_t BRMappedFileGetSize(BRMappedFileRef aMappedFile)
{
#ifdef _WIN32
	return aMappedFile ? aMappedFile->dwFileSize : 0;
#else
	return aMappedFile ? aMappedFile->size : 0;
#endif
}
