#pragma once
#ifndef ObjManager_H
#define ObjManager_H
#include <vector>
#include "BaseObject.h"
#include "Game.h"
#include "includes.h"
#include "Camera.h"
#include "BaseItem.h"

class Collision;
class ClientWrapper;
class Player;

class ObjManager
{
	static std::vector<BaseObject*> m_objectsList;
	static BaseObject*              m_playerHoldingKey;
	static std::vector<Player*>     m_playerList;
public:
	ObjManager();
	~ObjManager();
	static void Input();
	static void Update();
	static void Render(Camera* _camera);
	static void Load(STATE _currState, CComPtr<ID3D11Device> _device);
	// All objects
	static std::vector<BaseObject*>& GetObjects() { return m_objectsList; }
	static BaseObject&  AccessObject(unsigned int _index) { return (*m_objectsList[_index]); }
	static void ClearObjects();
	static void RefreshObjects();
	static void RefreshObject(unsigned int _indexs);
	static BaseItem* GetAvailableItem(BaseItem* _item);
	static void RemoveObject(BaseItem* _item);
	static bool CheckForItems();

private:
	static void LoadWalls(CComPtr<ID3D11Device> _device);
	static void LoadItems(CComPtr<ID3D11Device> _device);
	static void UpdateItem(unsigned int _index);
	static void SortRender();
};
#endif
