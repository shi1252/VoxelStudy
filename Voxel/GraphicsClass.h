#pragma once
#include "Defined.h"
#include <vector>

const bool VSYNC_ENABLED = true;
const float SCREEN_FAR = 1000.f;
const float SCREEN_NEAR = 0.1f;

class D3DClass;
class CameraClass;
class LightClass;
class ModelClass;
class TextureShaderClass;
class VoxelShaderClass;
class ColorShaderClass;
class Voxel;
class TextClass;
class LineClass;
class TextureClass;

class GraphicsClass
{
public:
	GraphicsClass() = default;
	GraphicsClass(const GraphicsClass&) = delete;
	~GraphicsClass() = default;

	bool Initialize(int width, int height, HWND hWnd);
	void Shutdown();
	bool Frame(int x, int y);
	bool Render();

	D3DClass* GetD3DClass();
	CameraClass* GetCamera();
	Voxel* GetVoxel();

	XMFLOAT3 GetScreenToWorldPoint(int& x, int& y, float z);

	LineClass* line = nullptr;
	static GraphicsClass* GetInstance() { return instance; }

	TextureClass* GetTextures() { return textures; }

private:
	static GraphicsClass* instance;

	D3DClass* d3d = nullptr;
	CameraClass* camera = nullptr;
	Voxel* voxel = nullptr;
	LightClass* light = nullptr;
	//ModelClass* model = nullptr;
	//TextureShaderClass* shader = nullptr;
	VoxelShaderClass* shader = nullptr;
	TextClass* text = nullptr;
	ColorShaderClass* colorShader = nullptr;

	TextureClass* textures;

	int width = 0;
	int height = 0;
};