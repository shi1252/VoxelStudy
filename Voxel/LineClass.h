#pragma once
#include "Defined.h"
#include <vector>

class LineClass
{
public:
	struct VertexType
	{
		XMFLOAT3 position = XMFLOAT3(0.f, 0.f, 0.f);
		XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	};

	LineClass() = default;
	LineClass(const LineClass&) = delete;
	~LineClass() { Shutdown(); };

	bool Initialize(ID3D11Device * device);
	bool InitializeBuffers(ID3D11Device * device);
	void Shutdown();
	void Render(ID3D11DeviceContext * context);

	int GetIndexCount();

	std::vector<VertexType> vertices;
	std::vector<int> indices;

private:
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext * context);

	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;
	int vCount = 0;
	int iCount = 0;
};

