#ifndef BR_OPENGL_RENDERER_H
#define BR_OPENGL_RENDERER_H

#include "BREngine.h"
#include "BRWindow.h"

typedef struct BROpenGLRenderer * BROpenGLRendererRef;

BROpenGLRendererRef BROpenGLRendererCreate(BRWindowRef aWindow, BREngineRef aEngine);

void _BROpenGLRendererGetInfo(BROpenGLRendererRef aRenderer);
void BROpenGLRendererRenderFrame(BROpenGLRendererRef aRenderer, BRVideoFrameRef videoFrame);

#endif
