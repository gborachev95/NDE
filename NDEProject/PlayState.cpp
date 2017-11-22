#include "PlayState.h"
#include "Camera.h"
#include "ObjManager.h"
#include "Graphics.h"
#include "Light.h"
#include "ClientWrapper.h"
#include "Game.h"
#include "SoundManager.h"
#include "Button.h"
#include "BaseUI.h"
#include "Player.h"
#include "Collision.h"

#define NUM_POINTLIGHTS 125
// Statics
Camera*          PlayState::m_playerCamera;
bool             PlayState::m_playStayRunning = true;
uint8_t          PlayState::m_checkWin = false;
float            PlayState::m_gameTime;
uint32_t         PlayState::m_playerScore[8];

PlayState::PlayState()
{
	m_client = nullptr;
	m_options = false;
	m_endScreen = false;
	m_winner = -1;
	m_monstersWon = false;
	m_txtAlpha = 1.0f;
	m_showMSG = WAITING_PLAYERS;
	m_menuScreen = false;
	m_tab = false;
}

PlayState::~PlayState()
{
	delete m_playerCamera;
	delete m_freeCamera;
	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		delete m_sceneLight[i];
	m_sceneLight.clear();

	ObjManager::ClearObjects();

	delete m_canvasQuad;

	// Clear buttons
	for (unsigned int i = 0; i < m_menuButtons.size(); ++i)
		delete m_menuButtons[i];
	m_menuButtons.clear();
	for (unsigned int i = 0; i < m_endButtons.size(); ++i)
		delete m_endButtons[i];
	m_endButtons.clear();
	for (unsigned int i = 0; i < m_optionsButtons.size(); ++i)
		delete m_optionsButtons[i];
	m_optionsButtons.clear();

	delete m_cursor;
}

void PlayState::Input()
{
	if (Graphics::single_keyboard->GetState().D0)
		m_freeCameraStatus = true;
	else if(Graphics::single_keyboard->GetState().D9)
		m_freeCameraStatus = false;

	if (Graphics::single_keyboard->GetState().Tab)
		m_tab = true;
	else
		m_tab = false;

	if (!m_menuScreen && !m_endScreen)
	{
		if (!m_freeCameraStatus)
			ObjManager::Input();

		//for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		//	m_sceneLight[i]->Input();
	}
	// Camera *Update
	if (!m_menuScreen && !m_endScreen)
	{
		if (!m_freeCameraStatus)
			m_playerCamera->Input();
		else
			m_freeCamera->Input();
	}

}

void PlayState::Update()
{
	if (m_gameTime > 0.0f)
	{
		SoundManager::GetSound(GAME_BACKGROUND_SOUND)->SetVolume(0.7f);
		SoundManager::GetSound(GAME_BACKGROUND_SOUND)->Resume();
	}
	else
		SoundManager::GetSound(GAME_BACKGROUND_SOUND)->Pause();

	ObjManager::Update();
	SoundManager::Update();

	for (unsigned int i = 0; i < m_sceneLight.size(); ++i)
		m_sceneLight[i]->Update();

	Menu();
	OptionsButtons();
	EndButtons();

	// For initial message
	if (m_gameTime <= 0.0f) m_showMSG = WAITING_PLAYERS;
	else if (m_gameTime < 4.5f)
	{
		m_showMSG = COUNT_DOWN;
		m_txtAlpha -= 0.016f;
		if (m_txtAlpha < 0.0f)
			m_txtAlpha = 1.0f;
	}
	else if (m_gameTime < 11.5f)
	{
		if (m_gameTime < 5.0f)
			m_txtAlpha = 1.0f;

		m_showMSG = FIND_KEY;
		m_txtAlpha -= 0.01f;
		if (m_txtAlpha < 0.0f)
			m_txtAlpha = 0.0f;
	}
	else
	{   
		if (m_gameTime < 12.0f)
			m_txtAlpha = 1.0f;

		m_showMSG = MONSTERS_RELEASED;
		m_txtAlpha -= 0.01f;
		if (m_txtAlpha < 0.0f)
			m_txtAlpha = 0.0f;
	}
	if (m_menuScreen || m_endScreen || m_options)
		UpdateCursor();

	SoundControl();

	if (m_endScreen)
	{
		if (m_winner == m_client->GetClientID())
		{
			if (!m_winSongPlayed)
			{
				m_winSongPlayed = true;
				SoundManager::PlaySoundEffct(WIN_SOUND, false);
			}
		}
		else
		{
			if (!m_loseSongPlayed)
			{
				m_loseSongPlayed = true;
				SoundManager::PlaySoundEffct(LOSE_SOUND, false);
			}
		}

	}
	else
	{
		m_loseSongPlayed = false;
		m_winSongPlayed = false;
	}

	if (m_gameTime > 4.0f && m_gameTime < 5.0f)
		SoundManager::PlaySoundEffct(START_GATE_RUNNER_SOUND, false);
	if (m_gameTime > 11.0f && m_gameTime < 12.0f)
		SoundManager::PlaySoundEffct(START_GATE_MONSTER_SOUND, false);
}

void PlayState::Render() const
{
	unsigned int i = 0;
	
	// Game renders
	ObjManager::Render(m_playerCamera);
	if (!m_freeCameraStatus)
		m_playerCamera->Render();
	else
		m_freeCamera->Render();
	for (i = 0; i < m_sceneLight.size(); ++i)
		m_sceneLight[i]->Render(Graphics::GetDeviceContext());

	// UI renders
	if (m_menuScreen && !m_endScreen && !m_options)
	{
		m_cursor->Render();
		for (i = 0; i < m_menuButtons.size(); ++i)
			m_menuButtons[i]->Render();
		m_canvasQuad->Render();
	}
	if (m_endScreen)
	{
		m_cursor->Render();
		for (i = 0; i < m_endButtons.size(); ++i)
			m_endButtons[i]->Render();
		m_canvasQuad->Render();
	}
	if (m_options && !m_endScreen)
	{
		m_cursor->Render();
		for (i = 0; i < m_optionsButtons.size(); ++i)
			m_optionsButtons[i]->Render();
		m_canvasQuad->Render();
	}

	if (m_tab)
		m_canvasQuad->Render();

	if (!m_menuScreen)
		ShowMessage();
}

void PlayState::Enter()
{
	Graphics::SetPostData(-0.1f);
	m_freeCameraStatus = false;

	// Set the back color to blue
	//Graphics::SetBackColor(0.39f, 0.58f, 0.92f, 1.0f);
	Graphics::SetBackColor(0.5f, 0.5f, 0.5f, 1.0f);


	// Parent the camera to the right object
	unsigned int objIndex = 0;
	if (m_client)
		objIndex = m_client->GetClientID();
	m_playerCamera->SetParentObject(&ObjManager::AccessObject(objIndex));

	// Set the camera to the correct propotions
	m_playerCamera->ResizeCameraProjection();

	// So we have a reference in PlayState of the main client obj.
	m_thisPlayer = ObjManager::GetObjects()[objIndex];

	// Options are false
	m_options = false;
	m_endScreen = false;
	m_checkWin = false;
	m_winner = -1;
	m_monstersWon = false;
	m_menuScreen = false;
	m_loseSongPlayed = false;
	m_winSongPlayed = false;

	SoundManager::PlaySoundEffct(START_SCREECH_SOUND, false);
	SoundManager::GetSound(MENU_BACKGROUND_SOUND)->Pause();
}

void PlayState::Exit()
{
	Graphics::SetPostData(-0.3f);
	ObjManager::RefreshObjects();
	SoundManager::GetSound(GAME_BACKGROUND_SOUND)->Pause();
	SoundManager::GetSound(GAME_BACKGROUND_SOUND)->Pause();

	SoundManager::GetSound(MENU_BACKGROUND_SOUND)->Resume();
}

void PlayState::Init()
{
	m_gameTime = 0.0f;
	// Creating
	m_playerCamera = new Camera();
	m_freeCamera = new Camera();
	// Lights stuff
	COLOR lightColorON;
	COLOR lightColorOFF;
	lightColorON.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	lightColorOFF.SetColor(1.0f, 1.0f, 1.0f, 0.0f);

	// Dir Light
	m_sceneLight.push_back(new Light(Graphics::GetDevice(), DIR_LIGHT,   XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, -0.5f, 0.0f), 0.0f, lightColorON));

#if NUM_POINTLIGHTS 

	// Point Lights
	vector<XMFLOAT4> _positions;
	vector<XMFLOAT3> _directions;
	vector<float>    _radius;
	vector<COLOR>    _colors;

#if 1
	float rat = 300.0f;
	COLOR c; 
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-20.0f, 130.972f, -12.375f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);

	c.SetColor(0.823f, 0.966f, 0.966f, 1.0f);
	_positions.push_back(XMFLOAT4(-11.1f, 61.931f, -154.15f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);

	c.SetColor(0.962f, 1.0f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(21.99f, 81.90f, -98.383f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);

	c.SetColor(0.962f, 1.0f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(38.68f, 113.931f, 26.722f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.0f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-7.62f, 81.9f, -98.383f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.0f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-35.60f, 81.9f, -98.383f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(21.99f, 81.900f, -137.558f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-7.62f, 81.900f, -137.558f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-35.60f, 81.9f, -137.558f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(21.99f, 81.900f, -166.822f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-7.62f, 81.9f, -166.822f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-35.60f, 81.9f, -166.822f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-84.08f, 82.7f, -26.482f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(38.68f, 89.039f, 64.986f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-2.828f, 54.518f, 57.434f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(62.11f, 0.329f, 72.601f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(125.301f, -2.289f, 8.089f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);

	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(149.147f, 29.918f, -62.989f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(78.738f, 52.771f, -39.662f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-80.418f, 30.945f, 33.848f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-53.614f, 95.801f, 4.234f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-75.474f, 16.301f, -66.172f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-6.772f, 17.281f, -93.822f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(37.167f, 17.281f, -57.318f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(24.327f, 54.86f, -20.925f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-38.687f, 44.302f, 22.78f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-36.687f, 84.762f, 24.309f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(-15.188f, 84.762f, 32.443f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
	
	c.SetColor(0.962f, 1.000f, 0.613f, 1.0f);
	_positions.push_back(XMFLOAT4(36.248f, -6.873f, -75.563f, 1.0f));
	_colors.push_back(c);
	_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_radius.push_back(rat);
#endif

	for (unsigned int i = 29; i < NUM_POINTLIGHTS; ++i)
	{
		_positions.push_back(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		_directions.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
		_radius.push_back(0);
		_colors.push_back(c);
	}
	m_sceneLight.push_back(new Light(Graphics::GetDevice(), POINT_LIGHT, _positions, _directions, _radius, _colors, NUM_POINTLIGHTS,30.0f));

	_positions.clear();
	_directions.clear();
	_radius.clear();
	_colors.clear();

#endif

	// Loading and initializing
	ObjManager::Load(PLAY_STATE, Graphics::GetDevice());
	
	m_playerCamera->Initialize(RTP_CAM);
	m_freeCamera->Initialize(FP_CAM);

	LoadButtons();

	m_cursor = new BaseUI();
	m_cursor->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\OBJ Files\\MAIN_ButtonUI.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f, UI);
	m_cursor->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.01f, 0.01f, 0.01f), m_cursor->GetWorldMatrix()));
	m_cursor->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\cursor.dds");
}

void PlayState::Menu()
{
	m_canvasQuad->Update();

	static double timer = 0;
	timer += Graphics::GetTime().Delta();
	if (Graphics::single_keyboard->GetState().Escape && !m_menuScreen && timer > 1.0f)
	{
		timer = 0;
		m_menuScreen = true;
	}
	else if (Graphics::single_keyboard->GetState().Escape && timer > 1.0f && !m_options)
	{
		timer = 0;
		m_menuScreen = false;
	}
	else if (Graphics::single_keyboard->GetState().Escape && timer > 1.0f && m_options)
	{
		timer = 0;
		m_options = false;
	}

	if (!m_options)
		MenuButtons();
}

void PlayState::MenuButtons()
{
	if (m_menuScreen && !m_endScreen && !m_options)
	{
		for (unsigned int i = 0; i < m_menuButtons.size(); ++i)
		{
			if (m_menuButtons[i]->DetectCollision())
			{
				if (GetAsyncKeyState(VK_LBUTTON) && Graphics::single_mouse->GetState().leftButton)
				{
					switch (m_menuButtons[i]->GetButtonType())
					{
					case RESUME_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						m_menuScreen = false;
						while (GetAsyncKeyState(VK_LBUTTON)) {}
						break;
					}
					case OPTIONS_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
					    m_options = true;
						while (GetAsyncKeyState(VK_LBUTTON)) {}
						break;
					}
					case QUIT_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						
						if (!m_client)
							m_client = nullptr;
						
						// Loading screen
						float timer = 0.0f;
						while (timer < 1.0f)
						{
							timer += float(Graphics::GetTime().Delta());
							m_canvasQuad->Render();
							FXMVECTOR clr = { 1.0f,1.0f,1.0f, 1.0f };
							Graphics::GetSwapChain()->Present(1, 0);
						}
						Game::ChangeState(NETWORK_LOBBY_STATE);
						break;
					}
					}
				}
			}
		}
	}
}

void PlayState::OptionsButtons()
{
	if (m_options && !m_endScreen)
	{
		BaseObject* obj = ObjManager::GetObjects()[m_client->GetClientID()];
		obj->SetForce(XMFLOAT3(0.0f, obj->GetForce().y, 0.0f));

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

void PlayState::EndButtons()
{
	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
		if (CHECK_BIT(m_checkWin, i))
		{
			m_winner = i;
			if (ObjManager::GetObjects()[m_winner]->GetID() == MONSTER_ID)
				m_monstersWon = true;

			m_endScreen = true;
		}
	}

	if (m_endScreen)
	{
		m_endScreen = true;

		BaseObject* obj = ObjManager::GetObjects()[m_client->GetClientID()];
		obj->SetForce(XMFLOAT3(0.0f, obj->GetForce().y, 0.0f));

		for (unsigned int i = 0; i < m_endButtons.size(); ++i)
		{
			if (m_endButtons[i]->DetectCollision())
			{
				if (GetAsyncKeyState(VK_LBUTTON) && Graphics::single_mouse->GetState().leftButton)
				{
					switch (m_endButtons[i]->GetButtonType())
					{
					case RESUME_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						ObjManager::RefreshObjects();
						m_endScreen = false;
						m_monstersWon = false;
						for (unsigned int i = 0; i < 5000; ++i){}
						break;

					}
					case QUIT_BUTTON:
					{
						SoundManager::PlaySoundEffct(BUTTON_CLICK_SOUND, false);
						ObjManager::RefreshObjects();
						m_endScreen = false;
						m_monstersWon = false;
						if (m_client)
							m_client = nullptr;

						// Loading screen
						float timer = 0.0f;
						while (timer < 1.0f)
						{
							timer += float(Graphics::GetTime().Delta());
							m_canvasQuad->Render();
							FXMVECTOR clr = { 1.0f,1.0f,1.0f, 1.0f };
							Graphics::RenderText(L"", XMFLOAT2(50, 0.0f), clr, 0);
							Graphics::GetSwapChain()->Present(1, 0);
						}

						m_loseSongPlayed = false;
						m_winSongPlayed = false;
						Game::ChangeState(NETWORK_LOBBY_STATE);
						break;
					}
					}
				}
			}
		}
	}
}

void PlayState::ShowMessage() const
{
	XMVECTORF32 BlackTrans = { 0.0f, 0.0f, 0.0f,m_txtAlpha };
	XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMVECTORF32 Orange = { 0.9803f, 0.4078f,0.0f, 1.0f };
	XMVECTORF32 Cobalt = { 0.0f, 0.3137f, 0.9372f, 1.0f };
	XMVECTORF32 Crimson = { 0.6352f, 0.0f, 0.1450f, 1.0f };
	XMVECTORF32 Violet = { 0.6666f, 0.0f, 1.0f, 1.0f };


	// Renders the final screen
	if (m_endScreen)
	{
		if (m_winner == m_client->GetClientID())
			Graphics::RenderText(L"You Win", XMFLOAT2(0.1f,0.5f), White,1);
		else if (m_monstersWon && ObjManager::GetObjects()[m_client->GetClientID()]->GetID() == MONSTER_ID)
			Graphics::RenderText(L"You Win", XMFLOAT2(0.1f, 0.5f), White,1);
		else
			Graphics::RenderText(L"You Lose", XMFLOAT2(0.1f, 0.5f), White,1);
	}
	else
	{
		// Renders the timer
		wstring t = to_wstring(m_gameTime);
		t.pop_back(); t.pop_back(); t.pop_back(); t.pop_back(); t.pop_back();

		Graphics::RenderText(t.c_str(), XMFLOAT2(-0.4f, 0.5f), White, 1);

		switch (m_showMSG)
		{
		case WAITING_PLAYERS:
			Graphics::RenderText(L"Waiting players", XMFLOAT2(0.1f, 0.5f), BlackTrans, 1);
			break;
		case COUNT_DOWN:
			SoundManager::PlaySoundEffct(COUNTDOWN_SOUND, false);
			break;
		case FIND_KEY:
			Graphics::RenderText(L"Find key", XMFLOAT2(0.1f, 0.5f), BlackTrans, 1);
			break;
		case MONSTERS_RELEASED:
			Graphics::RenderText(L"Monster Released", XMFLOAT2(0.1f, 0.5f), BlackTrans, 1);
			break;
		default:
			break;
		}
	}

	if (m_tab)
	{
		NetworkPlayers msg;
		m_client->GetPlayerData(msg);
		if (CHECK_BIT(msg.m_active, 0))
		{
			wstring t = L"Monster: " + to_wstring(m_playerScore[0]);
			Graphics::RenderText(t.c_str(), XMFLOAT2(0.5f, 0.3f), White, 1);
		}

		if (CHECK_BIT(msg.m_active, 1))
		{
			wstring t1 = L"Runner 1: " + to_wstring(m_playerScore[1]);
			Graphics::RenderText(t1.c_str(), XMFLOAT2(0.5f, 0.1f), Cobalt, 1);
		}

		if (CHECK_BIT(msg.m_active, 2))
		{
			wstring t2 = L"Runner 2: " + to_wstring(m_playerScore[2]);
			Graphics::RenderText(t2.c_str(), XMFLOAT2(0.5f, -0.1f), Orange, 1);
		}

		if (CHECK_BIT(msg.m_active, 3))
		{
			wstring t3 = L"Runner 3: " + to_wstring(m_playerScore[3]);
			Graphics::RenderText(t3.c_str(), XMFLOAT2(0.5f, -0.3f), Violet, 1);
		}
	}
}

void PlayState::LoadButtons()
{
	// Loading options quad
	MOD_TO_VRAM toVramData;
	toVramData.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f);
	m_canvasQuad = new BaseUI();
	m_canvasQuad->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\CanvasUI.obj", XMFLOAT3(0.0f, -2.0f, 0.0f), 0, UI);
	m_canvasQuad->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\nothing.dds");
	m_canvasQuad->SetToVramData(toVramData);
	//m_canvasQuad->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.5f, 1.5f, 0.5f), m_canvasQuad->GetWorldMatrix()));


	// Loading menu buttons
	m_menuButtons.resize(3);
	for (unsigned int i = 0; i < m_menuButtons.size(); ++i)
	{
		m_menuButtons[i] = new Button();
		m_menuButtons[i]->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.1f, 0.05f, 0.1f), m_menuButtons[i]->GetWorldMatrix()));
	}

	m_menuButtons[0]->SetPosition(-0.6f, 0.5f, 0.0f);
	m_menuButtons[0]->SetButtonType(RESUME_BUTTON);
	m_menuButtons[0]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_RESUME_BUTTON_MAP_RESCALED.dds");

	m_menuButtons[1]->SetPosition(0.0f, 0.5f, 0.0f);
	m_menuButtons[1]->SetButtonType(OPTIONS_BUTTON);
	m_menuButtons[1]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_OPTIONS_BUTTON_MAP_RESCALED.dds");

	m_menuButtons[2]->SetPosition(0.6f, 0.5f, 0.0f);
	m_menuButtons[2]->SetButtonType(QUIT_BUTTON);
	m_menuButtons[2]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_QUIT_BUTTON_MAP_RESCALED.dds");


	// Loading end buttons
	m_endButtons.resize(2);
	for (unsigned int i = 0; i < m_endButtons.size(); ++i)
	{
		m_endButtons[i] = new Button();
		m_endButtons[i]->SetWorldMatrix(XMMatrixMultiply(XMMatrixScaling(0.1f, 0.05f, 0.1f), m_endButtons[i]->GetWorldMatrix()));
	}
	
	m_endButtons[0]->SetPosition(-0.6f, 0.5f, 0.0f);
	m_endButtons[0]->SetButtonType(RESUME_BUTTON);
	m_endButtons[0]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_RESUME_BUTTON_MAP_RESCALED.dds");
	
	m_endButtons[1]->SetPosition(0.6f, 0.5f, 0.0f);
	m_endButtons[1]->SetButtonType(QUIT_BUTTON);
	m_endButtons[1]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_QUIT_BUTTON_MAP_RESCALED.dds");
	
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
	
	m_optionsButtons[2]->SetPosition(0.0f,-0.1f, 0.0f);
	m_optionsButtons[2]->SetButtonType(BACK_BUTTON);
	m_optionsButtons[2]->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_BACK_BUTTON_MAP_RESCALED.dds");
}

void PlayState::UpdateCursor()
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

void PlayState::SoundControl()
{
	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
		XMFLOAT3 pos1, pos2;
		XMStoreFloat3(&pos1, ObjManager::AccessObject(i).GetWorldMatrix().r[3]);
		XMStoreFloat3(&pos2, m_thisPlayer->GetWorldMatrix().r[3]);
		

		float dist = Collision::DistanceFormula(pos1, pos2);
		if (dist <= 30.0f)
		{
			float volRatio = dist / 30.0f;
			volRatio = 1 - volRatio;
			for (unsigned int soundIndex = 0; soundIndex < SoundManager::GetPlayersSoundsSize(); ++soundIndex)
			{
				float volumeInDistance = volRatio;
				SoundManager::SetPlayerSoundVolume(i, soundIndex, volumeInDistance);
			}
		}
	}
}