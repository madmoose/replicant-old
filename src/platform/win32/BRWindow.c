#include <windows.h>

#include "BRWindow.h"
#include "BRWinCommon.h"

#include "BRCommon/BRCommon.h"

#include <stdlib.h>
#include <stdio.h>

struct BRWindow
{
	HWND hWnd;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BRWindowRef BRWindowCreate()
{
	BRWindowRef window = calloc(1, sizeof(struct BRWindow));

	if (!window)
		return 0;

	WNDCLASS wc;

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
		0, 0, 640, 480,
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
