#pragma once
#include "BaseItem.h"
class Trap : public BaseItem
{
public:
	Trap(unsigned int _serverId);
	~Trap();

	void Input();
	void Update();
	void Render();

	void Action(XMFLOAT4X4* _serverMatrices);
	int OnCollision();
};

