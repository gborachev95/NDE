#pragma once
#ifndef Game_H
#define Game_H
#include <Windows.h>

class BaseState;
class Graphics;
class BaseObject;

enum STATE { MENU_STATE = 0, NETWORK_LOBBY_STATE, PLAY_STATE, MAX_STATES }; //  END_STATE, ROUNDEND_STATE

class Game
{
	// State objects
	static BaseState*  m_currentState;
	static BaseState** m_states;
					   
	// Game objects	   
	static Graphics*   m_graphics;
	static BaseObject* m_loadingScreen;
	int                m_frame;
	static bool        m_gameOn;
	bool               m_keypressed;
public:
	// Constructor
	Game();
	Game(HINSTANCE _hinst, WNDPROC _proc);
	// Destructor
	~Game();

	void Input();
	void Update();
	void Render();
	bool Run();
	static void ChangeState(STATE _currState);
	static BaseState** GetStates() { return m_states; }
	// Setters
	static void SetGameRunning(bool _running) { m_gameOn = _running; }
	void Shutdown();
};
#endif
