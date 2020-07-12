#pragma once
#include "Defined.h"
#include <vector>

class TargaTextureClass;
class TextureClass;

class MeshClass
{
public:
	struct VertexType
	{
		XMFLOAT3 position = XMFLOAT3(0.f, 0.f, 0.f);
		XMFLOAT2 uv = XMFLOAT2(0.f, 0.f);
		XMFLOAT3 normal = XMFLOAT3(0.f, 0.f, 0.f);
		XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	};

public:
	MeshClass() = default;
	MeshClass(const MeshClass&) = delete;
	~MeshClass() { Shutdown(); };

	bool Initialize(ID3D11Device * device, ID3D11DeviceContext * context, LPCSTR fileName);
	bool InitializeBuffers(ID3D11Device * device);
	void Shutdown();
	void Render(ID3D11DeviceContext * context);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
	TextureClass* GetTextureClass() { return texture; }
	bool LoadTexture(ID3D11Device * device, ID3D11DeviceContext * context, LPCSTR fileName);

	void CreateSphere(float radius, UINT sliceCount, UINT stackCount);

	std::vector<VertexType> vertices;
	std::vector<int> indices;

private:
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext * context);

	void ReleaseTexture();

	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;
	int vCount = 0;
	int iCount = 0;
	TextureClass* texture = nullptr;
};