#include "FontClass.h"
#include "TextureClass.h"

#include <fstream>
using namespace std;

bool FontClass::Initialize(ID3D11Device* device, LPCSTR fontFileName, LPCWSTR textureFileName)
{
	if (!LoadFontData(fontFileName))
		return false;

	return LoadTexture(device, textureFileName);
}

void FontClass::Shutdown()
{
	ReleaseTexture();
	ReleaseFontData();
}

ID3D11ShaderResourceView* FontClass::GetTexture()
{
	return texture->GetTexture();
}

void FontClass::BuildVertexArray(void* vertices, LPCSTR sentence, float drawX, float drawY)
{
	VertexType* vp;
	int length;

	vp = (VertexType*)vertices;

	length = (int)strlen(sentence);

	int index = 0, letter;

	// Draw each letter
	for (int i = 0; i < length; ++i)
	{
		letter = ((int)sentence[i]) - 32;

		// If the letter is a space then move three pixels
		if (letter == 0)
			drawX = drawX + 3.f;
		else
		{
			// First triangle in quad
			vp[index].position = XMFLOAT3(drawX, drawY, 0.f);// Top left
			vp[index].uv = XMFLOAT2(font[letter].left, 0.f);
			++index;

			vp[index].position = XMFLOAT3((drawX + font[letter].size), (drawY - 16.f), 0.f);// Bottom right
			vp[index].uv = XMFLOAT2(font[letter].right, 1.f);
			++index;

			vp[index].position = XMFLOAT3(drawX, (drawY - 16.f), 0.f);// Bottom left
			vp[index].uv = XMFLOAT2(font[letter].left, 1.f);
			++index;

			// Second triangle in quad
			vp[index].position = XMFLOAT3(drawX, drawY, 0.f);// Top left
			vp[index].uv = XMFLOAT2(font[letter].left, 0.f);
			++index;

			vp[index].position = XMFLOAT3(drawX + font[letter].size, drawY, 0.f);// Top right
			vp[index].uv = XMFLOAT2(font[letter].right, 0.f);
			++index;

			vp[index].position = XMFLOAT3(drawX + font[letter].size, (drawY - 16.f), 0.f);// Bottom right
			vp[index].uv = XMFLOAT2(font[letter].right, 1.f);
			++index;

			// Update x position to draw by size of the letter and one pixel
			drawX = drawX + font[letter].size + 1.f;
		}
	}
}

bool FontClass::LoadFontData(LPCSTR fileName)
{
	ifstream fin;
	
	// Create font spacing buffer
	font = new FontType[95];
	if (!font)
		return false;

	// Read
	fin.open(fileName);
	if (fin.fail())
		return false;

	char temp;
	// Read ascii characters
	for (int i = 0; i < 95; ++i)
	{
		fin.get(temp);
		while (temp != ' ')
			fin.get(temp);
		fin.get(temp);
		while (temp != ' ')
			fin.get(temp);

		fin >> font[i].left;
		fin >> font[i].right;
		fin >> font[i].size;
	}

	fin.close();

	return true;
}

void FontClass::ReleaseFontData()
{
	if (font)
	{
		delete[] font;
		font = nullptr;
	}
}

bool FontClass::LoadTexture(ID3D11Device* device, LPCWSTR fileName)
{
	texture = new TextureClass;
	if (!texture)
		return false;

	return texture->Initialize(device, fileName);
}

void FontClass::ReleaseTexture()
{
	if (texture)
	{
		texture->Shutdown();
		delete texture;
		texture = nullptr;
	}
}
