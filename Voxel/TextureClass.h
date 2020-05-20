#pragma once
#include "Defined.h"

class TextureClass
{
public:
	TextureClass() = default;
	TextureClass(const TextureClass&) = delete;
	~TextureClass() = default;

	bool Initialize(ID3D11Device* device, LPCWSTR fileName);
	virtual void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

protected:
	ID3D11ShaderResourceView* textureView = nullptr;
};