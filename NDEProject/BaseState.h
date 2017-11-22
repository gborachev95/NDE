#pragma once
#ifndef BaseState_H
#define BaseState_H
class Graphics;

class BaseState
{
public:
	BaseState();
	virtual ~BaseState();
	virtual void Input() = 0;
	virtual void Update() = 0;
	virtual void Render() const = 0;
	virtual void Enter() = 0;
	virtual void Exit() = 0;
	virtual void Init() = 0;
};
#endif