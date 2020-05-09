#pragma once
#include "Defined.h"
#include "AlignedAllocationPolicy.h"

class ColorShaderClass : public AlignedAllocationPolicy<16>
{
private:
	struct MatrixBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

public:
	ColorShaderClass() = default;
	ColorShaderClass(const ColorShaderClass&) = delete;
	~ColorShaderClass() = default;

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, int indexCount, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat);

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, LPCWSTR fileName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMsg, HWND hwnd, LPCWSTR fileName);

	bool SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat);
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* layout = nullptr;
	ID3D11Buffer* matBuffer = nullptr;
};