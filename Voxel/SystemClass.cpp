#include "Defined.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "InputClass.h"
#include "D3DClass.h"
#include "CameraClass.h"

SystemClass::SystemClass(HINSTANCE h)
{
	hInstance = h;
	appHandle = this;
}

bool SystemClass::Initialize()
{
	width = 800;
	height = 600;

	if (!InitializeWindows(width, height))
		return false;

	input = new InputClass;

	if (!input)
		return false;

	input->Initialize();

	graphics = new GraphicsClass;
	if (!graphics)
		return false;

	return graphics->Initialize(width, height, hWnd);
}

void SystemClass::Shutdown()
{
	if (graphics)
	{
		graphics->Shutdown();
		delete graphics;
		graphics = nullptr;
	}

	if (input)
	{
		delete input;
		input = nullptr;
	}

	ShutdownWindows();
}

void SystemClass::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	timer.Reset();

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			CameraClass *cam = graphics->GetCamera();
			float movespeed = 5.f;
			if (input->IsKeyDown('X'))
				cam->SetPosition(cam->GetPosition().x + cam->forward().x * movespeed * timer.DeltaTime(),
					cam->GetPosition().y + cam->forward().y * movespeed * timer.DeltaTime(),
					cam->GetPosition().z + cam->forward().z * movespeed * timer.DeltaTime());
			if (input->IsKeyDown('W'))
				cam->SetPosition(cam->GetPosition().x, cam->GetPosition().y + movespeed * timer.DeltaTime(), cam->GetPosition().z);
			if (input->IsKeyDown('S'))
				cam->SetPosition(cam->GetPosition().x, cam->GetPosition().y - movespeed * timer.DeltaTime(), cam->GetPosition().z);
			if (input->IsKeyDown('A'))
				cam->SetPosition(cam->GetPosition().x - movespeed * timer.DeltaTime(), cam->GetPosition().y, cam->GetPosition().z);
			if (input->IsKeyDown('D'))
				cam->SetPosition(cam->GetPosition().x + movespeed * timer.DeltaTime(), cam->GetPosition().y, cam->GetPosition().z);
			if (input->IsKeyDown('Q'))
				cam->SetPosition(cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z + movespeed * timer.DeltaTime());
			if (input->IsKeyDown('E'))
				cam->SetPosition(cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z - movespeed * timer.DeltaTime());
			if (input->IsKeyDown('Z'))
				cam->SetRotation(cam->GetRotation().x, cam->GetRotation().y - 180.f * timer.DeltaTime(), cam->GetRotation().z);
			if (input->IsKeyDown('C'))
				cam->SetRotation(cam->GetRotation().x, cam->GetRotation().y + 180.f * timer.DeltaTime(), cam->GetRotation().z);
			timer.Tick();

			if (!Frame())
				break;
		}
	}
}

LRESULT SystemClass::MsgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wparam) == WA_INACTIVE)
		{
			appPaused = true;
			timer.Stop();
		}
		else
		{
			appPaused = false;
			timer.Start();
		}
		return 0;
	case WM_SIZE:
		width = LOWORD(lparam);
		height = HIWORD(lparam);
		if (graphics != nullptr)
		{
			if (wparam == SIZE_MINIMIZED)
			{
				appPaused = true;
				minimized = true;
				maximized = false;
			}
			else if (wparam == SIZE_MAXIMIZED)
			{
				appPaused = false;
				minimized = false;
				maximized = true;
			}
			else if (wparam == SIZE_RESTORED)
			{
				if (minimized)
				{
					appPaused = false;
					minimized = false;
					//OnResize();
				}
				else if (maximized)
				{
					appPaused = false;
					maximized = false;
					//OnResize();
				}
				else if (resizing)
				{

				}
				else
				{
					//OnResize();
				}
			}
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		appPaused = true;
		resizing = true;
		timer.Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		appPaused = false;
		resizing = false;
		timer.Start();
		//OnResize();
		return 0;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lparam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lparam)->ptMinTrackSize.y = 200;
		return 0;
	//case WM_LBUTTONDOWN:
	//case WM_MBUTTONDOWN:
	//case WM_RBUTTONDOWN:
	//	OnMouseDown(wparam, GET_X_lparam(lparam), GET_Y_lparam(lparam));
	//	return 0;
	//case WM_LBUTTONUP:
	//case WM_MBUTTONUP:
	//case WM_RBUTTONUP:
	//	OnMouseUp(wparam, GET_X_lparam(lparam), GET_Y_lparam(lparam));
	//	return 0;
	//case WM_MOUSEMOVE:
	//	OnMouseMove(wparam, GET_X_lparam(lparam), GET_Y_lparam(lparam));
	//	return 0;
	case WM_KEYUP:
		if (wparam == VK_ESCAPE)
			PostQuitMessage(0);
		//else if ((int)wparam == VK_F2)
		//	Set4xMsaaState(!_4xMsaaState);
		return 0;
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool SystemClass::Frame()
{
	if (input->IsKeyDown(VK_ESCAPE))
		return false;

	return graphics->Frame();
}

bool SystemClass::InitializeWindows(int& sWidth, int& sHeight)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = appName;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, sWidth, sHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	hWnd = CreateWindow(appName, appName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
	if (!hWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return true;
}

void SystemClass::ShutdownWindows()
{
	DestroyWindow(hWnd);
	hWnd = NULL;

	UnregisterClass(appName, hInstance);
	hInstance = NULL;

	appHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return appHandle->MsgProc(hwnd, msg, wparam, lparam);
}