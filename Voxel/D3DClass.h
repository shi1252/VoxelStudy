#pragma once

#include "Defined.h"

class D3DClass
{
public:
	D3DClass() = default;
	D3DClass(const D3DClass&) = delete;
	~D3DClass() = default;

	bool Initialize(int width, int height, bool vsync, HWND hwnd, float screenFar, float screenNear);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProejctionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

private:
	bool vsync_enabled = false;
	int videoCardMemory = 0;
	char videoCardDescription[128] = { 0, };
	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11Texture2D* dsBuffer = nullptr;
	ID3D11DepthStencilState* dsState = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	ID3D11RasterizerState* rasterState = nullptr;

	XMMATRIX projMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX orthoMatrix;
};