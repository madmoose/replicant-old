#ifndef BR_OPENGL_RENDERER
#define BR_OPENGL_RENDERER

#include "BREngine.h"
#include "BRWindow.h"

typedef struct BROpenGLRenderer * BROpenGLRendererRef;

BROpenGLRendererRef BROpenGLRendererCreate(BRWindowRef aWindow, BREngineRef aEngine);

void _BROpenGLRendererGetInfo(BROpenGLRendererRef aRenderer);
void BROpenGLRendererRenderFrame(BROpenGLRendererRef aRenderer, BRVideoFrame frame);

#endif
