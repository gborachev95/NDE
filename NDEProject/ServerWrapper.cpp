#include "ServerWrapper.h"
#include "NetworkServer.h"
#include <thread>
#include "PlayState.h"

using namespace NetworkingPlatform;

ServerWrapper::ServerWrapper()
{
	m_server = new NetworkServer();
}

ServerWrapper::~ServerWrapper()
{
	delete m_server;
}

int ServerWrapper::Initialize(const char* _address, uint16_t _port)
{
	if (startup())
		return NETWORK_STARTUP_ERROR;

	return m_server->Initialize(_address,_port);
}

int ServerWrapper::Update()
{
	return m_server->Update();
}

void ServerWrapper::Stop()
{
	m_server->Stop();
	shutdown();
}
