#include "Graphics.h"
#include "OBJECT_VS.csh"
#include "OBJECT_PS.csh"
#include "OBJECT_VS_TARGET.csh"
#include "OBJECT_PS_TARGET.csh"
#include "ANIMATION_VS.csh"
#include "POST_PROCESSING_VS.csh"
#include "POST_PROCESSING_PS.csh"
#include "PARTICLE_VS.csh"
#include "PARTICLE_PS.csh"
#include "UI_VS.csh"
#include "UI_PS.csh"
#include "INSTANCING_VS.csh"
#include "SpriteBatch.h"
#include "SpriteFont.h"

#define COLOR_TEST 0

// Statics 
HWND						             Graphics::m_window;
HINSTANCE				                 Graphics::m_application;
WNDPROC						             Graphics::m_appWndProc;
								         
std::unique_ptr<Keyboard>                Graphics::single_keyboard = NULL;
std::unique_ptr<Mouse>                   Graphics::single_mouse = NULL;
CComPtr<IDXGISwapChain>		             Graphics::m_swapChain;
CComPtr<ID3D11Device>		             Graphics::m_device;
CComPtr<ID3D11DeviceContext>             Graphics::m_deviceContext;
CComPtr<ID3D11RasterizerState>           Graphics::m_rasterState;
CComPtr<ID3D11RasterizerState>           Graphics::m_wireRasterState;
CComPtr<ID3D11Texture2D>	             Graphics::m_depthBuffer;
CComPtr<ID3D11DepthStencilView>          Graphics::m_depthView;
CComPtr<ID3D11DepthStencilState>         Graphics::m_depthState;
CComPtr<ID3D11DepthStencilState>         Graphics::m_depthStateFront;
CComPtr<ID3D11RenderTargetView>          Graphics::m_renderTargetView;
D3D11_VIEWPORT				             Graphics::m_viewPort;
CComPtr<ID3D11SamplerState>              Graphics::m_samplerState;
CComPtr<ID3D11BlendState>                Graphics::m_blendTransperentState;
CComPtr<ID3D11Buffer>                    Graphics::m_structBuffer;
CComPtr<ID3D11ShaderResourceView>        Graphics::m_resourceViewStruct;
COLOR                                    Graphics::m_backColor;
bool                                     Graphics::m_resized;
bool                                     Graphics::m_fullscreen;
bool                                     Graphics::m_debugWire;
unsigned int                             Graphics::m_backBufferWidth;
unsigned int                             Graphics::m_backBufferHeight;
std::vector<CComPtr<ID3D11VertexShader>> Graphics::m_vsShaders;
std::vector<CComPtr<ID3D11PixelShader>>  Graphics::m_psShaders;
std::vector<CComPtr<ID3D11InputLayout>>  Graphics::m_layouts;
CComPtr<ID3D11RenderTargetView>          Graphics::m_postRenderTargetView;
CComPtr<ID3D11Texture2D>	             Graphics::m_postTexture;
CComPtr<ID3D11ShaderResourceView>        Graphics::m_postShaderResourceView;
CComPtr<ID3D11DepthStencilView>          Graphics::m_postDepthView;
CComPtr<ID3D11Buffer>                    Graphics::m_postDataConstBuffer;
CComPtr<ID3D11Buffer>                    Graphics::m_quadPostBuffer;
CComPtr<ID3D11Buffer>                    Graphics::m_quadPostIndexBuffer;
CComPtr<ID3D11Texture2D>	             Graphics::m_postDepthBuffer;
CComPtr<ID3D11ShaderResourceView>        Graphics::m_postZShaderToResourceView;
POST_DATA_TO_VRAM                        Graphics::m_postData;
XTime                                    Graphics::m_timer;
BUFFER_STRUCT                           *Graphics::m_bufferStruct;

// Globals
std::unique_ptr<SpriteBatch>             g_Sprites;
std::unique_ptr<SpriteFont>              g_FontFrank;
std::unique_ptr<SpriteFont>              g_FontItalic;

unsigned int g_flags = NULL;
const int sizeGrid = 1000;

// Constructor
Graphics::Graphics()
{
}

// Initializing
Graphics::Graphics(HINSTANCE _hinst, WNDPROC _proc)
{
	
#ifdef DEBUG
	g_flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
	single_keyboard = make_unique<Keyboard>();
	single_mouse = make_unique<Mouse>();
	m_resized = false;
	m_backBufferHeight = WINDOW_HEIGHT;
	m_backBufferWidth = WINDOW_WIDTH;
	m_debugWire = false;

	m_backColor.SetColor(0.39f, 0.58f, 0.92f, 1.0f);

	// Creates the window
	CreateAppWindow(_hinst, _proc);

	// Creates the swapchain and back buffer
	Initialize();

	// Creates the view port
	CreateViewPorts();

	// Initilizing the depth stencil view
	CreateDepthBuffer();

	// Creating the shaders
	CreateShaders();

	// Creating the layouts for the shaders
	CreateLayouts();

	// Create a sampler state
	CreateSamplerState();

	// Creates a rasterizer state
	CreateRasterState();

	// Creates the post quad filter
	CreatePostFilterQuad();

	// Creates the post processing texture
	CreatePostProcessingTexture();

	CreateConstBuffers();

	CreateBlendState();

	m_postData.data[0] = 0.0f;
	m_postData.data[1] = -0.3f;

	g_Sprites.reset(new SpriteBatch(m_deviceContext.p));
	g_FontFrank.reset(new SpriteFont(m_device.p, L"..\\NDEProject\\Assets\\FrankKnows.spritefont"));
	g_FontItalic.reset(new SpriteFont(m_device.p, L"..\\NDEProject\\Assets\\italic.spritefont"));
	
#if COLOR_TEST
	m_bufferStruct = new BUFFER_STRUCT[sizeGrid];
	//CreateStructBuffer();
#endif
}

// Destructor
Graphics::~Graphics()
{
#if COLOR_TEST
	delete m_bufferStruct;
#endif
}

// Resize Window
void Graphics::ResizeWindow()
{
	if (m_swapChain)
	{
		HWND desktop = GetDesktopWindow();
		RECT rect;
		GetWindowRect(desktop, &rect);

		if (!m_fullscreen)
		{
			m_fullscreen = true;
			m_backBufferHeight = rect.bottom;
			m_backBufferWidth = rect.right;
		}
		else
		{ 
			m_fullscreen = false;
			m_backBufferHeight = WINDOW_HEIGHT;
			m_backBufferWidth = WINDOW_WIDTH;
		}

		// Resizing the depth buffer
		m_depthBuffer.Release();
		m_depthView.Release();
		m_renderTargetView.Release();
	
		// Set the buffer parameters
		m_swapChain->ResizeBuffers(1, m_backBufferWidth, m_backBufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		// Creation of the texture
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = m_backBufferWidth;
		textureDesc.Height = m_backBufferHeight;
		textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthBuffer.p);

		// Creation of the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthDesc.Texture2D.MipSlice = 0;
		m_device->CreateDepthStencilView(m_depthBuffer.p, nullptr, &m_depthView.p);

		// Resizing the viewport
		ZeroMemory(&m_viewPort, sizeof(m_viewPort));
		m_viewPort.Width = float(m_backBufferWidth);
		m_viewPort.Height = float(m_backBufferHeight);
		m_viewPort.MaxDepth = 1;

		// Setting the backbuffer
		ID3D11Texture2D *backBuffer;
		m_swapChain->GetBuffer(0, __uuidof(backBuffer), (LPVOID*)(&backBuffer));
		// Creating the render view
		m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView.p);
		backBuffer->Release();

		m_resized = true;
		ResizePPTexture(m_backBufferWidth, m_backBufferHeight);

		// Swapchain resize function
		if (m_fullscreen)
			m_swapChain->SetFullscreenState(true, nullptr);
		else
			m_swapChain->SetFullscreenState(false, nullptr);


	}
}

// TODO:: WORK ON RESIZING
void Graphics::ResizeTextures(unsigned int _width, unsigned int _height)
{
	// Releasing
	//m_renderToTexture.Release();
	//zRendertoTextureBuffer.Release();
	//m_renderTargetView.Release();
	//renderToShaderResourceView.Release();
	//m_depthBuffer.Release();

	
	//// Creation of the texture
	//D3D11_TEXTURE2D_DESC textureDesc;
	//ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	//textureDesc.Width = _width;
	//textureDesc.Height = _height;
	//textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	//textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//textureDesc.MipLevels = 1;
	//textureDesc.ArraySize = 1;
	//textureDesc.SampleDesc.Count = 1;
	//textureDesc.SampleDesc.Quality = 0;
	//m_device->CreateTexture2D(&textureDesc, NULL, &renderToTexture.p);
	
	// Render target 
	//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	//ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	//renderTargetViewDesc.Format = textureDesc.Format;
	//renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//m_device->CreateRenderTargetView(renderToTexture, NULL, &RenderTargetViewToTexture.p);
	
	//Device->CreateShaderResourceView(renderToTexture, NULL, &renderToShaderResourceView);

	//// Creation of the texture for depth buffer
	//ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	//textureDesc.Width = _width;
	//textureDesc.Height = _height;
	//textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//textureDesc.MipLevels = 1;
	//textureDesc.ArraySize = 1;
	//textureDesc.SampleDesc.Count = 1;
	//textureDesc.SampleDesc.Quality = 0;
	//textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//
	//// Creation of the depth stencil view
	//D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	//ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	//depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//depthDesc.Texture2D.MipSlice = 0;
	//
	//// Creation of the texture for render to texture
	//Device->CreateTexture2D(&textureDesc, nullptr, &zRendertoTextureBuffer.p);
	//// Creation of the depth stencil view
	//Device->CreateDepthStencilView(zRendertoTextureBuffer.p, &depthDesc, &depthRenderToTextureView.p);
}

void Graphics::ResizePPTexture(unsigned int _width, unsigned int _height)
{
	// Releasing
	m_postTexture.Release();
	m_postRenderTargetView.Release();
	m_postShaderResourceView.Release();
	m_postDepthView.Release();
	m_postDepthBuffer.Release();
	m_postZShaderToResourceView.Release();

	CreatePostProcessingTexture();

	// Creation of the texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = _width;
	textureDesc.Height = _height;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Creation of the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	
	// Post processing
	// Creation of the texture for render to texture
	m_device->CreateTexture2D(&textureDesc, nullptr, &m_postDepthBuffer.p);
	// Creation of the depth stencil view
	m_device->CreateDepthStencilView(m_postDepthBuffer.p, &depthDesc, &m_postDepthView.p);
	m_device->CreateShaderResourceView(m_postDepthBuffer, &srvDesc, &m_postZShaderToResourceView.p);

}

// Renders the scene
void Graphics::Render()
{   
	m_timer.Signal();

	// Blend state for transperancy
	float blendFactor[] = { 0, 0, 0, 0 };
	unsigned int sampleMask = 0xffffffff;
	m_deviceContext->OMSetBlendState(m_blendTransperentState, blendFactor, sampleMask);

	// Setting the render target with the depth buffer
	m_deviceContext->RSSetViewports(1, &m_viewPort);
	m_deviceContext->OMSetRenderTargets(1, &m_postRenderTargetView.p, m_postDepthView.p);
	//m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView.p, m_depthView.p);

	if (m_debugWire)
		m_deviceContext->RSSetState(m_wireRasterState.p);
	else
		m_deviceContext->RSSetState(m_rasterState.p);

	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState.p);

	// Clearing the screen
	ClearScreen(m_backColor);
	
#if COLOR_TEST
	// TESTING:: Set the deffered rendering shader resource 
	m_deviceContext->PSSetShaderResources(4, 1, &m_resourceViewStruct.p);
#endif
}

// Post render to the screen
void Graphics::PostRender()
{
	if (m_debugWire)
		m_deviceContext->RSSetState(m_rasterState.p);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView.p, m_depthView.p);

	// Render the Quad filter
	unsigned int stride = sizeof(SM_VERTEX);
	unsigned int offset = 0;
	// Set the buffers
	m_deviceContext->IASetVertexBuffers(0, 1, &m_quadPostBuffer.p, &stride, &offset);
	m_deviceContext->IASetIndexBuffer(m_quadPostIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	m_deviceContext->VSSetShader(m_vsShaders[POST_PROCESSING], NULL, NULL);
	m_deviceContext->PSSetShader(m_psShaders[POST_PROCESSING], NULL, NULL);
	m_deviceContext->IASetInputLayout(m_layouts[POST_PROCESSING]);

	// Setting constant buffers - first parameter is the regester (b0) in shader
	m_deviceContext->VSSetConstantBuffers(0, 1, &m_postDataConstBuffer.p);
	m_deviceContext->PSSetConstantBuffers(0, 1, &m_postDataConstBuffer.p);
	
	// Set the shader resource - for the texture
	m_deviceContext->PSSetShaderResources(0, 1, &m_postShaderResourceView.p);
	m_deviceContext->PSSetShaderResources(1, 1, &m_postZShaderToResourceView.p);
	
	// Post effect information
    m_postData.data[0] = float(m_timer.TotalTimeExact());

	// DEBUG::TEMPORARY
#if 0
	if (single_keyboard->GetState().Z)
		m_postData.data[1] += float(m_timer.Delta());
	else if(single_keyboard->GetState().X)
		m_postData.data[1] -= float(m_timer.Delta());
#endif
	D3D11_MAPPED_SUBRESOURCE mapSubresource;
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	m_deviceContext->Map(m_postDataConstBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_postData, sizeof(POST_DATA_TO_VRAM));
	m_deviceContext->Unmap(m_postDataConstBuffer, NULL);
	
	// Drawing the filter quad
	m_deviceContext->DrawIndexed(6, 0, 0);
	
	// Don't read from the shader anymore
	CComPtr<ID3D11ShaderResourceView> nullShaderResourceView;
	m_deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView.p);
	m_deviceContext->PSSetShaderResources(1, 1, &nullShaderResourceView.p);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView.p, m_depthView.p);
	// Cleaning the depth buffer so it gives the infinite look
	m_deviceContext->ClearDepthStencilView(m_depthView.p, D3D11_CLEAR_DEPTH, 1, 0);

}

// Terminates the application
bool Graphics::ShutDown()
{
	m_swapChain->SetFullscreenState(false, NULL);
	UnregisterClass(L"DirectXApplication", m_application);
	return true;
}

// Creates the window
void Graphics::CreateAppWindow(HINSTANCE _hinst, WNDPROC _proc)
{
	m_application = _hinst;
	m_appWndProc = _proc;
	// Setting up the window
	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc = m_appWndProc;
	wndClass.hInstance = m_application;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	//wndClass.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WINLOGO));
	wndClass.hIconSm = wndClass.hIcon;
	wndClass.lpszClassName = L"NDEProject";
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_3DFACE);
	RegisterClassEx(&wndClass);

	// Window size rectangle
	RECT window_size = { 0, 0,WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&window_size, WS_BORDER/*WS_OVERLAPPEDWINDOW*/, false);
	// Creating the window
	m_window = CreateWindow(L"NDEProject", L"Near Death Experience", WS_BORDER/*WS_OVERLAPPEDWINDOW*/, CW_USEDEFAULT, CW_USEDEFAULT,
		window_size.right - window_size.left, window_size.bottom - window_size.top, NULL, NULL, m_application, this);

	// Moving it to the center
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0) and the bottom right corner will have coordinates (horizontal, vertical)
	int hX = int(desktop.right * 0.5f) - int(WINDOW_WIDTH * 0.5f);
	int vY = int(desktop.bottom * 0.5f) - int(WINDOW_HEIGHT * 0.5f);
	SetWindowPos(m_window, 0, hX, vY, 0, 0, SWP_NOSIZE);

	//HCURSOR hCursor = LoadCursor(NULL, L"..\\NDEProject\\Assets\\Cursor_Industrialized.png");
	//SetCursor(hCursor);

	// Turning off windows window settings
	LONG lStyle = GetWindowLong(m_window, GWL_STYLE);
	lStyle &= ~(/*WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE |*/ WS_SYSMENU);
	SetWindowLong(m_window, GWL_STYLE, lStyle);


	// Showing the window
	ShowCursor(false);
	ShowWindow(m_window, SW_SHOW);
}

// Clears the screen
void Graphics::ClearScreen(COLOR _color)
{
	m_deviceContext->ClearRenderTargetView(m_postRenderTargetView.p, _color.GetColor());
	m_deviceContext->ClearDepthStencilView(m_postDepthView.p, D3D11_CLEAR_DEPTH, 1, 0);
}

void Graphics::ClearBaseScreen(COLOR _color)
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.p, _color.GetColor());
	m_deviceContext->ClearDepthStencilView(m_depthView.p, D3D11_CLEAR_DEPTH, 1, 0);
}


// Initializes graphics
void Graphics::Initialize()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	// Creating the swap chain and the device
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set the buffer parameters
	DXGI_MODE_DESC buffer;
	ZeroMemory(&buffer, sizeof(DXGI_MODE_DESC));
	buffer.Height = m_backBufferHeight;
	buffer.Width = m_backBufferWidth;
	buffer.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the swap chain desc parameters
	swapChainDesc.BufferDesc = buffer;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_window;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// For frame capping
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	// Create the device and swap chain
	HRESULT hresultHandle = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, g_flags,
		NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);

	// Setting the backbuffer
	CComPtr<ID3D11Texture2D> backBuffer;
	m_swapChain->GetBuffer(0, __uuidof(backBuffer), (LPVOID*)&backBuffer/* reinterpret_cast<void**>(&backBuffer)*/);

	// Creating the render view
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView.p);
}

// Creates depth buffer
void Graphics::CreateDepthBuffer()
{
	// Creation of the texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = m_backBufferWidth;
	textureDesc.Height = m_backBufferHeight;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Creation of the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthBuffer.p);
	m_device->CreateDepthStencilView(m_depthBuffer.p, &depthDesc, &m_depthView.p);

	D3D11_DEPTH_STENCIL_DESC dpthStencil;
	ZeroMemory(&dpthStencil, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dpthStencil.DepthEnable = TRUE;
	dpthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dpthStencil.DepthFunc = D3D11_COMPARISON_LESS;
	dpthStencil.StencilEnable = FALSE;
	dpthStencil.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dpthStencil.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dpthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dpthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dpthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	m_device->CreateDepthStencilState(&dpthStencil, &m_depthState.p);


	// Post processing
	// Creation of the texture for render to texture
	m_device->CreateTexture2D(&textureDesc, nullptr, &m_postDepthBuffer.p);
	// Creation of the depth stencil view
	m_device->CreateDepthStencilView(m_postDepthBuffer.p, &depthDesc, &m_postDepthView.p);
	m_device->CreateShaderResourceView(m_postDepthBuffer, &srvDesc, &m_postZShaderToResourceView.p);

	// Render outfront stencil buffer
	ZeroMemory(&dpthStencil, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dpthStencil.DepthEnable = TRUE;
	dpthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dpthStencil.DepthFunc = D3D11_COMPARISON_GREATER;
	dpthStencil.StencilEnable = FALSE;
	dpthStencil.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dpthStencil.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dpthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dpthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dpthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dpthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	m_device->CreateDepthStencilState(&dpthStencil, &m_depthStateFront.p);

}

// Create view ports
void Graphics::CreateViewPorts()
{
	// Creating the view port
	ZeroMemory(&m_viewPort, sizeof(m_viewPort));
	m_viewPort.Width = WINDOW_WIDTH;
	m_viewPort.Height = WINDOW_HEIGHT;
	m_viewPort.MaxDepth = 1;
}

// Creates the layouts
void Graphics::CreateLayouts()
{
	CComPtr<ID3D11InputLayout> inputLayoutObject;
	CComPtr<ID3D11InputLayout> inputLayoutAnimation;
	CComPtr<ID3D11InputLayout> inputLayoutPostProcessor;
	CComPtr<ID3D11InputLayout> inputLayoutParticles;
	CComPtr<ID3D11InputLayout> inputLayoutUI;
	CComPtr<ID3D11InputLayout> inputLayoutInstanced;

	D3D11_INPUT_ELEMENT_DESC vLayoutTrival[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC vLayoutParticle[] =
	{
		{ "SIDE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UP", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "FORWARD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC vLayoutObject[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENTS", 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_INDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC vLayoutAnimation[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENTS", 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_INDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Creating layouts // TODO:: TRIVAL VS
	m_device->CreateInputLayout(vLayoutObject, ARRAYSIZE(vLayoutObject), OBJECT_VS, sizeof(OBJECT_VS), &inputLayoutObject.p);
	m_device->CreateInputLayout(vLayoutAnimation, ARRAYSIZE(vLayoutAnimation), ANIMATION_VS, sizeof(ANIMATION_VS), &inputLayoutAnimation.p);
	m_device->CreateInputLayout(vLayoutTrival, ARRAYSIZE(vLayoutTrival), POST_PROCESSING_VS, sizeof(POST_PROCESSING_VS), &inputLayoutPostProcessor.p);
	m_device->CreateInputLayout(vLayoutParticle, ARRAYSIZE(vLayoutParticle), PARTICLE_VS, sizeof(PARTICLE_VS), &inputLayoutParticles.p);
	m_device->CreateInputLayout(vLayoutTrival, ARRAYSIZE(vLayoutTrival), UI_VS, sizeof(UI_VS), &inputLayoutUI.p);
	m_device->CreateInputLayout(vLayoutObject, ARRAYSIZE(vLayoutObject), INSTANCING_VS, sizeof(INSTANCING_VS), &inputLayoutInstanced.p);
	
	m_layouts.push_back(inputLayoutObject);
	m_layouts.push_back(inputLayoutAnimation);
	m_layouts.push_back(inputLayoutPostProcessor);
	m_layouts.push_back(inputLayoutParticles);
	m_layouts.push_back(inputLayoutUI);
	m_layouts.push_back(inputLayoutInstanced);
	m_layouts.push_back(inputLayoutObject);

}

// Create shaders
void Graphics::CreateShaders()
{
	CComPtr<ID3D11VertexShader>       VS_OBJECT;
	CComPtr<ID3D11VertexShader>       VS_ANIMATION;
	CComPtr<ID3D11PixelShader>        PS_OBJECT;
	CComPtr<ID3D11VertexShader>       VS_OBJECT_TARGET;
	CComPtr<ID3D11PixelShader>        PS_OBJECT_TARGET;
	CComPtr<ID3D11VertexShader>       VS_POST_PROCESSING;
	CComPtr<ID3D11PixelShader>        PS_POST_PROCESSING;
	CComPtr<ID3D11VertexShader>       VS_PARTICLES;
	CComPtr<ID3D11PixelShader>        PS_PARTICLES;
	CComPtr<ID3D11VertexShader>       VS_UI;
	CComPtr<ID3D11PixelShader>        PS_UI;
	CComPtr<ID3D11VertexShader>       VS_INSTANCING;


	// Creates the shaders for the Object
	m_device->CreateVertexShader(OBJECT_VS, sizeof(OBJECT_VS), nullptr, &VS_OBJECT.p);
	m_device->CreatePixelShader(OBJECT_PS, sizeof(OBJECT_PS), nullptr, &PS_OBJECT.p);

	m_device->CreateVertexShader(OBJECT_VS_TARGET, sizeof(OBJECT_VS_TARGET), nullptr, &VS_OBJECT_TARGET.p);
	m_device->CreatePixelShader(OBJECT_PS_TARGET, sizeof(OBJECT_PS_TARGET), nullptr, &PS_OBJECT_TARGET.p);

	m_device->CreateVertexShader(ANIMATION_VS, sizeof(ANIMATION_VS), nullptr, &VS_ANIMATION.p);
	
	m_device->CreateVertexShader(POST_PROCESSING_VS, sizeof(POST_PROCESSING_VS), nullptr, &VS_POST_PROCESSING.p);
	m_device->CreatePixelShader(POST_PROCESSING_PS, sizeof(POST_PROCESSING_PS), nullptr, &PS_POST_PROCESSING.p);

	m_device->CreateVertexShader(PARTICLE_VS, sizeof(PARTICLE_VS), nullptr, &VS_PARTICLES.p);
	m_device->CreatePixelShader(PARTICLE_PS, sizeof(PARTICLE_PS), nullptr, &PS_PARTICLES.p);

	m_device->CreateVertexShader(UI_VS, sizeof(UI_VS), nullptr, &VS_UI.p);
	m_device->CreatePixelShader(UI_PS, sizeof(UI_PS), nullptr, &PS_UI.p);

	m_device->CreateVertexShader(INSTANCING_VS, sizeof(INSTANCING_VS), nullptr, &VS_INSTANCING.p);
	// Normal Objects
	m_vsShaders.push_back(VS_OBJECT);
	m_psShaders.push_back(PS_OBJECT);
	// Animated Objects
	m_vsShaders.push_back(VS_ANIMATION);
	m_psShaders.push_back(PS_OBJECT);
	// Post processing
	m_vsShaders.push_back(VS_POST_PROCESSING);
	m_psShaders.push_back(PS_POST_PROCESSING);
	// Particles
	m_vsShaders.push_back(VS_PARTICLES);
	m_psShaders.push_back(PS_PARTICLES);
	// UI objects
	m_vsShaders.push_back(VS_UI);
	m_psShaders.push_back(PS_UI);
	// Instanced Objects
	m_vsShaders.push_back(VS_INSTANCING);
	m_psShaders.push_back(PS_OBJECT);
	// Target
	m_vsShaders.push_back(VS_OBJECT_TARGET);
	m_psShaders.push_back(PS_OBJECT_TARGET);
}

// Creates the sampler state that goes to the pixel shader texture
void Graphics::CreateSamplerState()
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	m_device->CreateSamplerState(&samplerDesc, &m_samplerState.p);
}

// Creates a backculling rasterizer
void Graphics::CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterDesc.AntialiasedLineEnable = false;	
	rasterDesc.CullMode = D3D11_CULL_FRONT;//D3D11_CULL_NONE;//
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	m_device->CreateRasterizerState(&rasterDesc, &m_rasterState.p);

	// Wire rasterizer 
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	m_device->CreateRasterizerState(&rasterDesc, &m_wireRasterState.p);
}

// Create Post processing texture
void Graphics::CreatePostProcessingTexture()
{
	// Creation of the texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = m_backBufferWidth;
	textureDesc.Height = m_backBufferHeight;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	m_device->CreateTexture2D(&textureDesc, NULL, &m_postTexture.p);

	// Render target 
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	m_device->CreateRenderTargetView(m_postTexture, &renderTargetViewDesc, &m_postRenderTargetView.p);

	m_device->CreateShaderResourceView(m_postTexture, NULL, &m_postShaderResourceView);
}

void Graphics::CreatePostFilterQuad()
{
	const unsigned int numVerts = 4;
	SM_VERTEX quadPoints[numVerts];
	ZeroMemory(&quadPoints, sizeof(SM_VERTEX) * numVerts);

	quadPoints[3].position = XMFLOAT4(1, -1, 0.1f, 0.0f);
	quadPoints[2].position = XMFLOAT4(-1, -1, 0.1f,0.0f);
	quadPoints[1].position = XMFLOAT4(-1, 1, 0.1f, 0.0f);
	quadPoints[0].position = XMFLOAT4(1, 1, 0.1f, 0.0f);

	quadPoints[0].uv = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	quadPoints[1].uv = XMFLOAT4(0.0f, 0, 0.0f, 0.0f);
	quadPoints[2].uv = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	quadPoints[3].uv = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f);

	// Create the vertex buffer storing vertsPoints
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = sizeof(SM_VERTEX) * numVerts;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.StructureByteStride = sizeof(SM_VERTEX);

	// Setting the resource data
	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData.pSysMem = &quadPoints;
	m_device->CreateBuffer(&bufferDesc, &resourceData, &m_quadPostBuffer.p);

	// Create indices.
	const unsigned int numIndices = 6;

	unsigned int indices[numIndices] = { 2,1,0,3,2,0 };//{ 0, 2, 3, 0, 1, 2 };
	//320210 
	// Fill in a buffer description.
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(unsigned int) * numIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(unsigned int);

	// Define the resource data.
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = indices;

	// Create the buffer with the device.
	m_device->CreateBuffer(&bufferDesc, &initData, &m_quadPostIndexBuffer.p);
}

void Graphics::CreateConstBuffers()
{
	// Create data const buffer 
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(POST_DATA_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_device->CreateBuffer(&constBufferDesc, NULL, &m_postDataConstBuffer.p);
}

void Graphics::CreateBlendState()
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	m_device->CreateBlendState(&blendDesc, &m_blendTransperentState.p);

	//ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	//blendDesc.RenderTarget[0].BlendEnable = FALSE;
	//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	//blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	//blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//
	//m_device->CreateBlendState(&blendDesc, &blendClassicState.p);
}

void Graphics::OpenCSOShaders()
{
	//ID3DBlob* PS_Buffer;
	//ID3DReadFileToBlob(L"PixelShader.cso", &PS_Buffer);
}

// Sets the color of the raster
void Graphics::SetBackColor(float _r, float _g, float _b, float _a)
{
	m_backColor.SetColor(_r, _g, _b, _a);
}

void Graphics::RenderText(const wchar_t* _text, XMFLOAT2 _pos, FXMVECTOR _color, unsigned int _fontType)
{
	//POINT coordinate;
	//coordinate.x = LONG(_pos.x);
	//coordinate.y = LONG(_pos.y);
	
	//XMFLOAT2 position = XMFLOAT2(float(coordinate.x), float(coordinate.y));
	
	//position.x = float((Graphics::GetBackBufferWidth() * 0.5f) - position.x) - 100.0f;
	//position.y = (float((Graphics::GetBackBufferHeight() * 0.5f) - position.y)) + 5.0f;

	XMFLOAT2 position = XMFLOAT2(_pos.x, _pos.y);

	// Converting te ratio to be between 0 and max of screen
    position.x = ((-float(Graphics::GetBackBufferWidth()) * position.x) + (Graphics::GetBackBufferWidth() * 0.5f));
	position.y = ((-float(Graphics::GetBackBufferHeight()) * position.y) + (Graphics::GetBackBufferHeight() * 0.5f));

	// Draw sprite
	g_Sprites->Begin(SpriteSortMode::SpriteSortMode_Immediate);
	switch (_fontType)
	{
	case 0:
		g_FontFrank->DrawString(g_Sprites.get(), _text, position, _color);
		break;
	case 1:
		g_FontItalic->DrawString(g_Sprites.get(), _text, position, _color);
	default:
		break;
	}
		
	
	g_Sprites->End();

	// Don't read from their shaders anymore
	Graphics::GetDeviceContext()->OMSetDepthStencilState(m_depthState, 0);
	Graphics::GetDeviceContext()->RSSetState(m_rasterState);
	Graphics::GetDeviceContext()->PSSetSamplers(0, 1, &m_samplerState.p);
	float blendFactor[] = { 0, 0, 0, 0 };
	unsigned int sampleMask = 0xffffffff;
	m_deviceContext->OMSetBlendState(m_blendTransperentState, blendFactor, sampleMask);

}

#if 0
// TESTING:: Create struct buffer for screen 
void Graphics::CreateStructBuffer()
{
	// Creating the const buffer 
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(BUFFER_STRUCT) * sizeGrid;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(BUFFER_STRUCT);
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = m_bufferStruct;

	m_device->CreateBuffer(&bufferDesc, &initData, &m_structBuffer.p);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = sizeGrid;

	m_device->CreateShaderResourceView(m_structBuffer, &srvDesc, &m_resourceViewStruct.p);
}
#endif
void Graphics::ClearDepthBuffer()
{

	m_deviceContext->ClearDepthStencilView(m_postDepthView.p, D3D11_CLEAR_DEPTH, 1,0);
}

#if 0
void Graphics::SetIndicesOfLightsTEST(unsigned int _indexBuffer, unsigned int _indexLight)
{

	m_bufferStruct[_indexBuffer].index = _indexLight;
}

void Graphics::SetColorOfLightsTEST(unsigned int _indexBuffer, XMFLOAT4 _color)
{
	m_bufferStruct[_indexBuffer].color[0] = _color.x;
	m_bufferStruct[_indexBuffer].color[1] = _color.y;
	m_bufferStruct[_indexBuffer].color[2] = _color.z;
	m_bufferStruct[_indexBuffer].color[3] = _color.w;

}
#endif