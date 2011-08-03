#ifndef BR_NUMBER_LIST_ITERATOR
#define BR_NUMBER_LIST_ITERATOR

#include "BRCommon.h"

typedef struct BRNumberListIterator * BRNumberListIteratorRef;

BRNumberListIteratorRef BRNumberListIteratorCreate(int begin, int end);
BRNumberListIteratorRef BRNumberListIteratorCreateWithString(const char *s);

BOOL _BRNumberListIteratorReadNumber(const char **p, int *n);
int BRNumberListIteratorGetNext(BRNumberListIteratorRef nli);

#endif
