#pragma once
#include "BaseItem.h"
class Particle;
class PocketSand :	public BaseItem
{
	Particle * m_particleEffect;
	BaseObject* m_caster;
	XMFLOAT4X4 m_cam;

public:
	PocketSand(unsigned int _serverId);
	~PocketSand();
	
	void Input();
	void Update();
	void Render();

	void Action(XMFLOAT4X4* _serverMatrices, XMFLOAT3 _force);
	void Init();
	int OnCollision();
	BaseObject* GetCaster() { return m_caster; }
	void SetCaster(BaseObject* _obj) { m_caster = _obj; }
	void SetCam(XMFLOAT4X4 _cam) { m_cam = _cam; }
	Particle* GetParticle() { return m_particleEffect; }
};


