#include "TextClass.h"
#include "FontClass.h"
#include "FontShaderClass.h"

bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, int width, int height, XMMATRIX baseViewMat)
{
	this->width = width;
	this->height = height;

	baseViewMatrix = baseViewMat;

	font = new FontClass;
	if (!font)
		return false;

	if (!(font->Initialize(device, "Data/fontdata.txt", L"Texture/font.dds")))
	{
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}

	shader = new FontShaderClass;
	if (!shader)
		return false;

	if (!shader->Initialize(device, hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	if (!InitializeSentence(&sentence1, 16, device))
		return false;

	if (!UpdateSentence(sentence1, "Hello", 100, 100, 1.f, 1.f, 1.f, context))
		return false;

	if (!InitializeSentence(&sentence2, 16, device))
		return false;

	if (!UpdateSentence(sentence2, "Goodbye", 100, 200, 1.f, 1.f, 0.f, context))
		return false;

	return true;
}

void TextClass::Shutdown()
{
	ReleaseSentence(&sentence1);
	ReleaseSentence(&sentence2);

	if (shader)
	{
		shader->Shutdown();
		delete shader;
		shader = nullptr;
	}

	if (font)
	{
		font->Shutdown();
		delete font;
		font = nullptr;
	}
}

bool TextClass::Render(ID3D11DeviceContext* context, XMMATRIX worldMat, XMMATRIX orthoMat)
{
	if (!RenderSentence(context, sentence1, worldMat, orthoMat))
		return false;

	if (!RenderSentence(context, sentence2, worldMat, orthoMat))
		return false;

	return true;
}

bool TextClass::SetMousePosition(int x, int y, ID3D11DeviceContext* context)
{
	char temp[16] = { 0, };
	_itoa_s(x, temp, 10);

	char mouseString[16] = { 0, };
	strcpy_s(mouseString, "Mouse X: ");
	strcat_s(mouseString, temp);

	if (!UpdateSentence(sentence1, mouseString, 20, 20, 1.f, 1.f, 1.f, context))
		return false;

	_itoa_s(y, temp, 10);

	strcpy_s(mouseString, "Mouse Y: ");
	strcat_s(mouseString, temp);

	if (!UpdateSentence(sentence2, mouseString, 20, 40, 1.f, 1.f, 1.f, context))
		return false;

	return true;
}

bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vbd, ibd;
	D3D11_SUBRESOURCE_DATA vd, id;

	*sentence = new SentenceType;
	if (!*sentence)
		return false;

	(*sentence)->vb = nullptr;
	(*sentence)->ib = nullptr;

	(*sentence)->maxLength = maxLength;

	(*sentence)->vertexCount = 6 * maxLength;
	(*sentence)->indexCount = (*sentence)->vertexCount;

	vertices = new VertexType[(*sentence)->vertexCount];
	if (!vertices)
		return false;

	indices = new unsigned long[(*sentence)->indexCount];
	if (!indices)
		return false;

	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	for (int i = 0; i < (*sentence)->indexCount; ++i)
		indices[i] = i;

	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	vd.pSysMem = vertices;
	vd.SysMemPitch = 0;
	vd.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&vbd, &vd, &(*sentence)->vb)))
		return false;

	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	id.pSysMem = indices;
	id.SysMemPitch = 0;
	id.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&ibd, &id, &(*sentence)->ib)))
		return false;

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

bool TextClass::UpdateSentence(SentenceType* sentence, LPCSTR text, int posX, int posY, float r, float g, float b, ID3D11DeviceContext* context)
{
	int length;
	VertexType* vertices;
	float drawX, drawY;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	sentence->r = r;
	sentence->g = g;
	sentence->b = b;

	length = (int)strlen(text);

	if (length > sentence->maxLength)
		return false;

	vertices = new VertexType[sentence->vertexCount];
	if (!vertices)
		return false;

	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	drawX = (float)(((width / 2) * -1) + posX);
	drawY = (float)((height / 2) - posY);

	font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	if (FAILED(context->Map(sentence->vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	verticesPtr = (VertexType*)mappedResource.pData;

	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	context->Unmap(sentence->vb, 0);

	delete[] vertices;
	vertices = nullptr;

	return true;
}

void TextClass::ReleaseSentence(SentenceType** sentence)
{
	if (*sentence)
	{
		if ((*sentence)->vb)
		{
			(*sentence)->vb->Release();
			(*sentence)->vb = nullptr;
		}

		if ((*sentence)->ib)
		{
			(*sentence)->ib->Release();
			(*sentence)->ib = nullptr;
		}

		delete* sentence;
		*sentence = nullptr;
	}
}

bool TextClass::RenderSentence(ID3D11DeviceContext* context, SentenceType* sentence, XMMATRIX worldMat, XMMATRIX orthoMat)
{
	unsigned int stride, offset;
	XMFLOAT4 pixelColor;
	
	stride = sizeof(VertexType);
	offset = 0;

	context->IASetVertexBuffers(0, 1, &sentence->vb, &stride, &offset);
	context->IASetIndexBuffer(sentence->ib, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pixelColor = XMFLOAT4(sentence->r, sentence->g, sentence->b, 1.f);
	
	if (!shader->Render(context, sentence->indexCount, worldMat, baseViewMatrix, orthoMat, font->GetTexture(), pixelColor))
		return false;

	return true;
}
