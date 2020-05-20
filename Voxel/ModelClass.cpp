#include "ModelClass.h"
#include "TargaTextureClass.h"

/* Create Sphere Dx12
	MeshData meshData;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back( topVertex );

	float phiStep   = XM_PI/stackCount;
	float thetaStep = 2.0f*XM_PI/sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for(uint32 i = 1; i <= stackCount-1; ++i)
	{
		float phi = i*phiStep;

		// Vertices of ring.
		for(uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j*thetaStep;

			Vertex v;

			// spherical to cartesian
			v.Position.x = radius*sinf(phi)*cosf(theta);
			v.Position.y = radius*cosf(phi);
			v.Position.z = radius*sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius*sinf(phi)*sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius*sinf(phi)*cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

			v.TexC.x = theta / XM_2PI;
			v.TexC.y = phi / XM_PI;

			meshData.Vertices.push_back( v );
		}
	}

	meshData.Vertices.push_back( bottomVertex );

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for(uint32 i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices32.push_back(0);
		meshData.Indices32.push_back(i+1);
		meshData.Indices32.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32 baseIndex = 1;
	uint32 ringVertexCount = sliceCount + 1;
	for(uint32 i = 0; i < stackCount-2; ++i)
	{
		for(uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(baseIndex + i*ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i*ringVertexCount + j+1);
			meshData.Indices32.push_back(baseIndex + (i+1)*ringVertexCount + j);

			meshData.Indices32.push_back(baseIndex + (i+1)*ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i*ringVertexCount + j+1);
			meshData.Indices32.push_back(baseIndex + (i+1)*ringVertexCount + j+1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32 southPoleIndex = (uint32)meshData.Vertices.size()-1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for(uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(southPoleIndex);
		meshData.Indices32.push_back(baseIndex+i);
		meshData.Indices32.push_back(baseIndex+i+1);
	}

	return meshData;
*/

bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	// Initialize vertex index buffer
	if (!InitializeBuffers(device))
		return false;

	// Load texture
	return LoadTexture(device, context, fileName);
}

void ModelClass::Shutdown()
{
	ReleaseTexture();

	ShutdownBuffers();
}

void ModelClass::Render(ID3D11DeviceContext* context)
{
	// Put vb, ib to pipeline to draw
	RenderBuffers(context);
}

int ModelClass::GetIndexCount()
{
	return iCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	// Set vertex count
	vCount = 3;

	// Set index count
	iCount = 3;

	// Create vertex array
	VertexType* vertices = new VertexType[vCount];
	if (!vertices)
		return false;

	// Create index array
	ULONG* indices = new ULONG[iCount];
	if (!indices)
		return false;

	// Set vertex data
	vertices[0].position = XMFLOAT3(-1.f, -1.f, 0.f);
	vertices[0].texture = XMFLOAT2(0.f, 1.f);

	vertices[1].position = XMFLOAT3(0.f, 1.f, 0.f);
	vertices[1].texture = XMFLOAT2(0.5f, 0.f);
	
	vertices[2].position = XMFLOAT3(1.f, -1.f, 0.f);
	vertices[2].texture = XMFLOAT2(1.f, 1.f);

	// Set index data
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	// Initialize vertex buffer desc
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexType) * vCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Set ptr to subresource about vertext data
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create vertex buffer
	if (FAILED(device->CreateBuffer(&vbd, &vertexData, &vb)))
		return false;

	// Initialize index buffer desc
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(ULONG) * iCount;
	ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Set ptr to subresource about index data
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create index buffer
	if (FAILED(device->CreateBuffer(&ibd, &indexData, &ib)))
		return false;

	// Release vertices, indices
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void ModelClass::ShutdownBuffers()
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

void ModelClass::RenderBuffers(ID3D11DeviceContext* context)
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

bool ModelClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	// Create texture object
	texture = new TargaTextureClass;
	if (!texture)
		return false;

	// Initialize texture object
	return texture->Initialize(device, context, fileName);
}

void ModelClass::ReleaseTexture()
{
	// Release texture object
	if (texture)
	{
		texture->Shutdown();
		delete texture;
		texture = nullptr;
	}
}
