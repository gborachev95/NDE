#pragma once
#include "BaseItem.h"
class Particle;
class BlastGrenade : public BaseItem
{
	Particle * m_particleEffect;
	BaseObject* m_caster;
	XMFLOAT4X4 m_cam;

public:
	BlastGrenade(unsigned int _serverId);
	~BlastGrenade();

	void Input();
	void Update();
	void Render();

	void Action(XMFLOAT4X4* _serverMatrices);
	void Init();
	int OnCollision();
	void SetCaster(BaseObject* _obj) { m_caster = _obj; }
	void SetCam(XMFLOAT4X4 _cam) { m_cam = _cam; }
	Particle* GetParticle() { return m_particleEffect; }
	BaseObject* GetCaster() { return m_caster; }
};

