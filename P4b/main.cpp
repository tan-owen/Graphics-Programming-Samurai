#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <cmath>
#include "blender_model.h"   // Must be included AFTER <Windows.h>

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")

#define WINDOW_TITLE "OpenGL Window"

// Rotation angles for inspecting the model with Arrow Keys
float rotX = 0.0f;
float rotY = 0.0f;

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		if (wParam == VK_LEFT)   rotY -= 5.0f;
		if (wParam == VK_RIGHT)  rotY += 5.0f;
		if (wParam == VK_UP)     rotX -= 5.0f;
		if (wParam == VK_DOWN)   rotX += 5.0f;
		break;

	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		if (height == 0) height = 1;

		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
		glMatrixMode(GL_MODELVIEW);
		break;
	}

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//--------------------------------------------------------------------

bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	int n = ChoosePixelFormat(hdc, &pfd);

	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------

void initGL()
{
	glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
	glEnable(GL_DEPTH_TEST); // Prevents back faces from drawing over front faces
	glEnable(GL_NORMALIZE);  // Keeps lighting accurate if you use glScalef

	// Basic lighting to shade your exported glNormal3f vectors
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	GLfloat lightPos[] = { 5.0f, 10.0f, 5.0f, 1.0f };
	GLfloat lightAmbient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	GLfloat lightDiffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

	// Set initial 3D perspective
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
}
//--------------------------------------------------------------------

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Position the camera back so the origin (0,0,0) is visible
	gluLookAt(0.0, 2.0, 6.0,   // Eye / camera position
		0.0, 0.0, 0.0,   // Target point
		0.0, 1.0, 0.0);  // Up axis

	// Rotate the scene with arrow keys
	glRotatef(rotX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotY, 0.0f, 1.0f, 0.0f);

	// Set base color and draw your Blender model
	glColor3f(0.8f, 0.8f, 0.8f);
	draw_all_objects();

	// Or draw & transform individual objects separately:
	// glPushMatrix();
	//     glTranslatef(0.0f, 1.0f, 0.0f);
	//     draw_Cube();
	// glPopMatrix();
}
//--------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	// Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	initPixelFormat(hdc);

	HGLRC hglrc = wglCreateContext(hdc);

	if (!wglMakeCurrent(hdc, hglrc)) return false;

	initGL(); // Configure depth buffer, lighting, and perspective

	//--------------------------------
	// End initialization
	//--------------------------------

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		display();

		SwapBuffers(hdc);
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	ReleaseDC(hWnd, hdc);
	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------