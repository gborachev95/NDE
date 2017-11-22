#pragma once
#include "BaseObject.h"
#include <DirectXMath.h>

class BaseObject;
enum ITEM_TYPE { BASE_ITEM, KEY_ITEM, POCKET_SAND_ITEM, TRAP_ITEM, ADRENALINE_ITEM, FREEZING_GRENADE_ITEM, MAGNET_GRENADE_ITEM, BLAST_GRENADE_ITEM , DOOR_ITEM, SPAWN_ITEM};

class BaseItem : public BaseObject
{
	bool         m_carried;
	bool         m_dangerous; // Happens only on server 
	bool         m_isColliding; // Happens only on server 
	bool         m_particle;
	ITEM_TYPE    m_type;
	BaseObject*  m_parent; // Happens only on server 
	unsigned int m_mapToServerID; // Happens only on server 
	float        m_expireTime; // Happens only on server 
	float        m_activateTime; // Happens only on server 
	float        m_respawnTime; // Happens only on server 

public:
	BaseItem(unsigned int _serverId);
	~BaseItem();

	virtual void Input();
	virtual void Update();
	virtual void Render();
	void Carried();
	virtual void Action(XMFLOAT4X4* _serverMatrices);
	virtual int OnCollision();
	vector<unsigned int> CheckCollision(XMFLOAT3 *cP);
	virtual bool CheckExpiration(float _delta);
	virtual void Init();
	
	// Setters
	void SetParent(BaseObject* _parent) { m_parent = _parent; }
	void SetIsCarried(bool _isCarried) { m_carried = _isCarried; }
	void SetItemType(ITEM_TYPE _type) { m_type = _type; }
	void SetToServerID(unsigned int _id) { m_mapToServerID = _id; }
	void SetDangerous(bool _dangerous) { m_dangerous = _dangerous; }
	void AddToExpireTime(float _time) { m_expireTime += _time; }
	void AddToActivatedTime(float _time) { m_activateTime += _time; }
	void ResetExpireTime() { m_expireTime = 0.0f; }
	void SetRespawnTime(float _resTime) { m_respawnTime = _resTime; }
	void SetIsColliding(bool _col) { m_isColliding = _col; }
	void SetParticleActive(bool _part) { m_particle = _part; }
	void ResetActivatedTime() { m_activateTime = 0.0f; }

	// Getters
	BaseObject* GetParent() { return m_parent; }
	bool GetIsCarried() { return m_carried; }
	ITEM_TYPE GetItemType() { return m_type; }
	unsigned int GetServerID() { return m_mapToServerID; }
	bool GetDangerous() { return m_dangerous; }
	float GetExpirationTime() { return m_expireTime; }
	float GetActivatedTime() { return m_activateTime ; }
	float GetRespawnTime() { return m_respawnTime; }
	bool GetIsColliding() { return m_isColliding; }
	bool IsParticleActive() { return m_particle; }
};

