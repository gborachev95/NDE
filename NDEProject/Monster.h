#pragma once
#ifndef Monster_H
#define Monster_H
#include "Player.h"
class Monster : public Player
{
	//BaseObject* push1, *push2;
	Capsule   m_swipeBox;
	bool m_isDeadly;
	float m_charge;
public:
	Monster();
	Monster(BaseObject* _push1, BaseObject* _push2, float _speed, float _mass);
	Monster(float _speed, float _mass);
	~Monster();

	bool GetIsDeadly() { return m_isDeadly; }
	void SetIsDeadly(bool _dead) { m_isDeadly = _dead; }
	float GetCharge() { return m_charge; }
	void SetCharge(float _charge) { m_charge = _charge; }
	Capsule GetSwipeBox() { return m_swipeBox; }
	void SetSwipeBox(bool _swipe) { m_swipeBox.m_active = _swipe; }

	void Input();
	void Update();
	void Render();

	void UpdateSwipeBox();
	void Pounce();
};

#endif