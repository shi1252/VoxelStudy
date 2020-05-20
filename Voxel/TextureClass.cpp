#include "TextureClass.h"

bool TextureClass::Initialize(ID3D11Device* device, LPCWSTR fileName)
{
	if (FAILED(CreateDDSTextureFromFile(device, fileName, nullptr, &textureView)))
		return false;

	return true;
}

void TextureClass::Shutdown()
{
	if (textureView)
	{
		textureView->Release();
		textureView = nullptr;
	}
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return textureView;
}