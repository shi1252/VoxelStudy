#pragma once
#include "Defined.h"
#include "AlignedAllocationPolicy.h"

class TargaTextureClass;

class ModelClass : public AlignedAllocationPolicy<16>
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	ModelClass() = default;
	ModelClass(const ModelClass&) = delete;
	~ModelClass() = default;

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void Shutdown();
	void Render(ID3D11DeviceContext* context);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* context);

	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void ReleaseTexture();

	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;
	int vCount = 0;
	int iCount = 0;
	TargaTextureClass* texture = nullptr;
};