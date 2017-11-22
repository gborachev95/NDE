#pragma once
#ifndef Runner_H
#define Runner_H
#include "Player.h"

class BaseItem;
class Runner : public Player
{
	Capsule   m_pushBox;
	Capsule   m_tripBox;
	BaseItem* m_item;
	bool      m_availability;
	BaseObject* push1, *push2;
	BaseObject* trip1, *trip2;
	XMFLOAT3  m_collisionPoint; //Used to rotate runner towards monster position upon death

public:
	Runner(BaseObject* _push);
	Runner(float _speed, float _mass);
	Runner(BaseObject* _push1, BaseObject* _push2, float _speed, float _mass);
	~Runner();

	
	void Input();
	void Update();
	void Render();
	// Getters
	Capsule GetPushBox() { return m_pushBox; }
	Capsule  GetTripBox() { return m_tripBox; }
	BaseItem* GetItem() { return m_item; }
	bool GetAvailability() { return m_availability; }  
	XMFLOAT3 GetCollisionPoint() { return m_collisionPoint; }

	// Setters
	void SetPushBoxActive(bool _active) { m_pushBox.m_active = _active; }
	void SetTripBoxActive(bool _active) { m_tripBox.m_active = _active; }
	void SetPushBox(Capsule& _capsule) { m_pushBox = _capsule; }
	void SetTripBox(Capsule& _hitBox) { m_tripBox = _hitBox; }
	void SetItem(BaseItem* _item) { m_item = _item; }
	void SetAvailability(bool _available) { m_availability = _available;}
	void SetCollisionPoint(XMFLOAT3 _colPoint) { m_collisionPoint = _colPoint; }
	void SetBones(BaseObject* _p1, BaseObject* _p2, BaseObject* _t1, BaseObject* _t2) { push1 = _p1; push2 = _p2; trip1 = _t1; trip2 = _t2;}

	// Client functions
	void PickUPAsClient();
	void DropAsCilent();

	// Other
	void UpdatePushAndTripBox();
	void PickUp();
	void Drop();

};

#endif