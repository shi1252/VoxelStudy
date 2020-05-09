#pragma once
#include "Defined.h"
#include "Timer.h"

class InputClass;
class GraphicsClass;

class SystemClass
{
public:
	SystemClass(HINSTANCE h);
	SystemClass(const SystemClass&) = delete;
	~SystemClass() = default;

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	bool InitializeWindows(int&, int&);
	void ShutdownWindows();

	LPCWSTR appName = L"Voxel";
	HINSTANCE hInstance;
	HWND hWnd;

	InputClass* input = nullptr;
	GraphicsClass* graphics = nullptr;

	Timer timer;

	int width;
	int height;
	bool appPaused;
	bool minimized;
	bool maximized;
	bool resizing;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemClass* appHandle = 0;