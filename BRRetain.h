#ifndef BR_RETAIN_H
#define BR_RETAIN_H

typedef struct
{
	unsigned int retainCount;
} BRRetainable;

void BRRetain(void *);
void BRRelease(void *);

#endif
