#include "BRNumberListIterator.h"

#include <ctype.h>
#include <stdlib.h>

struct BRNumberListIterator
{
	const char *s;
	const char *p;
	int  rangeBegin;
	int  rangeEnd;
};

BRNumberListIteratorRef BRNumberListIteratorCreate(int begin, int end)
{
	BRNumberListIteratorRef nli = calloc(1, sizeof(struct BRNumberListIterator));

	nli->rangeBegin =  begin;
	nli->rangeEnd   =  end;

	return nli;
}

BRNumberListIteratorRef BRNumberListIteratorCreateWithString(const char *s)
{
	BRNumberListIteratorRef nli = calloc(1, sizeof(struct BRNumberListIterator));

	nli->s = nli->p = s;
	nli->rangeBegin =  0;
	nli->rangeEnd   = -1;

	return nli;
}

BOOL _BRNumberListIteratorReadNumber(const char **p, int *n)
{
	if (!*p || !isdigit(**p))
		return NO;

	*n = 0;
	while (isdigit(**p))
	{
		*n = 10 * *n + ((**p) - '0');

		++*p;
	}

	if (**p == ',')
		++*p;

	return YES;
}

int BRNumberListIteratorGetNext(BRNumberListIteratorRef nli)
{
	if (nli->rangeBegin <= nli->rangeEnd)
		return nli->rangeBegin++;

	int n;
	if (!_BRNumberListIteratorReadNumber(&nli->p, &n))
		return -1;

	if (*nli->p == '-')
	{
		++nli->p;

		int n2;
		if (!_BRNumberListIteratorReadNumber(&nli->p, &n2))
			return -1;

		nli->rangeBegin = n;
		nli->rangeEnd   = n2;

		return nli->rangeBegin++;
	}

	return n;
}
