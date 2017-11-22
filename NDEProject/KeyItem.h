#pragma once
#ifndef KeyItem_H
#define KeyItem_H
#include "BaseItem.h"

class KeyItem : public BaseItem
{
	vector<BaseItem*> m_doors;
public:
	KeyItem(unsigned int _serverId);
	~KeyItem();

	void Input();
	void Update();
	void Render();

	void AddDoor(BaseItem* _door) { m_doors.push_back(_door); }
	void Action(XMFLOAT4X4* _serverMatrices);
	int OnCollision();

};

#endif 