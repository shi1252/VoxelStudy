#pragma once
#include "Defined.h"

class FontClass;
class FontShaderClass;

class TextClass : public AlignedAllocationPolicy<16>
{
private:
	struct SentenceType
	{
		ID3D11Buffer* vb, *ib;
		int vertexCount, indexCount, maxLength;
		float r, g, b;
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

public:
	TextClass() = default;
	TextClass(const TextClass&) = delete;
	~TextClass() = default;

	bool Initialize(ID3D11Device * device, ID3D11DeviceContext *context, HWND hwnd, int width, int height, XMMATRIX baseViewMat);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, XMMATRIX worldMat, XMMATRIX orthoMat);

	bool SetMousePosition(int x, int y, ID3D11DeviceContext* context);

private:
	bool InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device);
	bool UpdateSentence(SentenceType* sentence, LPCSTR text, int posX, int posY, float r, float g, float b, ID3D11DeviceContext* context);
	void ReleaseSentence(SentenceType** sentence);
	bool RenderSentence(ID3D11DeviceContext* context, SentenceType* sentence, XMMATRIX worldMat, XMMATRIX orthoMat);

	FontClass* font = nullptr;
	FontShaderClass* shader = nullptr;
	int width = 0;
	int height = 0;
	XMMATRIX baseViewMatrix;
	SentenceType* sentence1;
	SentenceType* sentence2;
};