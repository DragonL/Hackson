#ifndef _5DG_WINDOW_H
#define _5DG_WINDOW_H

#include <windows.h>													
#include <gl\gl.h>														
#include <gl\glu.h>														

#define WM_TOGGLEFULLSCREEN (WM_USER + 1)								
#define TITLE		"Vioce Control App."									
#define CLASSNAME	"5DG_OPENGL"										
#define	WIDTH		800													
#define HEIGHT  600													
#define BPP			16													


typedef struct {														
	BOOL keyDown [256];													
} Keys;

typedef struct {														
	HINSTANCE		hInstance;											
	const char*		className;											
} Application;

typedef struct {														
	Application*	application;										
	char*			title;												
	int				width;												
	int				height;												
	int				bitsPerPixel;										
	BOOL			isFullScreen;										
} GL_WindowInit;

typedef struct {														
	Keys*			keys;												
	HWND			hWnd;												
	HDC				hDC;												
	HGLRC			hRC;												
	GL_WindowInit	init;												
	BOOL			isVisible;											
} GL_Window;

static BOOL g_isProgramLooping;
static BOOL g_createFullScreen;


void TerminateApplication (GL_Window* window);							
BOOL Initialize(GL_Window* window, Keys* keys);							
void DrawSceneGL(void);													
void Update(void);														
void Deinitialize(void);												

#endif																	