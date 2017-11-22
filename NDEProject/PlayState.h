#pragma once
#ifndef PlayState_H
#define PlayState_H
#include "BaseState.h"
#include "includes.h"

class ObjManager;
class Camera;
class Graphics;
class Light;
class ClientWrapper;
class Button;
class BaseUI;
class BaseObject;
enum GAME{WAITING_PLAYERS,COUNT_DOWN, FIND_KEY, MONSTERS_RELEASED};
class PlayState : public BaseState
{
	static Camera          *m_playerCamera;
	Camera                 *m_freeCamera;
	std::vector<Light*>     m_sceneLight;
	ClientWrapper          *m_client;	
	bool                    m_options;
	bool                    m_endScreen;
	bool                    m_menuScreen;
	bool                    m_monstersWon; 
	static bool             m_playStayRunning;
	BaseUI*                 m_canvasQuad;
	std::vector<Button*>    m_menuButtons;
	std::vector<Button*>    m_endButtons;
	std::vector<Button*>    m_optionsButtons;
	static uint8_t          m_checkWin;
	static uint32_t         m_playerScore[8];
	int                     m_winner;
	static float            m_gameTime;
	GAME                    m_showMSG;
	float  	                m_txtAlpha;
	bool                    m_freeCameraStatus;
	bool                    m_tab;
	BaseObject*             m_cursor;
	BaseObject*             m_thisPlayer;
	bool                    m_winSongPlayed;
	bool                    m_loseSongPlayed;
	// Called only in the class
private:
	void ShowMessage() const;
	void Menu();
	void MenuButtons();
	void OptionsButtons();
	void EndButtons();
	void LoadButtons();
	void UpdateCursor();
	void SoundControl();
public:
	PlayState();
	~PlayState();
	void Input();
	void Update();
	void Render() const;
	void Enter();
	void Exit();
	void Init();

	// Setters
	void SetClient(ClientWrapper* _client) { m_client = _client; }
	static void SetRunning(bool _running) { m_playStayRunning = _running; }
	static void SetCheckWin(uint8_t _gameState) { m_checkWin = _gameState; }
	static void SetGameTime(float _time) { m_gameTime = _time; }
	static void SetUsersScore(const unsigned int _player, const unsigned int _score) { m_playerScore[_player] = _score; }

	// Getters
	ClientWrapper* GetClient() { return m_client; }
	static bool GetRunning() { return m_playStayRunning; }
	static uint8_t GetCheckWin() { return m_checkWin; }
	static float GetGameTime() { return m_gameTime; }
	static uint32_t GetUsersScore(const unsigned int _player) { return m_playerScore[_player]; }
	static Camera* GetPlayerCamera() { return m_playerCamera; }
};
#endif
