#pragma once
#ifndef Physics_H
#define Physics_H
#include <DirectXMath.h>
class BaseObject;

class Physics
{

public:
	Physics();
	~Physics();

	static void ApplyForce(BaseObject* _obj, DirectX::XMFLOAT4X4& worldMatrix, float _deltaTime, BaseObject* level, unsigned int& _animOut);
	static bool WithinBounds(BaseObject* _obj, DirectX::XMFLOAT4X4& worldMatrix, BaseObject* _level, float _deltaTime, unsigned int& _animOut);

};

#endif