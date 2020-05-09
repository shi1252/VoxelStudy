#include "Defined.h"
#include "ColorShaderClass.h"

bool ColorShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	return InitializeShader(device, hwnd, L"Shader/ColorShader.hlsl");
}

void ColorShaderClass::Shutdown()
{
	ShutdownShader();
}

bool ColorShaderClass::Render(ID3D11DeviceContext* context, int indexCount, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat)
{
	if (!SetShaderParameters(context, worldMat, viewMat, projMat))
		return false;

	RenderShader(context, indexCount);

	return true;
}

bool ColorShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, LPCWSTR fileName)
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
	D3D11_INPUT_ELEMENT_DESC inputLayout[2];
	inputLayout[0].SemanticName = "POSITION";
	inputLayout[0].SemanticIndex = 0;
	inputLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayout[0].InputSlot = 0;
	inputLayout[0].AlignedByteOffset = 0;
	inputLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout[0].InstanceDataStepRate = 0;

	inputLayout[1].SemanticName = "COLOR";
	inputLayout[1].SemanticIndex = 0;
	inputLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputLayout[1].InputSlot = 0;
	inputLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout[1].InstanceDataStepRate = 0;

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

	return true;
}

void ColorShaderClass::ShutdownShader()
{
	if (matBuffer)
	{
		matBuffer->Release();
		matBuffer = nullptr;
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

void ColorShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMsg, HWND hwnd, LPCWSTR fileName)
{
	// Print debug message
	OutputDebugStringA(reinterpret_cast<const char*>(errorMsg->GetBufferPointer()));

	// Release errorMsg
	errorMsg->Release();
	errorMsg = nullptr;

	MessageBox(hwnd, L"Error compiling shader.", fileName, MB_OK);
}

bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMat, XMMATRIX viewMat, XMMATRIX projMat)
{
	// Transpose matrix to shader can use it
	worldMat = XMMatrixTranspose(worldMat);
	viewMat = XMMatrixTranspose(viewMat);
	projMat = XMMatrixTranspose(projMat);

	// Lock to write to constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(matBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	// Get ptr of constant buffer data
	MatrixBuffer* data = (MatrixBuffer*)mappedResource.pData;

	// Set constant buffer data
	data->world = worldMat;
	data->view = viewMat;
	data->proj = projMat;

	// Unlock constant buffer
	context->Unmap(matBuffer, 0);

	// Set constant buffer position value in vs
	UINT bufferNumber = 0;

	// Set constant buffer of vs
	context->VSSetConstantBuffers(bufferNumber, 1, &matBuffer);

	return true;
}

void ColorShaderClass::RenderShader(ID3D11DeviceContext* context, int indexCount)
{
	// Set input layout
	context->IASetInputLayout(layout);

	// Set vs, ps
	context->VSSetShader(vs, NULL, 0);
	context->PSSetShader(ps, NULL, 0);

	// Draw
	context->DrawIndexed(indexCount, 0, 0);
}