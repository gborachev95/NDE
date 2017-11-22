#include "Game.h"
#include "Graphics.h"
#include "BaseState.h"
#include "PlayState.h"
#include "NetworkLobbyState.h"
#include "MenuState.h"
#include "Log.h"
#include <thread>
#include "BaseObject.h"
#include "SpriteBatch.h"
#include "ObjManager.h"

// Statics
BaseState*           Game::m_currentState = NULL;
BaseState**          Game::m_states = NULL;
Graphics*            Game::m_graphics = NULL;
bool                 Game::m_gameOn;
BaseObject*          Game::m_loadingScreen;

Game::Game()
{
}

Game::Game(HINSTANCE _hinst, WNDPROC _proc)
{
	// Sets up the console
	//LogSetUp(L"NDE");
	
	m_graphics = new Graphics(_hinst, _proc);
	m_states = new BaseState*[MAX_STATES];

	// Initial screen
	//BaseObject *initScreen = new BaseObject(BASE_ID, 0, false);
	//initScreen->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\CanvasUI.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 0, OBJECT);
	//initScreen->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\nothing.dds");
	COLOR col;
	//while (true)
	//{
		col.SetColor(0.5f, 0.5f, 0.5f, 1.0f);
		m_graphics->ClearBaseScreen(col);
	//	m_graphics->Render();
//		initScreen->Render();
	//	m_graphics->PostRender();
		m_graphics->GetSwapChain()->Present(0, 0);
	//}
	//delete initScreen;
	
	// Initializing states 
	m_states[MENU_STATE] = new MenuState();
	m_states[NETWORK_LOBBY_STATE] = new NetworkLobbyState();
	m_states[PLAY_STATE] = new PlayState();
	for (int i = 0; i < MAX_STATES; ++i)
		m_states[i]->Init();

	m_currentState = m_states[MENU_STATE];
	m_currentState->Enter();

	m_frame = 0;
	m_gameOn = true;
	m_keypressed = false;
	m_loadingScreen = new BaseObject(BASE_ID, 0, false);
	m_loadingScreen->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\Canvas.obj", XMFLOAT3(0.0f, -2.0f, 0.0f), 0, OBJECT);
	m_loadingScreen->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\nothing.dds");

}

Game::~Game()
{
	// Clear out states
	for (int i = 0; i < MAX_STATES; ++i)
		delete m_states[i];
	delete[] m_states;

	// Clear out graphics
	delete m_graphics;

	delete m_loadingScreen;
}

void Game::Input()
{
	if (Graphics::single_keyboard->GetState().F2 && !m_keypressed)
	{
		m_keypressed = true;
		Graphics::SetWireFrameStatus(!Graphics::GetWireFrameStatus());
	}
	if (!Graphics::single_keyboard->GetState().F2)
		m_keypressed = false;

	m_currentState->Input();
}

void Game::Update()
{
	m_currentState->Update();
}

void Game::Render()
{
	m_graphics->Render();
	m_currentState->Render();
	m_graphics->PostRender();
	// 1 for frame capping
	m_graphics->GetSwapChain()->Present(0/*1*/, 0);
}

bool Game::Run()
{
	//if (GetActiveWindow() == Graphics::GetWindow())
	Input();
	Update();
	Render();

	return m_gameOn;
}

void Game::ChangeState(STATE _currState)
{
	m_currentState->Exit();
	m_currentState = m_states[_currState];
	m_currentState->Enter();

	if (_currState == PLAY_STATE)
	{
		// Loading screne
		float timer = 0.0f;
		while (timer < 0.5f)
		{
			timer += float(Graphics::GetTime().Delta());
			m_loadingScreen->Render();
			m_graphics->GetSwapChain()->Present(1, 0);
		}
	}
}

void Game::Shutdown()
{	
	m_graphics->ShutDown();

	// Loading screne
	//float timer = 0.0f;
	//while (timer < 2.5f)
	//{
	//	timer += float(Graphics::GetTime().Delta());
		m_loadingScreen->Render();
		m_graphics->GetSwapChain()->Present(1, 0);
	
	//}
}