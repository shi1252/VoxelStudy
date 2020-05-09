#include "D3DClass.h"
#include "Defined.h"

bool D3DClass::Initialize(int width, int height, bool vsync, HWND hwnd, float screenFar, float screenNear)
{
	vsync_enabled = vsync;

	// Create Graphic interface factory
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
		return false;

	// Create first graphic card interface adapter using factory
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(factory->EnumAdapters(0, &adapter)))
		return false;

	// Define first adapter about output(Monitor)
	IDXGIOutput* adapterOutput = nullptr;
	if (FAILED(adapter->EnumOutputs(0, &adapterOutput)))
		return false;

	// Get count of mode list form of DXGI_FORMAT_R8G8B8A8_UNORM about output
	unsigned int numModes = 0;
	if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED,
		&numModes, NULL)))
		return false;

	// Create every possible combination about monitor and graphic card
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
		return false;

	// Fill display mode list
	if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED,
		&numModes, displayModeList)))
		return false;

	// Find display mode that same as width/height
	// If it is same then set refresh rate
	UINT numerator = 0;
	UINT denominator = 0;
	for (UINT i = 0; i < numModes; ++i)
	{
		if (displayModeList[i].Width == (UINT)width)
		{
			if (displayModeList[i].Height == (UINT)height)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get video card struct
	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(adapter->GetDesc(&adapterDesc)))
		return false;

	// Set video card memory by MB
	videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Set video card name
	size_t stringLength = 0;
	if (wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128) != 0)
		return false;

	// Release display mode list
	delete[] displayModeList;
	displayModeList = nullptr;

	// Release adapter output
	adapterOutput->Release();
	adapterOutput = nullptr;

	// Release adapter
	adapter->Release();
	adapter = nullptr;

	// Release factory
	factory->Release();
	factory = nullptr;

	// Initialize swapchain descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set back buffer count to 1
	swapChainDesc.BufferCount = 1;

	// Set back buffer width, height
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;

	// Set back buffer format
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set back buffer refresh rate
	if (vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set back buffer usage
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set rendering window handle
	swapChainDesc.OutputWindow = hwnd;

	// Turn off ms
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set windowed mode
	swapChainDesc.Windowed = true;

	// Set scanline order and scaling to unspecified
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Set discard back buffer after present
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Set option flags to 0
	swapChainDesc.Flags = 0;

	// Set feature level to dx 11
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create swapchain, D3D device, D3D device context
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext)))
		return false;

	// Get back buffer ptr
	ID3D11Texture2D* backBufferPtr = nullptr;
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr)))
		return false;

	// Create rtv using back buffer ptr
	if (FAILED(device->CreateRenderTargetView(backBufferPtr, NULL, &rtv)))
		return false;

	// Release back buffer ptr
	backBufferPtr->Release();
	backBufferPtr = nullptr;

	// Initialize depth stencil descriptor
	D3D11_TEXTURE2D_DESC dsBufferDesc;
	ZeroMemory(&dsBufferDesc, sizeof(dsBufferDesc));

	// Write ds buffer desc
	dsBufferDesc.Width = width;
	dsBufferDesc.Height = height;
	dsBufferDesc.MipLevels = 1;
	dsBufferDesc.ArraySize = 1;
	dsBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsBufferDesc.SampleDesc.Count = 1;
	dsBufferDesc.SampleDesc.Quality = 0;
	dsBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	dsBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsBufferDesc.CPUAccessFlags = 0;
	dsBufferDesc.MiscFlags = 0;

	// Create ds buffer texture using desc
	if (FAILED(device->CreateTexture2D(&dsBufferDesc, NULL, &dsBuffer)))
		return false;

	// Initilize ds state desc
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// Write ds state desc
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xff;
	dsDesc.StencilWriteMask = 0xff;

	// Set option front face stencil
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Set option back face stencil
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create ds state
	if (FAILED(device->CreateDepthStencilState(&dsDesc, &dsState)))
		return false;

	// Set ds state
	deviceContext->OMSetDepthStencilState(dsState, 1);

	// Initilize ds view desc
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	// Write ds view desc
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	// Create ds view
	if (FAILED(device->CreateDepthStencilView(dsBuffer, &dsvDesc, &dsv)))
		return false;

	// Bind rtv and ds buffer to pipeline
	deviceContext->OMSetRenderTargets(1, &rtv, dsv);

	// Write raster desc
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create rasterizer state
	if (FAILED(device->CreateRasterizerState(&rasterDesc, &rasterState)))
		return false;

	// Set rasterizer state
	deviceContext->RSSetState(rasterState);

	// Set viewport option
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create viewport
	deviceContext->RSSetViewports(1, &viewport);

	// Set projection matrix
	float fov = 3.141592654f / 4.0f;
	float screenAspect = (float)width / (float)height;
	projMatrix = XMMatrixPerspectiveFovLH(fov, screenAspect, screenNear, screenFar);

	// Set world matrix to I
	worldMatrix = XMMatrixIdentity();

	// Set orthogonal projection matrix
	orthoMatrix = XMMatrixOrthographicLH((float)width, (float)height, screenNear, screenFar);

	return true;
}

void D3DClass::Shutdown()
{
	// Exception occur if it doesn't window mode
	if (swapChain)
		swapChain->SetFullscreenState(false, NULL);

	if (rasterState)
	{
		rasterState->Release();
		rasterState = nullptr;
	}

	if (dsv)
	{
		dsv->Release();
		dsv = nullptr;
	}

	if (dsState)
	{
		dsState->Release();
		dsState = nullptr;
	}

	if (dsBuffer)
	{
		dsBuffer->Release();
		dsBuffer = nullptr;
	}

	if (rtv)
	{
		rtv->Release();
		rtv = nullptr;
	}

	if (deviceContext)
	{
		deviceContext->Release();
		deviceContext = nullptr;
	}

	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (swapChain)
	{
		swapChain->Release();
		swapChain = nullptr;
	}
}

void D3DClass::BeginScene(float r, float g, float b, float a)
{
	// Set buffer clear color
	float color[4] = { r, g, b, a };

	// Clear back buffer
	deviceContext->ClearRenderTargetView(rtv, color);

	// Clear ds buffer
	deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DClass::EndScene()
{
	// Present back buffer
	if (vsync_enabled)
	{
		// Fix refresh rate
		swapChain->Present(1, 0);
	}
	else
	{
		// Present asap
		swapChain->Present(0, 0);
	}
}

ID3D11Device* D3DClass::GetDevice()
{
	return device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return deviceContext;
}

void D3DClass::GetProejctionMatrix(XMMATRIX& proj)
{
	proj = projMatrix;
}

void D3DClass::GetWorldMatrix(XMMATRIX& world)
{
	world = worldMatrix;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& ortho)
{
	ortho = orthoMatrix;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, videoCardDescription);
	memory = videoCardMemory;
}
