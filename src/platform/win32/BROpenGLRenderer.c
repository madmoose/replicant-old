#include <windows.h>

#include "BROpenGLRenderer.h"

#include "BREngine.h"
#include "BRWinCommon.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct BROpenGLRenderer
{
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
	BREngineRef engine;
	GLuint texture;
};

#define glError() { \
	GLenum err = glGetError(); \
	if (err != GL_NO_ERROR) { \
		while (err != GL_NO_ERROR) { \
			fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
			err = glGetError(); \
		} \
		exit(1); \
	} \
}

BROpenGLRendererRef BROpenGLRendererCreate(BRWindowRef aWindow, BREngineRef aEngine)
{
	BROpenGLRendererRef r = calloc(1, sizeof(struct BROpenGLRenderer));
	if (!r) return 0;

	r->hWnd   = BRWindowGetHWnd(aWindow);
	r->engine = aEngine;

	// set up opengl context
	PIXELFORMATDESCRIPTOR pfd;
	int format;

	// get the device context (DC)
	r->hDC = GetDC(r->hWnd);
	if (!r->hDC) return 0;

	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize      = sizeof(pfd);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(r->hDC, &pfd);
	SetPixelFormat(r->hDC, format, &pfd);

	// create and enable the render context (RC)
	r->hRC = wglCreateContext(r->hDC);
	if (!r->hRC)
		ErrorExit("wglCreateContext");

	wglMakeCurrent(r->hDC, r->hRC);
	glError();

	glGenTextures(1, &r->texture);
	glError();

	return r;
}

void _BROpenGLRendererGetInfo(BROpenGLRendererRef aRenderer)
{
	const char *vendor, *renderer, *version, *extensions;

	vendor = (const char*)glGetString(GL_VENDOR);
	if (!vendor) vendor = "Unknown";

	renderer = (const char*)glGetString(GL_RENDERER);
	if (!renderer) renderer = "Unknown";

	version = (const char*)glGetString(GL_VERSION);
	if (!version) version = "Unknown";

	extensions = (const char*)glGetString(GL_EXTENSIONS);
	if (!extensions) extensions = "Unknown";

	printf("Vendor:     %s\n", vendor);
	printf("Renderer:   %s\n", renderer);
	printf("Version:    %s\n", version);
	printf("Extensions:\n");

	if (extensions && *extensions)
		putchar('\t');
	const char *p = extensions;
	for (p = extensions; *p; ++p)
		if (*p == ' ')
			printf("\n\t");
		else
			putchar(*p);
	putchar('\n');
}

void BROpenGLRendererRenderFrame(BROpenGLRendererRef aRenderer, BRVideoFrameRef videoFrame)
{
	assert(videoFrame);
	BRDataRef videoData = BRVideoFrameGetData(videoFrame);
	assert(videoData);
	assert(BRDataGetBytes(videoData));

	BRSize frameStride = BRVideoFrameGetStride(videoFrame);
	BRSize frameSize   = BRVideoFrameGetSize(videoFrame);

	glBindTexture(GL_TEXTURE_2D, aRenderer->texture);
	glError();

	glTexImage2D(GL_TEXTURE_2D,       // target
	             0,                   // level
	             4,                   // internalFormat
	             frameStride.width,   // width
	             frameStride.height,  // height
	             0,                   // border
	             GL_RGBA,             // format
	             GL_UNSIGNED_BYTE,    // type
	             BRDataGetBytes(videoData)); // data
	glError();

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glError();



	glViewport(0, 0, 640, 480);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glError();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glError();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	float tx = (float)frameSize.width  / frameStride.width;
	float ty = (float)frameSize.height / frameStride.height;

	float vw = (float)frameSize.width;
	float wh = (float)frameSize.height;

	glBegin(GL_QUADS);
		glTexCoord2d( 0,  0);
		glVertex2f(   0,  0);

		glTexCoord2d(tx,  0);
		glVertex2f(  vw,  0);

		glTexCoord2d(tx, ty);
		glVertex2f(  vw, wh);

		glTexCoord2d( 0, ty);
		glVertex2f(   0, wh);
 	glEnd();
	glError();

 	SwapBuffers(aRenderer->hDC);
}
