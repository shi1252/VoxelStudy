#include "VoxelShaderClass.h"
#include "Defined.h"
#include "LightClass.h"
#include "ShaderParameter.h"

bool VoxelShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	return InitializeShader(device, hwnd, L"Shader/VoxelShader.hlsl");
}

void VoxelShaderClass::Shutdown()
{
	ShutdownShader();
}

bool VoxelShaderClass::Render(ID3D11DeviceContext* context, int indexCount, ShaderParameter& params, ID3D11ShaderResourceView* texture)
{
	if (!SetShaderParameters(context, params, texture))
		return false;

	RenderShader(context, indexCount);

	return true;
}

bool VoxelShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, LPCWSTR fileName)
{
	ID3D10Blob* errorMsg = nullptr;

	// Compile vs code
	ID3D10Blob* vsBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(fileName, NULL, NULL, "VS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &vsBuffer, &errorMsg)))
	{
		// Print error message if the shader compile faied
		if (errorMsg)
			OutputShaderErrorMessage(errorMsg, hwnd, fileName);
		// Print error if the shader file couldn't find
		else
			MessageBox(hwnd, fileName, L"Missing Shader file", MB_OK);

		return false;
	}

	// Compile ps code
	ID3D10Blob* psBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(fileName, NULL, NULL, "PS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &psBuffer, &errorMsg)))
	{
		// Print error message if the shader compile faied
		if (errorMsg)
			OutputShaderErrorMessage(errorMsg, hwnd, fileName);
		// Print error if the shader file couldn't find
		else
			MessageBox(hwnd, fileName, L"Missing Shader file", MB_OK);

		return false;
	}

	// Create vs shader using buffer
	if (FAILED(device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), NULL, &vs)))
		return false;

	// Create ps shader using buffer
	if (FAILED(device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), NULL, &ps)))
		return false;

	// Initialize input layout desc
	// It has to equal with VertexIn
	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	// Get layout count
	UINT numElements = sizeof(inputLayout) / sizeof(inputLayout[0]);

	// Create input layout
	if (FAILED(device->CreateInputLayout(inputLayout, numElements,
		vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &layout)))
		return false;

	// Release vs buffer, ps buffer
	vsBuffer->Release();
	vsBuffer = nullptr;

	psBuffer->Release();
	psBuffer = nullptr;

	// Intialize constant buffer desc
	D3D11_BUFFER_DESC matBufferDesc;
	matBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matBufferDesc.MiscFlags = 0;
	matBufferDesc.StructureByteStride = 0;

	// Create constant buffer ptr to access vs constant buffer
	if (FAILED(device->CreateBuffer(&matBufferDesc, NULL, &matBuffer)))
		return false;

	matBufferDesc.ByteWidth = sizeof(LightBuffer);
	if (FAILED(device->CreateBuffer(&matBufferDesc, NULL, &lightBuffer)))
		return false;

	matBufferDesc.ByteWidth = sizeof(CameraBuffer);
	if (FAILED(device->CreateBuffer(&matBufferDesc, NULL, &camBuffer)))
		return false;

	// Initialize texture sampler desc
	D3D11_SAMPLER_DESC sd;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MipLODBias = 0.f;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	// Create texture sampler state
	if (FAILED(device->CreateSamplerState(&sd, &sampleState)))
		return false;

	return true;
}

void VoxelShaderClass::ShutdownShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = nullptr;
	}

	if (matBuffer)
	{
		matBuffer->Release();
		matBuffer = nullptr;
	}

	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = nullptr;
	}

	if (camBuffer)
	{
		camBuffer->Release();
		camBuffer = nullptr;
	}

	if (layout)
	{
		layout->Release();
		layout = nullptr;
	}

	if (ps)
	{
		ps->Release();
		ps = nullptr;
	}

	if (vs)
	{
		vs->Release();
		vs = nullptr;
	}
}

void VoxelShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMsg, HWND hwnd, LPCWSTR fileName)
{
	// Print debug message
	OutputDebugStringA(reinterpret_cast<const char*>(errorMsg->GetBufferPointer()));

	// Release errorMsg
	errorMsg->Release();
	errorMsg = nullptr;

	MessageBox(hwnd, L"Error compiling shader.", fileName, MB_OK);
}

bool VoxelShaderClass::SetShaderParameters(ID3D11DeviceContext* context, ShaderParameter& params, ID3D11ShaderResourceView* texture)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

#pragma region MatrixBuffer
	// Lock to write to constant buffer
	if (FAILED(context->Map(matBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	// Transpose matrix to shader can use it
	XMMATRIX worldMat = params.GetParam<XMMATRIX>("world");//XMMatrixTranspose(params.GetParam<XMMATRIX>("world"));
	XMMATRIX viewMat = XMMatrixTranspose(params.GetParam<XMMATRIX>("view"));
	XMMATRIX projMat = XMMatrixTranspose(params.GetParam<XMMATRIX>("proj"));

	// Get ptr of constant buffer data
	MatrixBuffer* data = (MatrixBuffer*)mappedResource.pData;

	// Set constant buffer data
	data->world = worldMat;
	data->view = viewMat;
	data->proj = projMat;

	// Unlock constant buffer
	context->Unmap(matBuffer, 0);
#pragma endregion

#pragma region LightBuffer
	if (FAILED(context->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	LightBuffer* data2 = (LightBuffer*)mappedResource.pData;

	data2->ambient = params.GetParam<LightClass*>("light")->GetAmbientColor();
	data2->diffuse = params.GetParam<LightClass*>("light")->GetDiffuseColor();
	data2->lightDir = params.GetParam<LightClass*>("light")->GetLightDirection();
	data2->padding = 0.f;

	context->Unmap(lightBuffer, 0);
#pragma endregion

#pragma region CameraBuffer
	if (FAILED(context->Map(camBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	CameraBuffer* data3 = (CameraBuffer*)mappedResource.pData;

	data3->camPos = params.GetParam<XMFLOAT4>("camPos");

	context->Unmap(camBuffer, 0);
#pragma endregion

	// Set constant buffer position value in vs
	UINT bufferNumber = 0;

	// Set constant buffer of vs
	context->VSSetConstantBuffers(bufferNumber++, 1, &matBuffer);
	context->VSSetConstantBuffers(bufferNumber++, 1, &camBuffer);

	// Set constant buffer of ps
	context->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set texture resource of ps
	context->PSSetShaderResources(0, 1, &texture);

	return true;
}

void VoxelShaderClass::RenderShader(ID3D11DeviceContext* context, int indexCount)
{
	// Set input layout
	context->IASetInputLayout(layout);

	// Set vs, ps
	context->VSSetShader(vs, NULL, 0);
	context->PSSetShader(ps, NULL, 0);

	// Set ps sampler state
	context->PSSetSamplers(0, 1, &sampleState);

	// Draw
	context->DrawIndexed(indexCount, 0, 0);
}