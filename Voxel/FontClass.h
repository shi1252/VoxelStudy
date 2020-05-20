#pragma once
#include "Defined.h"

class TextureClass;

class FontClass
{
private:
	struct FontType
	{
		float left, right;
		int size;
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

public:
	FontClass() = default;
	FontClass(const FontClass&) = delete;
	~FontClass() = default;

	bool Initialize(ID3D11Device* device, LPCSTR fontFileName, LPCWSTR textureFileName);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	void BuildVertexArray(void* vertices, LPCSTR sentence, float drawX, float drawY);

private:
	bool LoadFontData(LPCSTR fileName);
	void ReleaseFontData();
	bool LoadTexture(ID3D11Device* device, LPCWSTR fileName);
	void ReleaseTexture();

	FontType* font = nullptr;
	TextureClass* texture = nullptr;
};