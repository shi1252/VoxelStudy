#include "MeshClass.h"
#include "TargaTextureClass.h"

bool MeshClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	// Initialize vertex index buffer
	if (!InitializeBuffers(device))
		return false;

	// Load texture
	return LoadTexture(device, context, fileName);
}

void MeshClass::Shutdown()
{
	ReleaseTexture();

	ShutdownBuffers();
}

void MeshClass::Render(ID3D11DeviceContext* context)
{
	// Put vb, ib to pipeline to draw
	RenderBuffers(context);
}

int MeshClass::GetIndexCount()
{
	return iCount;
}

ID3D11ShaderResourceView* MeshClass::GetTexture()
{
	return texture->GetTexture();
}

bool MeshClass::InitializeBuffers(ID3D11Device* device)
{
	if (vertices.empty() || indices.empty())
		return true;

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

void MeshClass::ShutdownBuffers()
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

void MeshClass::RenderBuffers(ID3D11DeviceContext* context)
{
	// Set stride and offset of vertex buffer
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	// Activate vertex buffer to render from IA
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	// Activate index buffer to render from IA
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool MeshClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	// Create texture object
	texture = new TargaTextureClass;
	if (!texture)
		return false;

	// Initialize texture object
	return texture->Initialize(device, context, fileName);
}

void MeshClass::CreateSphere(float radius, UINT sliceCount, UINT stackCount)
{
	vertices.clear();
	indices.clear();

	VertexType topVertex = { XMFLOAT3(0.f, radius, 0.f), XMFLOAT2(0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f) };
	VertexType bottomVertex = { XMFLOAT3(0.f, -radius, 0.f), XMFLOAT2(0.f, 1.f), XMFLOAT3(0.f, -1.f, 0.f) };

	vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;
	float thetaStep = XM_2PI / sliceCount;

	for (int i = 1; i < stackCount; ++i)
	{
		float phi = i * phiStep;

		for (int j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			VertexType v;

			v.position.x = radius * sinf(phi) * cosf(theta);
			v.position.y = radius * cosf(phi);
			v.position.z = radius * sinf(phi) * sinf(theta);

			XMVECTOR p = XMLoadFloat3(&v.position);
			XMStoreFloat3(&v.normal, XMVector3Normalize(p));

			v.uv.x = theta / XM_2PI;
			v.uv.y = phi / XM_PI;

			vertices.push_back(v);
		}
	}

	vertices.push_back(bottomVertex);

	for (int i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; ++i)
	{
		for (int j = 0; j < sliceCount; ++j)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	int southPoleIndex = vertices.size() - 1;

	baseIndex = southPoleIndex - ringVertexCount;

	for (int i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}
}

void MeshClass::ReleaseTexture()
{
	// Release texture object
	if (texture)
	{
		texture->Shutdown();
		delete texture;
		texture = nullptr;
	}
}
