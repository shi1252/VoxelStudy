#include "FontShaderClass.h"
#include "Defined.h"

bool FontShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	return InitializeShader(device, hwnd, L"Shader/FontShader.hlsl");
}

void FontShaderClass::Shutdown()
{
	ShutdownShader();
}

bool FontShaderClass::Render(ID3D11DeviceContext* context, int indexCount, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
	// Set shader parameters
	if (!SetShaderParameters(context, worldMat, viewMat, projMat, texture, pixelColor))
		return false;

	RenderShader(context, indexCount);

	return true;
}

bool FontShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, LPCWSTR fileName)
{
	ID3D10Blob* errorMsg = nullptr;

	// Compile vs code
	ID3D10Blob* vsb = nullptr;
	if (FAILED(D3DCompileFromFile(fileName, NULL, NULL, "VS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &vsb, &errorMsg)))
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
	ID3D10Blob* psb= nullptr;
	if (FAILED(D3DCompileFromFile(fileName, NULL, NULL, "PS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
		0, &psb, &errorMsg)))
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
	if (FAILED(device->CreateVertexShader(vsb->GetBufferPointer(), vsb->GetBufferSize(), NULL, &vs)))
		return false;

	// Create ps shader using buffer
	if (FAILED(device->CreatePixelShader(psb->GetBufferPointer(), psb->GetBufferSize(), NULL, &ps)))
		return false;

	// Initialize input layout desc
	// It has to equal with VertexIn
	D3D11_INPUT_ELEMENT_DESC inputLayout[2];
	inputLayout[0].SemanticName = "POSITION";
	inputLayout[0].SemanticIndex = 0;
	inputLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayout[0].InputSlot = 0;
	inputLayout[0].AlignedByteOffset = 0;
	inputLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout[0].InstanceDataStepRate = 0;

	inputLayout[1].SemanticName = "TEXCOORD";
	inputLayout[1].SemanticIndex = 0;
	inputLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout[1].InputSlot = 0;
	inputLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout[1].InstanceDataStepRate = 0;

	// Get layout count
	UINT numElements = sizeof(inputLayout) / sizeof(inputLayout[0]);

	// Create input layout
	if (FAILED(device->CreateInputLayout(inputLayout, numElements,
		vsb->GetBufferPointer(), vsb->GetBufferSize(), &layout)))
		return false;

	// Release vs buffer, ps buffer
	vsb->Release();
	vsb = nullptr;

	psb->Release();
	psb = nullptr;

	// Intialize constant buffer desc
	D3D11_BUFFER_DESC matBufferDesc;
	matBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matBufferDesc.ByteWidth = sizeof(ConstantBufferType);
	matBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matBufferDesc.MiscFlags = 0;
	matBufferDesc.StructureByteStride = 0;

	// Create constant buffer ptr to access vs constant buffer
	if (FAILED(device->CreateBuffer(&matBufferDesc, NULL, &cb)))
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

	// Initialize pixel buffer desc
	D3D11_BUFFER_DESC pbd;
	pbd.Usage = D3D11_USAGE_DYNAMIC;
	pbd.ByteWidth = sizeof(PixelBufferType);
	pbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pbd.MiscFlags = 0;
	pbd.StructureByteStride = 0;

	// Create pixel buffer ptr to access ps pixel buffer
	if (FAILED(device->CreateBuffer(&pbd, NULL, &pb)))
		return false;

	return true;
}

void FontShaderClass::ShutdownShader()
{
	if (pb)
	{
		pb->Release();
		pb = nullptr;
	}

	if (sampleState)
	{
		sampleState->Release();
		sampleState = nullptr;
	}

	if (cb)
	{
		cb->Release();
		cb = nullptr;
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

void FontShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMsg, HWND hwnd, LPCWSTR fileName)
{
	// Print debug message
	OutputDebugStringA(reinterpret_cast<const char*>(errorMsg->GetBufferPointer()));

	// Release errorMsg
	errorMsg->Release();
	errorMsg = nullptr;

	MessageBox(hwnd, L"Error compiling shader.", fileName, MB_OK);
}

bool FontShaderClass::SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
	// Lock to write to constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	// Transpose matrix to shader can use it
	worldMat = XMMatrixTranspose(worldMat);
	viewMat = XMMatrixTranspose(viewMat);
	projMat = XMMatrixTranspose(projMat);

	// Get ptr of constant buffer data
	ConstantBufferType* data = (ConstantBufferType*)mappedResource.pData;

	// Set constant buffer data
	data->world = worldMat;
	data->view = viewMat;
	data->proj = projMat;

	// Unlock constant buffer
	context->Unmap(cb, 0);

	// Set constant buffer position value in vs
	UINT bufferNumber = 0;

	// Set constant buffer of vs
	context->VSSetConstantBuffers(bufferNumber, 1, &cb);

	// Set texture resource of ps
	context->PSSetShaderResources(0, 1, &texture);

	// Lock pixel constant buffer
	if (FAILED(context->Map(pb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	// Get ptr of pixel constant buffer data
	PixelBufferType* pbData = (PixelBufferType*)mappedResource.pData;

	// Set pixel constant buffer data
	pbData->pixelColor = pixelColor;

	// Unlock pixel constant buffer
	context->Unmap(pb, 0);

	// Set pixel constant buffer position value in ps
	bufferNumber = 0;

	// Set pixel constant buffer of ps
	context->PSSetConstantBuffers(bufferNumber, 1, &pb);

	return true;
}

void FontShaderClass::RenderShader(ID3D11DeviceContext* context, int indexCount)
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