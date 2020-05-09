#include "Defined.h"
#include "SystemClass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SystemClass* system = new SystemClass(hInstance);
	if (!system)
		return -1;

	if (system->Initialize())
		system->Run();

	system->Shutdown();
	delete system;
	system = nullptr;

	return 0;
}