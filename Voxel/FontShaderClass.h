#pragma once
#include "Defined.h"

class FontShaderClass
{
private:
	struct ConstantBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	struct PixelBufferType
	{
		XMFLOAT4 pixelColor;
	};

public:
	FontShaderClass() = default;
	FontShaderClass(const FontShaderClass&) = delete;
	~FontShaderClass() = default;

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, int indexCount, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor);

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, LPCWSTR fileName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMsg, HWND hwnd, LPCWSTR fileName);

	bool SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor);
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* layout = nullptr;
	ID3D11Buffer* cb = nullptr;
	ID3D11SamplerState* sampleState = nullptr;
	ID3D11Buffer* pb = nullptr;
};