#pragma once
//#include <dinput.h>
class InputClass
{
public:
	InputClass() = default;
	InputClass(const InputClass&) = delete;
	~InputClass() = default;

	void Initialize();
	bool IsKeyDown(int vKeyCode);

//	bool Initialize(HINSTANCE hInstance, HWND hWnd, int width, int height);
//	void Shutdown();
//	bool Frame();
//
//	bool IsEscapePressed();
//	void GetMouseLocation(int& x, int& y);
//
//private:
//	bool ReadKeyboard();
//	bool ReadMouse();
//	void ProcessInput();
//
//	IDirectInput8* directInput = nullptr;
//	IDirectInputDevice8* keyboard = nullptr;
//	IDirectInputDevice8* mouse = nullptr;
};