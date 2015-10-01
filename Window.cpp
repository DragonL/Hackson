#include "Window.h"

void TerminateApplication(GL_Window* window)
{
	PostMessage(window->hWnd, WM_QUIT, 0, 0);
	g_isProgramLooping = FALSE;
} 

void ResizeWindowGL(int width, int height)								
{
	glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));				
	glMatrixMode(GL_PROJECTION);										
	glLoadIdentity();													
	glOrtho(-(float)width/2,(float)width/2,-(float)height/2,(float)height/2,-100,100);			
	glMatrixMode(GL_MODELVIEW);										
	glLoadIdentity();
}

BOOL ChangeScreenResolution(int width, int height, int bitsPerPixel)
{
	DEVMODE dmScreenSettings;											
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));						
	dmScreenSettings.dmSize			= sizeof(DEVMODE);
	dmScreenSettings.dmPelsWidth	= width;
	dmScreenSettings.dmPelsHeight	= height;
	dmScreenSettings.dmBitsPerPel	= bitsPerPixel;
	dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CreateWindowGL(GL_Window* window)
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,																
		PFD_DRAW_TO_WINDOW |											
		PFD_SUPPORT_OPENGL |											
		PFD_DOUBLEBUFFER,												
		PFD_TYPE_RGBA,													
		window->init.bitsPerPixel,										
		0, 0, 0, 0, 0, 0,												
		0,																
		0,																
		0,																
		0, 0, 0, 0,														
		16,																
		0,																
		0,																
		PFD_MAIN_PLANE,													
		0,																
		0, 0, 0															
	};

	RECT windowRect = {0, 0, window->init.width, window->init.height};

	GLuint PixelFormat;

	if (window->init.isFullScreen == TRUE)
	{
    /* L: does not support fullScreen mode.
		if (ChangeScreenResolution(window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			MessageBox(HWND_DESKTOP, "Mode Switch Failed,Running In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							
		}
		else
		{
			ShowCursor (FALSE);
			windowStyle = WS_POPUP;
			windowExtendedStyle |= WS_EX_TOPMOST;
		}
    */
	}
	else
	{
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	window->hWnd = CreateWindowEx(windowExtendedStyle,
									window->init.application->className,
									window->init.title,
									windowStyle,
									0, 0,
									windowRect.right - windowRect.left,
									windowRect.bottom - windowRect.top,	
									HWND_DESKTOP,
									0,
									window->init.application->hInstance,
									window);

	if (window->hWnd == 0)
	{
		return FALSE;
	}

	window->hDC = GetDC(window->hWnd);
	if (window->hDC == 0)	
	{
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	PixelFormat = ChoosePixelFormat(window->hDC, &pfd);
	if (PixelFormat == 0)
	{
		ReleaseDC(window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow(window->hWnd);
		window->hWnd = 0;
		return FALSE;
	}

	if (SetPixelFormat(window->hDC, PixelFormat, &pfd) == FALSE)
	{
		ReleaseDC(window->hWnd, window->hDC);
		window->hDC = 0;												
		DestroyWindow(window->hWnd);									
		window->hWnd = 0;												
		return FALSE;													
	}

	window->hRC = wglCreateContext(window->hDC);						
	if (window->hRC == 0)												
	{
		ReleaseDC(window->hWnd, window->hDC);							
		window->hDC = 0;												
		DestroyWindow(window->hWnd);									
		window->hWnd = 0;												
		return FALSE;													
	}

	if (wglMakeCurrent(window->hDC, window->hRC) == FALSE)				
	{
		wglDeleteContext(window->hRC);									
		window->hRC = 0;												
		ReleaseDC(window->hWnd, window->hDC);							
		window->hDC = 0;												
		DestroyWindow(window->hWnd);									
		window->hWnd = 0;												
		return FALSE;													
	}

	ShowWindow(window->hWnd, SW_NORMAL);								
	ResizeWindowGL(window->init.width, window->init.height);			
	ZeroMemory(window->keys, sizeof(Keys));								

	return TRUE;														
}

BOOL DestroyWindowGL(GL_Window* window)									
{
	if (window->hWnd != 0)												
	{	
		if (window->hDC != 0)											
		{
			wglMakeCurrent(window->hDC, 0);								
			if (window->hRC != 0)										
			{
				wglDeleteContext(window->hRC);							
				window->hRC = 0;										
			}
			ReleaseDC(window->hWnd, window->hDC);						
			window->hDC = 0;											
		}
		DestroyWindow(window->hWnd);									
		window->hWnd = 0;												
	}

	if (window->init.isFullScreen)										
	{
		ChangeDisplaySettings(NULL, 0);									
		ShowCursor(TRUE);												
	}	
	return TRUE;														
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	GL_Window* window = (GL_Window*)(GetWindowLong(hWnd, GWL_USERDATA));

	switch (uMsg)														
	{
	case WM_SYSCOMMAND:													
		{
			switch (wParam)												
			{
			case SC_SCREENSAVE:											
			case SC_MONITORPOWER:										
				return 0;												
			}
			break;														
		}

	case WM_CREATE:														
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));			
			return 0;													
		}
	
	case WM_CLOSE:														
		{
			PostMessage(window->hWnd, WM_QUIT, 0, 0);					
			g_isProgramLooping = FALSE;
			return 0;
		}

	case WM_SIZE:														
		switch (wParam)													
		{
		case SIZE_MINIMIZED:											
			window->isVisible = FALSE;									
			return 0;

		case SIZE_MAXIMIZED:											
			window->isVisible = TRUE;									
			ResizeWindowGL (LOWORD (lParam), HIWORD (lParam));			
			return 0;													

		case SIZE_RESTORED:												
			window->isVisible = TRUE;									
			ResizeWindowGL (LOWORD (lParam), HIWORD (lParam));			
			return 0;													
		}
		break;															

	case WM_KEYDOWN:													
		if ((wParam >= 0) && (wParam <= 255))							
		{
			window->keys->keyDown[wParam] = TRUE;						
			return 0;													
		}
		break;															

	case WM_KEYUP:														
		if ((wParam >= 0) && (wParam <= 255))							
		{
			window->keys->keyDown[wParam] = FALSE;						
			return 0;													
		}
		break;															

	case WM_TOGGLEFULLSCREEN:											
		{
			g_createFullScreen = !g_createFullScreen;
			PostMessage(hWnd, WM_QUIT, 0, 0);
			break;														
		}

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);					
}

BOOL RegisterWindowClass(Application* application)						
{	
	WNDCLASSEX windowClass;												
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));						
	windowClass.cbSize		= sizeof(WNDCLASSEX);						
	windowClass.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;		
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				
	windowClass.hInstance		= application->hInstance;				
	windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			
	windowClass.lpszClassName	= application->className;				
	if (RegisterClassEx(&windowClass) == 0)								
	{
		MessageBox(HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													
	}
	return TRUE;														
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application			application;									
	GL_Window			window;											
	Keys				keys;											
	MSG					msg;											
	BOOL				isMessagePumpActive;

	
	application.className = CLASSNAME;									
	application.hInstance = hInstance;									

	
	ZeroMemory(&window, sizeof(GL_Window));								
	window.keys					= &keys;								
	window.init.application		= &application;							
	window.init.title			= TITLE;								
	window.init.width			= WIDTH;								
	window.init.height			= HEIGHT;								
	window.init.bitsPerPixel	= BPP;									
	window.init.isFullScreen	= FALSE;								
	ZeroMemory(&keys, sizeof(Keys));									

	if (RegisterWindowClass(&application) == FALSE)						
	{
		MessageBox(HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														
	}

	g_isProgramLooping = TRUE;											
	g_createFullScreen = window.init.isFullScreen;						
	while (g_isProgramLooping)											
	{
		
		window.init.isFullScreen = g_createFullScreen;					
		if (CreateWindowGL(&window) == TRUE)							
		{
			if (Initialize(&window, &keys) == FALSE)					
			{
				
				PostMessage(window.hWnd, WM_QUIT, 0, 0);				
				g_isProgramLooping = FALSE;
			}
			else
			{	
				isMessagePumpActive = TRUE;								
				while (isMessagePumpActive == TRUE)						
				{
					
					if (PeekMessage(&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						
						if (msg.message != WM_QUIT)						
						{
							DispatchMessage(&msg);						
						}
						else											
						{
							isMessagePumpActive = FALSE;				
						}
					}
					else												
					{
						if (window.isVisible == FALSE)					
						{
							WaitMessage();								
						}
						else											
						{
							Update();									
							DrawSceneGL();								
							SwapBuffers(window.hDC);					
						}
					}
				}														
			}

			
			Deinitialize();												
			DestroyWindowGL(&window);									
		}
		else															
		{
			MessageBox(HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									
		}
	}																	
	UnregisterClass(application.className, application.hInstance);		
	return 0;
}