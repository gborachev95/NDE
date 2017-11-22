#pragma once
#ifndef NetworkLobbyState_H
#define NetworkLobbyState_H
#include "BaseState.h"
#include <thread>
#include <DirectXMath.h>
#include <vector>

#define LOBBY_BUTTONS 3
class ServerWrapper;
class ClientWrapper;
class Button;
class Light;
class Camera;
class BaseObject;
class NetworkLobbyState : public BaseState
{
	std::string              m_ipAddress;
	static ServerWrapper     *m_server;
	static ClientWrapper     *m_client;
	std::thread              *m_serverThread;
	std::thread              *m_clientThread;
	Camera                   *m_mainCamera;
	std::vector<Light*>      m_sceneLight;
	std::vector<Button*>     m_buttons;
	DirectX::XMFLOAT3        m_buttonPositions[LOBBY_BUTTONS];
	std::vector<BaseObject*> m_envObj;
	BaseObject*              m_cursor;

private:
	void UpdateCursor();
public:
	NetworkLobbyState();
	~NetworkLobbyState();

	void Input();
	void Update();
	void Render() const;
	void Enter();
	void Exit();
	void Init();

	bool Host(const char* _ipAddress);
	bool Join(const char* _ipAddress);
	int RunServerThread();
	int RunClientThread();
	void EnterIPAddress();

	static ClientWrapper* GetClient() { return m_client; }
	static ServerWrapper* GetServer() { return m_server; }

};

#endif