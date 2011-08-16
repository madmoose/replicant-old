#ifndef BR_WINDOW_H
#define BR_WINDOW_H

#include <windows.h>

typedef struct BRWindow * BRWindowRef;

BRWindowRef BRWindowCreate();

HWND BRWindowGetHWnd(BRWindowRef);

#endif
