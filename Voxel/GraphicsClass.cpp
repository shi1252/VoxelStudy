#include "GraphicsClass.h"
#include "D3DClass.h"
#include "CameraClass.h"
#include "LightClass.h"
#include "ShaderParameter.h"
#include "ColorShaderClass.h"
#include "TextureShaderClass.h"
#include "VoxelShaderClass.h"
#include "ModelClass.h"
#include "Voxel.h"
#include "TextClass.h"
#include "LineClass.h"
#include "MathHelper.h"

using namespace MathHelper;

bool GraphicsClass::Initialize(int width, int height, HWND hWnd)
{
	this->width = width;
	this->height = height;

	// Create D3D
	d3d = (D3DClass*)_aligned_malloc(sizeof(D3DClass), 16);
	if (!d3d)
		return false;

	// Initialize D3D
	if (!d3d->Initialize(width, height, VSYNC_ENABLED, hWnd, SCREEN_FAR, SCREEN_NEAR))
	{
		MessageBox(hWnd, L"Direct3D initialize error.", L"Error", MB_OK);
		return false;
	}

	light = new LightClass;
	if (!light)
		return false;
	light->SetAmbientColor(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f));
	light->SetDiffuseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// Create camera
	camera = new CameraClass;
	if (!camera)
		return false;

	// Set camera position
	camera->SetPosition(0.f, 0.f, -5.0f);
	//camera->SetPosition(0.f, 0.f, -30.f);

	// Create text
	text = new TextClass;
	if (!text)
		return false;

	XMMATRIX baseViewMatrix;
	camera->Render();
	camera->GetViewMatrix(baseViewMatrix);

	// Initialize text object
	if (!text->Initialize(d3d->GetDevice(), d3d->GetDeviceContext(), hWnd, width, height, baseViewMatrix))
	{
		MessageBox(hWnd, L"Could not initialize text object.", L"Error", MB_OK);
		return false;
	}

	// Create Voxel
	voxel = new Voxel(XMFLOAT3(-50, -80, 0), 64, 1.f);
	if (!voxel)
		return false;

	if (!voxel->Initialize(d3d->GetDevice(), d3d->GetDeviceContext(), "Texture/stone01.tga"))
		return false;

	//// Create model
	//model = new ModelClass;
	//if (!model)
	//	return false;

	//// Initialize model
	//if (!model->Initialize(d3d->GetDevice(), d3d->GetDeviceContext(), "Texture/stone01.tga"))
	//{
	//	MessageBox(hWnd, L"Could not initialize the model obeject.", L"Error", MB_OK);
	//	return false;
	//}

	line = new LineClass();

	// Create shader
	shader = new VoxelShaderClass;
	if (!shader)
		return false;

	// Initialize shader
	if (!shader->Initialize(d3d->GetDevice(), hWnd))
	{
		MessageBox(hWnd, L"Could not initialize the shader obeject.", L"Error", MB_OK);
		return false;
	}

	colorShader = new ColorShaderClass;
	if (!colorShader)
		return false;

	if (!colorShader->Initialize(d3d->GetDevice(), hWnd))
	{
		MessageBox(hWnd, L"Could not initialize the shader obeject.", L"Error", MB_OK);
		return false;
	}

	return true;
}

void GraphicsClass::Shutdown()
{
	if (shader)
	{
		shader->Shutdown();
		delete shader;
		shader = nullptr;
	}

	if (colorShader)
	{
		colorShader->Shutdown();
		delete colorShader;
		colorShader = nullptr;
	}

	if (voxel)
	{
		voxel->Shutdown();
		delete voxel;
		voxel = nullptr;
	}

	//if (model)
	//{
	//	model->Shutdown();
	//	delete model;
	//	model = nullptr;
	//}

	if (line)
	{
		line->Shutdown();
		delete line;
		line = nullptr;
	}

	if (text)
	{
		text->Shutdown();
		delete text;
		text = nullptr;
	}

	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	if (light)
	{
		delete light;
		light = nullptr;
	}

	if (d3d)
	{
		d3d->Shutdown();
		_aligned_free(d3d);
		d3d = nullptr;
	}
}

XMFLOAT3 prevPos = XMFLOAT3(0.f, 0.f, 0.f);

bool GraphicsClass::Frame(int x, int y)
{
	if (!text->SetMousePosition(x, y, d3d->GetDeviceContext()))
		return false;

	XMFLOAT3 currPos = camera->GetPosition();
	//if (prevPos != currPos)
	{
		voxel->Update();
	}
	prevPos = currPos;

	return true;
}

//bool GraphicsClass::Frame()
//{
//	return Render();
//}

D3DClass* GraphicsClass::GetD3DClass()
{
	return d3d;
}

CameraClass* GraphicsClass::GetCamera()
{
	return camera;
}

Voxel* GraphicsClass::GetVoxel()
{
	return voxel;
}

XMFLOAT3 GraphicsClass::GetScreenToWorldPoint(int& x, int& y, float z)
{
	XMFLOAT3 pos;

	XMMATRIX view, proj, inv;
	camera->GetViewMatrix(view);
	d3d->GetProejctionMatrix(proj);

	inv = XMMatrixInverse(nullptr, view * proj);

	float hx = (2.f * x) / width - 1.f;
	float hy = ((2.f * y) / height - 1.f) * -1.f;

	//XMStoreFloat3(&pos, XMVector3Unproject(XMVectorSet(hx, hy, z, 0.f), 0, 0, width, height, 0.f, 1.f, proj, view, XMMatrixIdentity()));

	XMVECTOR screenPos = XMVectorSet(hx, hy, z, z);

	XMVECTOR vec = XMVector4Transform(screenPos, inv);
	XMVECTOR div = XMVectorSplatW(vec);
	
	XMStoreFloat3(&pos, XMVectorDivide(vec, div));

	return pos;
}

bool GraphicsClass::Render()
{
	// Buffer clear
	d3d->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	// Create view matrix from camera transform
	camera->Render();

	// Get world, view, proj matrices
	XMMATRIX world, view, proj, ortho;
	d3d->GetWorldMatrix(world);
	camera->GetViewMatrix(view);
	d3d->GetProejctionMatrix(proj);
	d3d->GetOrthoMatrix(ortho);

#pragma region Set Parameters
	ShaderParameter params;
	params.SetParam("world", world);
	params.SetParam("view", view);
	params.SetParam("proj", proj);
	params.SetParam("ortho", ortho);

	XMFLOAT3 camPos3 = camera->GetPosition();
	XMFLOAT4 camPos4 = XMFLOAT4(camPos3.x, camPos3.y, camPos3.z, 1.f);
	params.SetParam("camPos", camPos4);

	params.SetParam("light", light);
#pragma endregion


	// Render model by using shader
	//if (!shader->Render(d3d->GetDeviceContext(), model->GetIndexCount(), world, view, proj, model->GetTexture()))
	for (int i = 0; i < 64; ++i)
	{
		// Bind vertex buffer, index buffer to pipeline to draw
		//model->Render(d3d->GetDeviceContext());
		voxel->Render(d3d->GetDeviceContext(), i);

		if (!shader->Render(d3d->GetDeviceContext(), voxel->subChunks[i].mesh->GetIndexCount(), params, voxel->subChunks[i].mesh->GetTexture()))
			return false;
	}

	line->Initialize(d3d->GetDevice());
	line->Render(d3d->GetDeviceContext());
	if (!colorShader->Render(d3d->GetDeviceContext(), line->GetIndexCount(), world, view, proj))
		return false;

	// Turn off Z buffer to draw 2D
	d3d->SetZBuffer(false);

	// Turn on alpha blending before draw text
	d3d->SetAlphaBlending(true);

	// Draw text
	if (!text->Render(d3d->GetDeviceContext(), world, ortho))
		return false;

	// Turn off alpha blending after draw text
	d3d->SetAlphaBlending(false);

	// Turn on Z buffer after draw whole 2D object
	d3d->SetZBuffer(true);

	// Present
	d3d->EndScene();

	return true;
}
