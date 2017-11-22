#pragma once
#ifndef Graphics_H
#define Graphics_H
#include "includes.h"
#include "Keyboard.h"
#include "XTime.h"
#include "Mouse.h"

enum POST_EFFECT_TYPE {NO_EFFECT, FLASH_EFFECT };
enum GRAPHICS { OBJECT, ANIMATED, POST_PROCESSING, PARTICLE, UI,INSTANCED, TARGET };

/*__declspec(align(16)) */class Graphics
{
private:
	// Window handles
	static HWND						         m_window;
	static HINSTANCE				         m_application;
	static WNDPROC						     m_appWndProc;

	// Graphics handles
	static CComPtr<IDXGISwapChain>		     m_swapChain;
	static CComPtr<ID3D11Device>		     m_device;
	static CComPtr<ID3D11DeviceContext>      m_deviceContext;
	static CComPtr<ID3D11RasterizerState>    m_rasterState;
	static CComPtr<ID3D11RasterizerState>    m_wireRasterState;
	static CComPtr<ID3D11Texture2D>	         m_depthBuffer;
	static CComPtr<ID3D11DepthStencilView>   m_depthView;
	static CComPtr<ID3D11DepthStencilState>  m_depthState;
	static CComPtr<ID3D11DepthStencilState>  m_depthStateFront;
	static CComPtr<ID3D11RenderTargetView>   m_renderTargetView;
	static D3D11_VIEWPORT				     m_viewPort;
	static CComPtr<ID3D11SamplerState>       m_samplerState;
	static CComPtr<ID3D11BlendState>         m_blendTransperentState;
	static CComPtr<ID3D11Buffer>             m_structBuffer;
	static CComPtr<ID3D11ShaderResourceView> m_resourceViewStruct;
	static COLOR                             m_backColor;
	static BUFFER_STRUCT                     *m_bufferStruct;
	static bool                              m_resized;
	static bool                              m_fullscreen;
	static bool                              m_debugWire; 
	static unsigned int                      m_backBufferWidth;
	static unsigned int                      m_backBufferHeight;
	// Post rendering data
	static CComPtr<ID3D11RenderTargetView>   m_postRenderTargetView;
	static CComPtr<ID3D11ShaderResourceView> m_postShaderResourceView;
	static CComPtr<ID3D11DepthStencilView>   m_postDepthView;
	static CComPtr<ID3D11Buffer>             m_postDataConstBuffer;
	static CComPtr<ID3D11Buffer>             m_quadPostBuffer;
	static CComPtr<ID3D11Buffer>             m_quadPostIndexBuffer;
	static CComPtr<ID3D11Texture2D>	         m_postTexture;
	static CComPtr<ID3D11Texture2D>	         m_postDepthBuffer;
	static CComPtr<ID3D11ShaderResourceView> m_postZShaderToResourceView;
	static POST_DATA_TO_VRAM                 m_postData;
	static XTime                             m_timer;

	// Private methods that are called only inside of the class
private:
	// Creates the window
	void CreateAppWindow(HINSTANCE _hinst, WNDPROC _proc);
	static void CreateDepthBuffer();
	void CreateViewPorts();
	void CreateLayouts();
	void CreateShaders();
	void CreateSamplerState();
	void CreateRasterState();
	static void CreatePostProcessingTexture();
	void CreatePostFilterQuad();
	void CreateConstBuffers();
	void CreateBlendState();
	void OpenCSOShaders();
	static void ResizeTextures(unsigned int _width, unsigned int _height);
	static void ResizePPTexture(unsigned int _width, unsigned int _height);

	// Public methods that are called outside of the class
public:

	// Containers
	static std::vector<CComPtr<ID3D11VertexShader>> m_vsShaders;
	static std::vector<CComPtr<ID3D11PixelShader>>  m_psShaders;
	static std::vector<CComPtr<ID3D11InputLayout>>  m_layouts;
	static std::unique_ptr<Keyboard>                single_keyboard;
	static std::unique_ptr<Mouse>                   single_mouse;

	//m_keyboard = std::make_unique<Keyboard>();
	// Constructor
	Graphics();
	Graphics(HINSTANCE _hinst, WNDPROC _proc);
	// Destructor
	~Graphics();

	void Initialize();
	void Render();
	void PostRender();
	static void ResizeWindow();
	bool ShutDown();
	static void CreateStructBuffer();
	static void ClearDepthBuffer();
	static void ClearScreen(COLOR _color);
	static void ClearBaseScreen(COLOR _color);
	// Getters
	static HWND GetWindow() { return m_window; }
	static HINSTANCE GetApplication() { return m_application; }
	static WNDPROC	GetWinProc() { return m_appWndProc; }
	static CComPtr<ID3D11Device> GetDevice() { return m_device; }
	static CComPtr<ID3D11DeviceContext> GetDeviceContext() { return m_deviceContext; }
	static CComPtr<IDXGISwapChain>	GetSwapChain() { return m_swapChain; }
	static bool GetIsResized() { return m_resized; }
	static unsigned int GetBackBufferHeight() { return m_backBufferHeight; }
	static unsigned int GetBackBufferWidth() { return m_backBufferWidth; }
	static POST_DATA_TO_VRAM GetPostData() { return m_postData; }
	static XTime GetTime() { return m_timer; }
	static ID3D11DepthStencilState* GetDepthState() { return m_depthState.p; }
	static ID3D11RasterizerState* GetRasterState() { return m_rasterState.p; }
	static ID3D11SamplerState* GetSamplerState() { return m_samplerState.p; }
	static CComPtr<ID3D11DepthStencilView> GetDepthView() { return m_depthView; }
	static void RenderText(const wchar_t* _text, XMFLOAT2 _pos, FXMVECTOR _color, unsigned int _fontType);
	static bool GetWireFrameStatus() { return m_debugWire; }
	//static ID3D11Buffer* GetStructuredBuffer() { return m_structBuffer.p; }
	static ID3D11DepthStencilState* GetDepthStncilFrontBuffer() { return m_depthStateFront; }

	// Setters
	static void SetBackColor(float _r, float _g, float _b, float _a);
	static void SetResized(bool _resized) { m_resized = _resized; }
	static void SetPostData(float _sandEffect) { m_postData.data[1] = _sandEffect; }
	static void SetWireFrameStatus(bool _wireStatus) {m_debugWire = _wireStatus; }
	//static void SetIndicesOfLightsTEST(unsigned int _indexBuffer, unsigned int _indexLight);
	//static void SetColorOfLightsTEST(unsigned int _indexBuffer, XMFLOAT4 _color);
	// Alligning by 16 bytes so we don't get a warning 
	//void* operator new(size_t i){return _mm_malloc(i,16);}
	//
	//void operator delete(void* p){_mm_free(p);}
	
};

#endif