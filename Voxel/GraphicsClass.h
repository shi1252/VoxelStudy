#pragma once
#include "Defined.h"

const bool VSYNC_ENABLED = true;
const float SCREEN_FAR = 1000.f;
const float SCREEN_NEAR = 0.1f;

class D3DClass;
class CameraClass;
class ModelClass;
class TextureShaderClass;
class Voxel;

class GraphicsClass
{
public:
	GraphicsClass() = default;
	GraphicsClass(const GraphicsClass&) = delete;
	~GraphicsClass() = default;

	bool Initialize(int width, int height, HWND hWnd);
	void Shutdown();
	bool Frame();

	D3DClass* GetD3DClass();
	CameraClass* GetCamera();

private:
	bool Render();

	D3DClass* d3d = nullptr;
	CameraClass* camera = nullptr;
	Voxel* voxel = nullptr;
	//ModelClass* model = nullptr;
	TextureShaderClass* shader = nullptr;
};