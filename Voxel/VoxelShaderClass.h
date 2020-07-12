#pragma once
#include "Defined.h"
#include "AlignedAllocationPolicy.h"

class ShaderParameter;

class VoxelShaderClass : public AlignedAllocationPolicy<16>
{
private:
	struct MatrixBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	struct LightBuffer
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 lightDir;
		//float specularPower;
		//XMFLOAT4 specular;
		float padding;
	};

	struct CameraBuffer
	{
		XMFLOAT4 camPos;
	};

public:
	VoxelShaderClass() = default;
	VoxelShaderClass(const VoxelShaderClass&) = delete;
	~VoxelShaderClass() = default;

	bool Initialize(ID3D11Device * device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext * context, int indexCount, ShaderParameter& params, ID3D11ShaderResourceView** srv);

private:
	bool InitializeShader(ID3D11Device * device, HWND hwnd, LPCWSTR fileName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob * errorMsg, HWND hwnd, LPCWSTR fileName);

	bool SetShaderParameters(ID3D11DeviceContext * context, ShaderParameter& params, ID3D11ShaderResourceView** srv);
	void RenderShader(ID3D11DeviceContext * context, int indexCount);

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* layout = nullptr;
	ID3D11Buffer* matBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* camBuffer = nullptr;

	ID3D11SamplerState* sampleState = nullptr;
};