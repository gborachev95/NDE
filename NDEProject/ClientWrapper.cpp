#include "ClientWrapper.h"
#include "NetworkClient.h"
#include "PlayState.h"

using namespace NetworkingPlatform;

ClientWrapper::ClientWrapper()
{
	m_client = new NetworkClient();
}

ClientWrapper::~ClientWrapper()
{
	if (m_client)
		delete m_client;
}

int ClientWrapper::Initialize(const char* address, uint16_t port)
{
	if (startup())
		return NETWORK_STARTUP_ERROR;
	return m_client->Initialize(address, port);
}

int ClientWrapper::Run()
{
	return m_client->Run();
}

void ClientWrapper::Stop()
{
	m_client->Stop();
	shutdown();
}

int ClientWrapper::SendData(float _mouseX, uint8_t _actionsInput)
{
	return m_client->SendData(_mouseX,_actionsInput);
}

void ClientWrapper::GetPlayerData(NetworkPlayers& _data)
{
	m_client->GetPlayerData(_data);
}

void ClientWrapper::GetItemData(NetworkObjects& _data)
{
	m_client->GetItemData(_data);
}

NetworkClient* ClientWrapper::GetClient()
{
	return m_client;
}

unsigned int ClientWrapper::GetClientID()
{
	return m_client->GetID();
}
