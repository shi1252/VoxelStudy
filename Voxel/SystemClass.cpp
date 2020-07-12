#include "Defined.h"
#include "SystemClass.h"
#include "GraphicsClass.h"
#include "InputClass.h"
#include "LineClass.h"
#include "D3DClass.h"
#include "CameraClass.h"
#include "Ray3D.h"
#include "Voxel.h"
#include "MathHelper.h"

using namespace MathHelper;

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

	if (!input->Initialize(hInstance, hWnd, width, height))
	{
		MessageBox(hWnd, L"Could not initialize input object.", L"Error", MB_OK);
		return false;
	}

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
		input->Shutdown();
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
			timer.Tick();

			if (!Frame())
				break;
		}

		if (input->IsKeyDown(DIK_ESCAPE))
			break;
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
	if (!input->Frame())
		return false;

	int mouseX = 0, mouseY = 0;
	input->GetMouseLocation(mouseX, mouseY);

	if (!graphics->Frame(mouseX, mouseY))
		return false;

#pragma region CameraControl
	CameraClass* cam = graphics->GetCamera();
	cam->Render();
	float movespeed = 5.f;
	if (input->IsKeyDown(DIK_LSHIFT))
		movespeed = 50.f;
	if (input->IsKeyDown(DIK_W))
		cam->SetPosition(cam->GetPosition().x + cam->forward().x * movespeed * timer.DeltaTime(),
			cam->GetPosition().y + cam->forward().y * movespeed * timer.DeltaTime(),
			cam->GetPosition().z + cam->forward().z * movespeed * timer.DeltaTime());
	if (input->IsKeyDown(DIK_S))
		cam->SetPosition(cam->GetPosition().x - cam->forward().x * movespeed * timer.DeltaTime(),
			cam->GetPosition().y - cam->forward().y * movespeed * timer.DeltaTime(),
			cam->GetPosition().z - cam->forward().z * movespeed * timer.DeltaTime());
	if (input->IsKeyDown(DIK_A))
		cam->SetPosition(cam->GetPosition().x - cam->right().x * movespeed * timer.DeltaTime(),
			cam->GetPosition().y - cam->right().y * movespeed * timer.DeltaTime(),
			cam->GetPosition().z - cam->right().z * movespeed * timer.DeltaTime());
	if (input->IsKeyDown(DIK_D))
		cam->SetPosition(cam->GetPosition().x + cam->right().x * movespeed * timer.DeltaTime(),
			cam->GetPosition().y + cam->right().y * movespeed * timer.DeltaTime(),
			cam->GetPosition().z + cam->right().z * movespeed * timer.DeltaTime());

	if (input->GetMouseButtonState(input->RIGHT))
	{
		float mouseSensitivity = 5.f;
		int dx = 0, dy = 0;
		input->GetMouseDelta(dx, dy);
		
		cam->SetRotation(cam->GetRotation().x, cam->GetRotation().y + dx * mouseSensitivity * timer.DeltaTime(), cam->GetRotation().z);
		cam->SetRotation(cam->GetRotation().x + dy * mouseSensitivity * timer.DeltaTime(), cam->GetRotation().y, cam->GetRotation().z);
	}

	bool draw = true;
	if (input->IsKeyDown(DIK_LCONTROL))
		draw = false;

	if (input->GetMouseButtonState(input->LEFT))
	{
		XMFLOAT3 origin = cam->GetPosition();//graphics->GetScreenToWorldPoint(mouseX, mouseY, 0.f);
		XMFLOAT3 target = graphics->GetScreenToWorldPoint(mouseX, mouseY, 1.f);

		Ray3D ray(origin, target);

		XMFLOAT3 out = XMFLOAT3(0, 0, 0);
		if (graphics->GetVoxel()->RayCast(ray, out))
			graphics->GetVoxel()->SetVoxelSphere(out, 1.f, draw);
	}

	if (input->IsKeyDown(DIK_Z))
	{
		XMFLOAT3 origin = cam->GetPosition();//graphics->GetScreenToWorldPoint(mouseX, mouseY, 0.f);
		XMFLOAT3 target = graphics->GetScreenToWorldPoint(mouseX, mouseY, 1.f);

		Ray3D ray(origin, target);

		XMFLOAT3 out = XMFLOAT3(0, 0, 0);
		if (graphics->GetVoxel()->RayCast(ray, out))
			graphics->GetVoxel()->SetNavStart(out);
	}

	if (input->IsKeyDown(DIK_X))
	{
		XMFLOAT3 origin = cam->GetPosition();//graphics->GetScreenToWorldPoint(mouseX, mouseY, 0.f);
		XMFLOAT3 target = graphics->GetScreenToWorldPoint(mouseX, mouseY, 1.f);

		Ray3D ray(origin, target);

		XMFLOAT3 out = XMFLOAT3(0, 0, 0);
		if (graphics->GetVoxel()->RayCast(ray, out))
			graphics->GetVoxel()->SetNavEnd(out);
	}

	if (input->IsKeyDown(DIK_X))
	{
		std::vector<XMFLOAT3> path;
		graphics->GetVoxel()->VoxelAStar(path);
		for (int i = 0; i < path.size(); ++i)
		{
			LineClass::VertexType v = { path[i] + XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT4(1.f, 0.f, 0.f, 1.f) };
			graphics->line->vertices.push_back(v);
		}
		for (int i = 0; i < graphics->line->vertices.size(); ++i)
			graphics->line->indices.push_back(i);
	}

	if (input->IsKeyDown(DIK_C))
	{
		std::vector<XMFLOAT3> path;
		graphics->GetVoxel()->VoxelAStar(path);
		graphics->GetVoxel()->PathOptimization(path);
		for (int i = 0; i < path.size(); ++i)
		{
			LineClass::VertexType v = { path[i] + XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT4(1.f, 0.f, 0.f, 1.f) };
			graphics->line->vertices.push_back(v);
		}
		for (int i = 0; i < graphics->line->vertices.size(); ++i)
			graphics->line->indices.push_back(i);
	}
#pragma endregion

	return graphics->Render();
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