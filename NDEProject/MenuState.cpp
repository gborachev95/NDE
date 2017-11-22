#include "MenuState.h"
#include "Graphics.h"
#include "BaseObject.h"
#include "Camera.h"
#include "Light.h"
#include "Button.h"
#include <DirectXMath.h>
#include <iostream>
#include "Game.h"
#include "Particle.h"
#include "SoundManager.h"
#include "SpriteBatch.h"

#define PARTICLES 0
#define NUM_POINTLIGHTS 125

MenuState::MenuState()
{
}

MenuState::~MenuState()
{
	unsigned int i;
	for ( i = 0; i < m_buttons.size(); ++i)
		delete m_buttons[i];
	m_buttons.clear();

	for (i = 0; i < m_optionsButtons.size(); ++i)
		delete m_optionsButtons[i];
	m_optionsButtons.clear();

	for (i = 0; i < m_sceneLight.size(); ++i)
		delete m_sceneLight[i];
	m_sceneLight.clear();

	delete m_mainCamera;

	for (unsigned int i = 0; i < m_envObj.size(); ++i)
		delete m_envObj[i];
	m_envObj.clear();

#if PARTICLES
	for (unsigned int i = 0; i < m_particleSys.size(); ++i)
		delete m_particleSys[i];
#endif

	delete m_cursor;
}

void MenuState::Input()
{
	static float t_vol = 1.0f;
	if (Graphics::single_keyboard->GetState().K && t_vol < 1.0f)
		t_vol += 0.01f;
	else if (Graphics::single_keyboard->GetState().J && t_vol > 0.0f)
		t_vol -= 0.01f;

	if (Graphics::single_keyboard->GetState().M)
		SoundManager::Mute();

	SoundManager::SetSoundVolume(MENU_BACKGROUND_SOUND, t_vol);


	if (!m_options && !m_credits)
	{
		for (unsigned int i = 0; i < m_buttons.size(); ++i)
		{
			if (m_buttons[i]->DetectCollision())
			{
				if (GetAsyncKeyState(VK_LBUTTON) && Graphics::single_mouse->GetState().leftButton && !m_buttonClick)
				{
					switch (m_buttons[i]->GetButtonType())
					{
					case PLAY_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND,false);
						Game::ChangeState(NETWORK_LOBBY_STATE);
						m_buttonClick = true;
						break;
					}
					case OPTIONS_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						m_options = true;
						m_buttonClick = true;
						while (GetAsyncKeyState(VK_LBUTTON)) {}
						break;
					}
					case CREDITS_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						m_credits = true;
						m_buttonClick = true;
						break;
					}
					case EXIT_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						Game::SetGameRunning(false);
						m_buttonClick = true;
						break;
					}
					}
				}
			}
		}
	}
     

	// Input for particles
	for (unsigned int i = 0; i < m_particleSys.size(); ++i)
		m_particleSys[i]->Input();

	// Input for lights
	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		m_sceneLight[i]->Input();
	
	// Input for camera - used for testing
	//m_mainCamera->Input();
}

void MenuState::Update()
{
	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		m_sceneLight[i]->Update();

	for (unsigned int i = 0; i < m_buttons.size(); ++i)
		m_buttons[i]->Update();

	SoundManager::Update();
#if PARTICLES
	for (unsigned int i = 0; i < m_particleSys.size(); ++i)
	{
		m_particleSys[i]->SetBillBoardVector(m_mainCamera->GetViewMatrix().viewMatrix);
		m_particleSys[i]->Update(true, XMFLOAT4{ .75f, .75f, .75f, 1.0f });
	}
#endif

	Options();
	Credits();

	if (!GetAsyncKeyState(VK_LBUTTON) && !Graphics::single_mouse->GetState().leftButton)
		m_buttonClick = false;

	UpdateCursor();
}

void MenuState::Render() const
{
	m_cursor->Render();

	// Render camera
	m_mainCamera->Render();

	// Render scene lighting
	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		m_sceneLight[i]->Render(Graphics::GetDeviceContext());

	// Render menu buttons if neither options and credits are on
	if (!m_options && !m_credits)
	{
		for (unsigned int i = 0; i < m_buttons.size(); ++i)
			m_buttons[i]->Render();
	}
	if (m_options)
	{
		for (unsigned int i = 0; i < m_optionsButtons.size(); ++i)
			m_optionsButtons[i]->Render();
	}


	// Render enviroment
	if (!m_credits)
		for (unsigned int i = 0; i < m_envObj.size(); ++i)
			m_envObj[i]->Render();

	if (m_credits)
	{
		m_creditsQuad->Render();
	}
	
	// Render Particles
#if PARTICLES
	for (unsigned int i = 0; i < m_particleSys.size(); ++i)
		m_particleSys[i]->Render();
#endif

}

void MenuState::Enter()
{
	while (GetAsyncKeyState(VK_LBUTTON)){}
	Graphics::SetBackColor(0.0f,0.0f, 0.0f, 1.0f);
	m_mainCamera->ResizeCameraProjection();
	m_options = false;
	m_credits = false;
	m_buttonClick = false;
	m_sceneLight[0]->SetLightStatus(0.0f);
}

void MenuState::Exit()
{
	m_sceneLight[0]->SetLightStatus(1.0f);
}

void MenuState::Init()
{
	m_options = false;
	m_credits = false;
	m_buttonClick = false;

	// Lights stuff
	COLOR lightColorON;
	COLOR lightColorOFF;
	lightColorON.SetColor(0.9176f, 0.7254f, 0.5333f, 1.0f);
	lightColorOFF.SetColor(1.0f, 1.0f, 1.0f, 0.0f);

	// Dir Light
	m_sceneLight.push_back(new Light(Graphics::GetDevice(), DIR_LIGHT, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, -0.5f, 0.0f), 0.0f, lightColorOFF));


	// Camera initialization
	m_mainCamera = new Camera();
	m_mainCamera->Initialize(FP_CAM);

	LoadButtons();

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

	// Load enviorment
	m_envObj.push_back(new BaseObject(BASE_ID, 0.0f, false));
	m_envObj[0]->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\menucan.obj",XMFLOAT3(0.0f,0.0f,-100.0f), 0, OBJECT);
	m_envObj[0]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\StoneBrick1_AlbedoBUTCHERED.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");

	// Ground
	m_envObj.push_back( new BaseObject(BASE_ID, 0.0f, false));
	m_envObj[1]->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\ground.obj", XMFLOAT3(0,-5.0f,0), 0, OBJECT);
	m_envObj[1]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\StoneBrick1_AlbedoBUTCHERED.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");

#if 1
// Instancing
	BaseObject* instanced = new BaseObject();
	XMFLOAT4X4* mat = new XMFLOAT4X4[20];

	float x = 30.0f;
	float z = -30.0f;
	for (unsigned int i = 0; i < 20; ++i)
	{
		XMStoreFloat4x4(&mat[i], XMMatrixMultiply(XMMatrixRotationY(float(rand())), XMMatrixIdentity()));
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
	// Sounds
	SoundManager::Initialize();
	
#if PARTICLES
	for (unsigned int i = 0; i < 3; ++i)
	{
		m_particleSys.push_back(new Particle());
		m_particleSys[i]->Initialize(L"..\\NDEProject\\Assets\\Textures\\PAR_DUST.dds", m_buttonPositions[i], XMFLOAT3{ 1, 1, 1 }, XMFLOAT3{ 1, 1, 1 }, 50, 60.0f, 0.25f, WEIGHTED);
	}
#endif

	m_cursor = new BaseUI();
	m_cursor->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\OBJ Files\\MAIN_ButtonUI.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f, UI);
	m_cursor->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), m_cursor->GetWorldMatrix()));
	m_cursor->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\cursor.dds");
	
	m_creditsQuad = new BaseObject();
	m_creditsQuad->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\Canvas.obj", XMFLOAT3(25.0f, -7.9f, -13.0f), 0, OBJECT);
	m_creditsQuad->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\Credits.dds");

}

void MenuState::Options()
{
	if (m_options)
	{
		if (Graphics::single_keyboard->GetState().Escape)
			m_options = false;

			for (unsigned int i = 0; i < m_optionsButtons.size(); ++i)
			{
				if (m_optionsButtons[i]->DetectCollision())
				{
					if (GetAsyncKeyState(VK_LBUTTON) && Graphics::single_mouse->GetState().leftButton)
					{
						switch (m_optionsButtons[i]->GetButtonType())
						{
						case RESIZE_BUTTON:
						{
							SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
							Graphics::ResizeWindow();
							while (GetAsyncKeyState(VK_LBUTTON)) {}
							break;
						}
						case OPTIONS_BUTTON:
						{
							SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
							// TODO:: Set volume to float offset from moved object
							break;
						}
						case BACK_BUTTON:
						{
							SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
							m_options = false;
							while (GetAsyncKeyState(VK_LBUTTON)) {}
							break;
						}
						}
					}
				}
			}
	}
}

void MenuState::Credits()
{
	if (m_credits)
	{
		m_sceneLight[0]->SetLightStatus(1.0f);
		if (GetAsyncKeyState(VK_LBUTTON) && Graphics::single_mouse->GetState().leftButton && !m_buttonClick)
		{
			m_buttonClick = true;
			SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
			m_options = false;
			m_credits = false;
			m_sceneLight[0]->SetLightStatus(0.0f);
		}

		if (Graphics::single_keyboard->GetState().Escape)
		{
			m_sceneLight[0]->SetLightStatus(0.0f);
			m_credits = false;
		}
	}
}

void MenuState::LoadButtons()
{
	// Button stable positions
	m_buttonPositions[0] = XMFLOAT3(20.0f, 3.0f, -20.0f);
	m_buttonPositions[1] = XMFLOAT3(6.7f, 1.0f, -20.0f);
	m_buttonPositions[2] = XMFLOAT3(-6.7f, 1.0f, -20.0f);
	m_buttonPositions[3] = XMFLOAT3(-20.0f, 3.0f, -20.0f);

	// Loading menu buttons
	m_buttons.resize(4);
	m_buttons[0] = new Button(m_buttonPositions[0].x, m_buttonPositions[0].y, m_buttonPositions[0].z, PLAY_BUTTON);
	m_buttons[1] = new Button(m_buttonPositions[1].x, m_buttonPositions[1].y, m_buttonPositions[1].z, OPTIONS_BUTTON);
	m_buttons[2] = new Button(m_buttonPositions[2].x, m_buttonPositions[2].y, m_buttonPositions[2].z, CREDITS_BUTTON);
	m_buttons[3] = new Button(m_buttonPositions[3].x, m_buttonPositions[3].y, m_buttonPositions[3].z, EXIT_BUTTON);

	// Loading options buttons
	m_optionsButtons.resize(3);
	for (unsigned int i = 0; i < m_optionsButtons.size(); ++i)
	{
		m_optionsButtons[i] = new Button();
		m_optionsButtons[i]->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.1f, 0.05f, 0.1f), m_optionsButtons[i]->GetWorldMatrix()));
	}

	m_optionsButtons[0]->SetPosition(0.0f, 0.5f, 0.0f);
	m_optionsButtons[0]->SetButtonType(RESIZE_BUTTON);
	m_optionsButtons[0]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_RESIZE_BUTTON_MAP_RESCALED.dds");

	m_optionsButtons[1]->SetPosition(0.0f, 0.1f, 0.0f);
	m_optionsButtons[1]->SetButtonType(OPTIONS_BUTTON);
	m_optionsButtons[1]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_STONE_TEXTURE.dds");

	m_optionsButtons[2]->SetPosition(0.0f, -0.3f, 0.0f);
	m_optionsButtons[2]->SetButtonType(BACK_BUTTON);
	m_optionsButtons[2]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_BACK_BUTTON_MAP_RESCALED.dds");
}

void MenuState::UpdateCursor()
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
	position.y = ((-2 * position.y) + 1) - 0.05f;
	m_cursor->SetPosition(position.x, position.y, 0.0f);
}