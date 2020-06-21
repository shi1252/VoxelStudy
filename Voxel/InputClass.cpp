#include "Defined.h"
#include "InputClass.h"
#include "MathHelper.h"

bool InputClass::IsKeyDown(unsigned char keyCode)
{
	return (keyboardState[keyCode] & 0x80);
}

bool InputClass::Initialize(HINSTANCE hInstance, HWND hWnd, int width, int height)
{
	this->width = width;
	this->height = height;

	// Initialize direct input interface
	if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL)))
		return false;

	// Create keyboard direct input interface
	if (FAILED(directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL)))
		return false;

	// Set data format for keyboard
	if (FAILED(keyboard->SetDataFormat(&c_dfDIKeyboard)))
		return false;

	// Set cooperative level to not share input with other program
	if (FAILED(keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
		return false;

	// Acquire keyboard
	if (FAILED(keyboard->Acquire()))
		return false;

	// Create mouse direct input interface
	if (FAILED(directInput->CreateDevice(GUID_SysMouse, &mouse, NULL)))
		return false;

	// Set data format for mouse
	if (FAILED(mouse->SetDataFormat(&c_dfDIMouse)))
		return false;

	// Set cooperative level to not share input with other program
	if (FAILED(mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return false;

	// Acquire mouse
	if (FAILED(mouse->Acquire()))
		return false;

	if (ReadMouse())
	{
		prevMouseState = mouseState;
		init = false;
	}

	return true;
}

void InputClass::Shutdown()
{
	// Release mouse
	if (mouse)
	{
		mouse->Unacquire();
		mouse->Release();
		mouse = nullptr;
	}

	// Release keyboard
	if (keyboard)
	{
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}

	// Release direct input object
	if (directInput)
	{
		directInput->Release();
		directInput = nullptr;
	}
}

bool InputClass::Frame()
{
	// Read current keyboard state
	if (!ReadKeyboard())
		return false;

	// Read current mouse state
	if (!ReadMouse())
		return false;

	ProcessInput();

	return true;
}

void InputClass::GetMouseLocation(int& x, int& y)
{
	x = mouseX;
	y = mouseY;
}

void InputClass::GetMouseDelta(int& x, int& y)
{
	x = mouseState.lX;
	y = mouseState.lY;
}

bool InputClass::GetMouseButtonState(MState button)
{
	return (mouseState.rgbButtons[button] & 0x80);
}

bool InputClass::GetMouseButtonDown(MState button)
{
	return !(prevMouseState.rgbButtons[button] & 0x80) && (mouseState.rgbButtons[button] & 0x80);
}

bool InputClass::GetMouseButtonUp(MState button)
{
	return (prevMouseState.rgbButtons[button] & 0x80) && !(mouseState.rgbButtons[button] & 0x80);
}

bool InputClass::ReadKeyboard()
{
	// Get keyboard device
	HRESULT result = keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if (FAILED(result))
	{
		// Get control if the keyboard lose focus or doesn't acquire
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			keyboard->Acquire();
		else
			return false;
	}

	return true;
}

bool InputClass::ReadMouse()
{
	if (!init)
		prevMouseState = mouseState;
	// Get mouse device
	HRESULT result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState);
	if (FAILED(result))
	{
		// Get control if the mouse lose focus or doesn't acquire
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			mouse->Acquire();
		else
			return false;
	}

	return true;
}

void InputClass::ProcessInput()
{
	// Update mouse cursor position based on delta position between frame
	mouseX += mouseState.lX;
	mouseY += mouseState.lY;

	// Clamp mouse position
	mouseX = MathHelper::Clamp(mouseX, 0, width);
	mouseY = MathHelper::Clamp(mouseY, 0, height);
}