#pragma once
#include "Defined.h"

class TextureClass
{
private:
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

public:
	TextureClass() = default;
	TextureClass(const TextureClass&) = delete;
	~TextureClass() = default;

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	bool LoadTarga(LPCSTR fileName, int& height, int& width);

	unsigned char* targaData = nullptr;
	ID3D11Texture2D* texture = nullptr;
	ID3D11ShaderResourceView* textureView = nullptr;
};