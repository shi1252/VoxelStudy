#include "LineClass.h"

bool LineClass::Initialize(ID3D11Device* device)
{
	// Initialize vertex index buffer
	return InitializeBuffers(device);
}

void LineClass::Shutdown()
{
	ShutdownBuffers();
}

void LineClass::Render(ID3D11DeviceContext* context)
{
	// Put vb, ib to pipeline to draw
	RenderBuffers(context);
}

int LineClass::GetIndexCount()
{
	return iCount;
}

bool LineClass::InitializeBuffers(ID3D11Device* device)
{
	if (vertices.empty() || indices.empty())
		return true;

	vCount = vertices.size();
	iCount = indices.size();

	// Initialize vertex buffer desc
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexType) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Set ptr to subresource about vertext data
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = &vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create vertex buffer
	if (FAILED(device->CreateBuffer(&vbd, &vertexData, &vb)))
		return false;

	// Initialize index buffer desc
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(ULONG) * indices.size();
	ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Set ptr to subresource about index data
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = &indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create index buffer
	if (FAILED(device->CreateBuffer(&ibd, &indexData, &ib)))
		return false;

	vCount = vertices.size();
	iCount = indices.size();

	// Release vertices, indices
	vertices.clear();
	indices.clear();

	return true;
}

void LineClass::ShutdownBuffers()
{
	// Release index buffer
	if (ib)
	{
		ib->Release();
		ib = nullptr;
	}

	// Release vertex buffer
	if (vb)
	{
		vb->Release();
		vb = nullptr;
	}
}

void LineClass::RenderBuffers(ID3D11DeviceContext* context)
{
	// Set stride and offset of vertex buffer
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	// Activate vertex buffer to render from IA
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	// Activate index buffer to render from IA
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}