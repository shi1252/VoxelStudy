#include "Defined.h"
#include "InputClass.h"

void InputClass::Initialize()
{
}

bool InputClass::IsKeyDown(int vKeyCode)
{
	return (GetAsyncKeyState(vKeyCode) & 0x8000) != 0;
}