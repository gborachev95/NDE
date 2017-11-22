#include "includes.h"
#include "Game.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
//#include <vld.h>
#define DUMP 0

#if DUMP
#include <DbgHelp.h>
#endif
// Prototypes
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#if DUMP
LONG WINAPI errorFunc(_EXCEPTION_POINTERS *pExceptionInfo);
#endif
//Application* myAppAccessor;
// Main loop 
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
	srand(unsigned int(time(0)));
#if DUMP
	SetUnhandledExceptionFilter(errorFunc);
#endif
	Game NearDeathExpirience(hInstance, (WNDPROC)WndProc);
	
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && NearDeathExpirience.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	NearDeathExpirience.Shutdown();
	return 0;
}

// Callback function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//if (GetAsyncKeyState(VK_ESCAPE))
	//	message = WM_DESTROY;

	static bool keypressed = false;
	if (Graphics::single_keyboard->GetState().F1 && !keypressed)
	{
		keypressed = true;
		Graphics::ResizeWindow();
	}

	if (!Graphics::single_keyboard->GetState().F1)
		keypressed = false;

	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
		break;
	case WM_SIZE:
		break;
	case WM_ACTIVATEAPP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	case WM_LBUTTONDOWN:  //Left button down
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_LBUTTONUP:	  //Left button up
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_MBUTTONDOWN:  //Middle button down
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_MBUTTONUP:    //Middle button up
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_RBUTTONDOWN:  //Right button down
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_RBUTTONUP:    //Right button up
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_SETCURSOR:

		//// Turn off window cursor 
		//SetCursor(NULL);
		//
		//ShowCursor(TRUE);
		//
		//return TRUE; // prevent Windows from setting cursor to window class cursor

		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#if DUMP
LONG WINAPI errorFunc(_EXCEPTION_POINTERS *pExceptionInfo)
{
	HANDLE hFile = ::CreateFile(L"NDEdumpfile.mdmp", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = pExceptionInfo;
		ExInfo.ClientPointers = NULL;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		//MessageBox("Dump File Saved look x directory please email to developer at the following email adress crashdmp@gmail.com with the subject Gamename - Version ");
		::CloseHandle(hFile);
	}

	return 0;
}
#endif