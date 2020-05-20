#pragma once
#include "Defined.h"
#include "TextureClass.h"

class TargaTextureClass : public TextureClass
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
	TargaTextureClass() = default;
	TargaTextureClass(const TargaTextureClass&) = delete;
	~TargaTextureClass() = default;

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void Shutdown() override;

	bool LoadTarga(LPCSTR fileName, int& height, int& width);

	unsigned char* targaData = nullptr;
	ID3D11Texture2D* texture = nullptr;
};