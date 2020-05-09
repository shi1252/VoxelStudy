#include <fstream>
#include "TextureClass.h"

bool TextureClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	int width = 0;
	int height = 0;

	// Load targa imaga to memory
	if (!LoadTarga(fileName, height, width))
		return false;

	// Set texture desc
	D3D11_TEXTURE2D_DESC td;
	td.Height = height;
	td.Width = width;
	td.MipLevels = 0;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	td.CPUAccessFlags = 0;
	td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create empty texture;
	if (FAILED(device->CreateTexture2D(&td, NULL, &texture)))
		return false;

	// Set width of targa image data
	UINT rowPitch = (width * 4) * sizeof(unsigned char);

	// Copy targa data to texture
	context->UpdateSubresource(texture, 0, NULL, targaData, rowPitch, 0);

	// Intialize shader resource view desc
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = td.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = -1;

	// Create shader resource view of texture
	if (FAILED(device->CreateShaderResourceView(texture, &srvd, &textureView)))
		return false;

	// Create mipmap of the texture
	context->GenerateMips(textureView);

	// Release targa image data cuz the data is loaded to texture
	delete[] targaData;
	targaData = nullptr;

	return true;
}

void TextureClass::Shutdown()
{
	if (textureView)
	{
		textureView->Release();
		textureView = nullptr;
	}

	if (texture)
	{
		texture->Release();
		texture = nullptr;
	}

	if (targaData)
	{
		delete[] targaData;
		targaData = nullptr;
	}
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return textureView;
}

bool TextureClass::LoadTarga(LPCSTR fileName, int& height, int& width)
{
	// Load targa file as binary
	FILE* file;
	if (fopen_s(&file, fileName, "rb") != 0)
		return false;

	// Read file header
	TargaHeader header;
	UINT count = (UINT)fread(&header, sizeof(TargaHeader), 1, file);
	if (count != 1)
		return false;

	// Get information from header
	height = (int)header.height;
	width = (int)header.width;
	int bpp = (int)header.bpp;

	// Check the file is 32 bit or 24 bit
	if (bpp != 32)
		return false;

	// Calculate 32 bit image data size
	int size = width * height * 4;

	// Allocate memory to load targa image data
	unsigned char* targaImage = new unsigned char[size];
	if (!targaImage)
		return false;

	// Read targa image data
	count = (unsigned int)fread(targaImage, 1, size, file);
	if (count != size)
		return false;

	// Close file
	if (fclose(file) != 0)
		return false;

	// Allocate memory targa data
	targaData = new unsigned char[size];
	if (!targaData)
		return false;

	// Initialize targa data index
	int index = 0;

	// Initialize targa image data index
	int k = (width * height * 4) - (width * 4);

	// Copy inverse targa image data to targa data cuz the targa image data saved backward
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			targaData[index + 0] = targaImage[k + 2]; // R
			targaData[index + 1] = targaImage[k + 1]; // G
			targaData[index + 2] = targaImage[k + 0]; // B
			targaData[index + 3] = targaImage[k + 3]; // A

			k += 4;
			index += 4;
		}

		k -= (width * 8);
	}

	delete[] targaImage;
	targaImage = 0;

	return true;
}
