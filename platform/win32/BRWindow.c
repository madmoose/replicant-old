#include <windows.h>

#include "BRWindow.h"

#include "BRCommon.h"

#include <stdlib.h>
#include <stdio.h>

struct BRWindow
{
	HWND hWnd;
};

void ErrorExit(LPTSTR lpszFunction);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BRWindowRef BRWindowCreate()
{
	BRWindowRef window = calloc(1, sizeof(struct BRWindow));

	if (!window)
		return 0;

	WNDCLASS wc;
	HDC hDC;
	HGLRC hRC;

	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra  = 0;
	wc.cbWndExtra  = 0;
	wc.hInstance   = 0;
	wc.hIcon       = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor     = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "ReplicantWindow";
	if (!RegisterClass(&wc))
		ErrorExit("CreateWindow");

	// create main window
	window->hWnd = CreateWindow(
		"ReplicantWindow", "Replicant",
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
		0, 0, 320, 240,
		NULL, NULL, wc.hInstance, NULL);
	if (!window->hWnd)
		ErrorExit("CreateWindow");

	return window;
}

HWND BRWindowGetHWnd(BRWindowRef aWindow)
{
	return aWindow->hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			return 0;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_DESTROY:
			return 0;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					return 0;
			}
			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process
	printf("Error in %s: %s\n", lpszFunction, lpMsgBuf);

	ExitProcess(dw);
}
