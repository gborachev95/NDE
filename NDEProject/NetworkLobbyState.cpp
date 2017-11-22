#include "NetworkLobbyState.h"
#include "ServerWrapper.h"
#include "ClientWrapper.h"
#include "PlayState.h"
#include "NetworkClient.h"
#include "Graphics.h"
#include "Game.h"
#include "Button.h"
#include "Light.h"
#include "Camera.h"
#include "SoundManager.h"
#include <iostream>
#include "SpriteBatch.h"
#include <Windows.h>
#include <mutex>
#include "Player.h"
#include "ObjManager.h"

// Defines
#define PORT 32327
#define NUM_POINTLIGHTS 125

// Statics
ServerWrapper* NetworkLobbyState::m_server = nullptr;
ClientWrapper* NetworkLobbyState::m_client = nullptr;

std::mutex g_serverMutex;
std::mutex g_clientMutex;

NetworkLobbyState::NetworkLobbyState()
{
	m_server = nullptr;
	m_client = nullptr;
	m_serverThread = nullptr;
	m_clientThread = nullptr;
}

NetworkLobbyState::~NetworkLobbyState()
{
	if (m_server)
		delete m_server;

	if (m_serverThread)
		delete m_serverThread;

	if (m_clientThread)
		delete m_clientThread;

	if (m_client)
		delete m_client;

	for (unsigned int i = 0; i < m_buttons.size(); ++i)
		delete m_buttons[i];
	m_buttons.clear();

	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		delete m_sceneLight[i];
	m_sceneLight.clear();

	delete m_mainCamera;


	for (unsigned int i = 0; i < m_envObj.size(); ++i)
		delete m_envObj[i];
	m_envObj.clear();
	
	delete m_cursor;
}

void NetworkLobbyState::Input()
{
	auto kb = Graphics::single_keyboard->GetState();

	EnterIPAddress();

	for (unsigned int i = 0; i < m_buttons.size(); ++i)
	{
		if (m_buttons[i]->DetectCollision())
		{
			if (GetAsyncKeyState(VK_LBUTTON) && Graphics::single_mouse->GetState().leftButton)
			{
				switch (m_buttons[i]->GetButtonType())
				{
				case HOST_BUTTON:
				{
					if (m_server == nullptr)
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						Host(m_ipAddress.c_str()); // Local
					}
					break;
				}
				case JOIN_BUTTON:
				{
					if (m_client == nullptr)
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						Join(m_ipAddress.c_str()); // Local
					}
					break;
				}
				case BACK_BUTTON:
				{
					SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
					Game::ChangeState(MENU_STATE);
					break;
				}
				default:
					break;
				}
			}
		}
	}
	//m_mainCamera->Input();
}

void NetworkLobbyState::Update()
{
	SoundManager::Update();
	for (unsigned int i = 0; i < m_buttons.size(); ++i)
		m_buttons[i]->Update();

	UpdateCursor();
}

void NetworkLobbyState::Render() const
{
	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		m_sceneLight[i]->Render(Graphics::GetDeviceContext());

	m_mainCamera->Render();

	for (unsigned int i = 0; i < m_buttons.size(); ++i)
		m_buttons[i]->Render();

	for (unsigned int i = 0; i < m_envObj.size(); ++i)
		m_envObj[i]->Render();

	if (m_ipAddress.size() > 0)
	{
		wchar_t showIP[20] = L"";
		for (unsigned int i = 0; i < m_ipAddress.size(); ++i)
			showIP[i] = (wchar_t)m_ipAddress[i];

		Graphics::RenderText(showIP, XMFLOAT2(0.1f,0.45f), DirectX::Colors::WhiteSmoke,0);
	}

	m_cursor->Render();
}

void NetworkLobbyState::Enter()
{
	while (GetAsyncKeyState(VK_LBUTTON)){}
	Graphics::SetBackColor(0.5f, 0.5f, 0.5f, 1.0f);

	m_mainCamera->ResizeCameraProjection();


	if (m_server)
	{   
		m_server->Stop();
		delete m_server;
		m_server = nullptr;
	}

	g_clientMutex.lock();
	if (m_client)
	{
		printf("Exited on Main thread\n");

		m_client->Stop();
		delete m_client;
		m_client = nullptr;
	}
	g_clientMutex.unlock();	
}

void NetworkLobbyState::Exit()
{
}

void NetworkLobbyState::Init()
{
	m_ipAddress = "127.0.0.1";

	// Lights stuff
	COLOR lightColorON;
	COLOR lightColorOFF;
	lightColorON.SetColor(0.9176f, 0.7254f, 0.5333f,1.0f);
	lightColorOFF.SetColor(1.0f, 1.0f, 1.0f, 0.0f);

	// Dir Light
	m_sceneLight.push_back(new Light(Graphics::GetDevice(), DIR_LIGHT, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, -0.5f, 0.0f), 0.0f, lightColorOFF));


#if NUM_POINTLIGHTS
	// Point Lights
	vector<XMFLOAT4> _positions;
	vector<XMFLOAT3> _directions;
	vector<float>    _radius;
	vector<COLOR>    _colors;

	_positions.push_back(XMFLOAT4(0.0f, 5.0f, 1.0f, 1.0f));
	for (unsigned int i = 0; i < NUM_POINTLIGHTS; ++i)
	{
		_positions.push_back(XMFLOAT4(-5.0f, 5.0f, 1.0f, 1.0f));
		_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
		_radius.push_back(3000.0f);
		_colors.push_back(lightColorON);
	}
	m_sceneLight.push_back(new Light(Graphics::GetDevice(), POINT_LIGHT, _positions, _directions, _radius, _colors, NUM_POINTLIGHTS,125.0f));
#endif

	m_mainCamera = new Camera();
	m_mainCamera->Initialize(FP_CAM);

	// Button stable positions
	m_buttonPositions[0] = XMFLOAT3(15.0f, 3.0f, -20.0f);
	m_buttonPositions[1] = XMFLOAT3(0.0f, 1.0f, -20.0f);
	m_buttonPositions[2] = XMFLOAT3(-15.0f, 3.0f, -20.0f);

	m_buttons.resize(LOBBY_BUTTONS);

	m_buttons[0] = new Button(m_buttonPositions[0].x, m_buttonPositions[0].y, m_buttonPositions[0].z, HOST_BUTTON);
	m_buttons[1] = new Button(m_buttonPositions[1].x, m_buttonPositions[1].y, m_buttonPositions[1].z, JOIN_BUTTON);
	m_buttons[2] = new Button(m_buttonPositions[2].x, m_buttonPositions[2].y, m_buttonPositions[2].z, BACK_BUTTON);

	// Load enviorment
	m_envObj.push_back(new BaseObject(BASE_ID, 0.0f, false));
	m_envObj[0]->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\menucan.obj", XMFLOAT3(0.0f, 0.0f, -100.0f), 0, OBJECT);
	m_envObj[0]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\StoneBrick1_AlbedoBUTCHERED.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	// Ground
	m_envObj.push_back(new BaseObject(BASE_ID, 0.0f, false));
	m_envObj[1]->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\ground.obj", XMFLOAT3(0, -5.0f, 0), 0, OBJECT);
	m_envObj[1]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\StoneBrick1_AlbedoBUTCHERED.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	
	//m_envObj.push_back(new BaseObject(BASE_ID, 0.0f, false));
	//m_envObj[2]->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\Canvas.obj", XMFLOAT3(0.0f, 50.0f, 0.0f), 0, OBJECT);
	//m_envObj[2]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\nothing.dds");
#if 1
	// Instancing
	BaseObject* instanced = new BaseObject();
	XMFLOAT4X4* mat = new XMFLOAT4X4[20];

	float x = 30.0f;
	float z = -30.0f;
	for (unsigned int i = 0; i < 20; ++i)
	{
		XMStoreFloat4x4(&mat[i], XMMatrixIdentity());
		mat[i]._41 = x;
		mat[i]._42 = -7.0f;
		mat[i]._43 = z;

		x -= 4.0f;

		float rndNum = float(rand() % 15 + 5);
		XMStoreFloat4x4(&mat[i], XMMatrixMultiply(XMMatrixScaling(rndNum, rndNum, rndNum), XMLoadFloat4x4(&mat[i])));
	}
	instanced->InitializeInstances(mat, 20);
	instanced->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\StalagmiteLow.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 1, INSTANCED);
	instanced->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\Enviroment\\Stalagimate_DEF2.dds", L"..\\NDEProject\\Assets\\Textures\\Enviroment\\StalagimateLow_Normal.dds");

	m_envObj.push_back(instanced);
	delete mat;
#endif

	m_cursor = new BaseUI();
	m_cursor->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\OBJ Files\\MAIN_ButtonUI.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f, UI);
	m_cursor->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), m_cursor->GetWorldMatrix()));
	m_cursor->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\cursor.dds");
}

// Checks if clicked on host and creating a server.
bool NetworkLobbyState::Host(const char* _ipAddress)
{
	m_ipAddress = { _ipAddress };

	// Starting the server
	m_server = new ServerWrapper();
	if (m_server->Initialize(_ipAddress, PORT) != NETWORK_SUCCESS)
	{
		m_server->Stop();
		delete m_server;
		m_server = nullptr;
		return false;
	}

	// Running a server thread
	m_serverThread = new std::thread(&NetworkLobbyState::RunServerThread, this);
	m_serverThread->detach();

	// Starting the client
	m_client = new ClientWrapper();
	if (m_client->Initialize(m_ipAddress.c_str(), PORT) != NETWORK_SUCCESS)
	{
		m_server->Stop();
		m_client->Stop();
		delete m_server;
		m_server = nullptr;
		delete m_client;
		m_client = nullptr;
		return false;
	}

	// Running a client thread
	m_clientThread = new std::thread(&NetworkLobbyState::RunClientThread, this);
	m_clientThread->detach();

	// ADDED FROM WAITING STATE
	dynamic_cast<PlayState*>(Game::GetStates()[PLAY_STATE])->SetClient(m_client);
	dynamic_cast<Player*>(ObjManager::GetObjects()[m_client->GetClientID()])->SetClient(m_client);

	Game::ChangeState(PLAY_STATE);
	return true;
}

// Checks if clicked on join and joining a server
bool NetworkLobbyState::Join(const char* _ipAddress)
{
	m_ipAddress = { _ipAddress };

	// Starting the client
	m_client = new ClientWrapper();
	if (m_client->Initialize(m_ipAddress.c_str(), PORT) != NETWORK_SUCCESS)
	{
		m_client->Stop();
		delete m_client;
		m_client = nullptr;
		return false;
	}

	// Running a client thread
	m_clientThread = new std::thread(&NetworkLobbyState::RunClientThread, this);
	m_clientThread->detach();

	dynamic_cast<PlayState*>(Game::GetStates()[PLAY_STATE])->SetClient(m_client);
	dynamic_cast<Player*>(ObjManager::GetObjects()[m_client->GetClientID()])->SetClient(m_client);
	dynamic_cast<PlayState*>(Game::GetStates()[PLAY_STATE])->SetRunning(true);

	Game::ChangeState(PLAY_STATE);
	return true;
}

int NetworkLobbyState::RunServerThread()
{
	// Runs the server while there is network success
	do 
	{
		if (!m_server)
			break;
	} while (m_server->Update() == NETWORK_SUCCESS);

	// Stop the server if not running anymore
	if (m_server)
		m_server->Stop();

	delete m_serverThread;
	m_serverThread = nullptr;

	return NETWORK_SHUTDOWN;
}

int NetworkLobbyState::RunClientThread()
{
	// Runs the client while there is network success
	m_client->Run();

	// Stop the client if not running anymore
	g_clientMutex.lock();
	if (m_client)
	{   
		printf("Exited on Client thread\n");
		m_client->Stop();
		delete m_client;
		m_client = nullptr;
	}
	g_clientMutex.unlock();

	delete m_clientThread;
	m_clientThread = nullptr;

	return NETWORK_SHUTDOWN;
}

void NetworkLobbyState::EnterIPAddress()
{
	auto kb = Graphics::single_keyboard->GetState();
	static bool keypressed = true;

	if (m_ipAddress.size() < 19)
	{
		if ((kb.NumPad0 || kb.D0) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '0';
		}
		if ((kb.NumPad1 || kb.D1) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '1';
		}
		if ((kb.NumPad2 || kb.D2) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '2';
		}
		if ((kb.NumPad3 || kb.D3) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '3';
		}
		if ((kb.NumPad4 || kb.D4) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '4';
		}
		if ((kb.NumPad5 || kb.D5) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '5';
		}
		if ((kb.NumPad6 || kb.D6) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '6';
		}
		if ((kb.NumPad7 || kb.D7) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '7';
		}
		if ((kb.NumPad8 || kb.D8) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '8';
		}
		if ((kb.NumPad9 || kb.D9) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '9';
		}
		if ((kb.OemPeriod ) && !keypressed)
		{
			keypressed = true;
			m_ipAddress += '.';
		}
	}
	if (m_ipAddress.size() > 0)
	{
		if (kb.Back && !keypressed)
		{
			keypressed = true;
			m_ipAddress.pop_back();
		}
	}

	if (!kb.NumPad0 && !kb.NumPad1 && !kb.NumPad2 && !kb.NumPad3 && !kb.NumPad4 && !kb.NumPad5 && !kb.NumPad6 && !kb.NumPad7 && !kb.NumPad8 && !kb.NumPad9 && !kb.Back
		&& !kb.D0 && !kb.D1 && !kb.D2 && !kb.D3 && !kb.D4 && !kb.D5 && !kb.D6&& !kb.D7 && !kb.D8 && !kb.D9 && !kb.OemPeriod)
		keypressed = false;
	
}

void NetworkLobbyState::UpdateCursor()
{
	// Update Cursor pos
	POINT mousePos;
	ZeroMemory(&mousePos, sizeof(POINT));
	GetCursorPos(&mousePos);
	ScreenToClient(Graphics::GetWindow(), &mousePos);
	XMFLOAT3 position = XMFLOAT3(float(mousePos.x), float(mousePos.y), 0.0f);
	// Converting to a ratio
	position.x /= Graphics::GetBackBufferWidth();
	position.y /= Graphics::GetBackBufferHeight();
	position.x = ((-2 * position.x) + 1) * (-1);
	position.y = ((-2 * position.y) + 1);
	m_cursor->SetPosition(position.x, position.y, 0.0f);
}