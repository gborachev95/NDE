#include "ObjManager.h"
#include "Player.h"
#include "Collision.h"
#include "Game.h"
#include "ClientWrapper.h"
#include <mutex>
#include "KeyItem.h"
#include "Runner.h"
#include "Monster.h"
#include "PocketSand.h"
#include "MagnetGranade.h"
#include "BlastGrenade.h"
#include "Trap.h"

// Defines
#define INSTANCES 10
#define MONSTER 1
#define LEVEL_VISUALS 1

// Statics
std::vector<BaseObject*> ObjManager::m_objectsList;
BaseObject*              ObjManager::m_playerHoldingKey;
std::vector<Player*>     ObjManager::m_playerList;

ObjManager::ObjManager()
{
}

ObjManager::~ObjManager()
{
	ClearObjects();
}

void ObjManager::Input()
{
	for (unsigned int i = 0; i < m_objectsList.size(); ++i)
	{
		switch (m_objectsList[i]->GetID())
		{
		case BASE_ID:
			m_objectsList[i]->Input();
			break;
		case PLAYER_ID:
			dynamic_cast<Player*>(m_objectsList[i])->Input();
			break;
		case RUNNER_ID:
			dynamic_cast<Runner*>(m_objectsList[i])->Input();
			break;
		case MONSTER_ID:
			dynamic_cast<Monster*>(m_objectsList[i])->Input();
			break;
		}			
	}
}

void ObjManager::Update()
{
	for (unsigned int i = 0; i < m_objectsList.size(); ++i)
	{
		switch (m_objectsList[i]->GetID())
		{
		case BASE_ID:
			m_objectsList[i]->Update();
			break;
		case PLAYER_ID:
			dynamic_cast<Player*>(m_objectsList[i])->Update();
			break;
		case RUNNER_ID:
			dynamic_cast<Runner*>(m_objectsList[i])->Update();
			break;
		case MONSTER_ID:
			dynamic_cast<Monster*>(m_objectsList[i])->Update();
			break;
		case ITEM_ID:
			UpdateItem(i);
			break;
		}
	}
}

void ObjManager::UpdateItem(unsigned int _index)
{
	switch (dynamic_cast<BaseItem*>(m_objectsList[_index])->GetItemType())
	{
	case KEY_ITEM:
		dynamic_cast<KeyItem*>(m_objectsList[_index])->Update();
		m_playerHoldingKey = dynamic_cast<BaseItem*>(m_objectsList[_index])->GetParent();

		if (m_playerHoldingKey != nullptr)
			m_playerHoldingKey = dynamic_cast<BaseItem*>(m_objectsList[_index])->GetParent();
		break;
	case POCKET_SAND_ITEM:
		dynamic_cast<PocketSand*>(m_objectsList[_index])->Update();
		break;
	case TRAP_ITEM:
		dynamic_cast<Trap*>(m_objectsList[_index])->Update();
		break;
	case ADRENALINE_ITEM:
		break;
	case FREEZING_GRENADE_ITEM:
		break;
	case MAGNET_GRENADE_ITEM:
		dynamic_cast<MagnetGranade*>(m_objectsList[_index])->Update();
		break;
	case BLAST_GRENADE_ITEM:
		dynamic_cast<BlastGrenade*>(m_objectsList[_index])->Update();
		break;
	}
}

void ObjManager::Render(Camera* _camera)
{
	bool keyBearer = false;
	unsigned int you = 0;
	// Render loop

	Player* kB = dynamic_cast<Player*>(m_playerHoldingKey);

	if (kB && kB->GetClient())
	{
		for (unsigned int i = 0; i < MAX_USERS; i++)
		{
			//if (dynamic_cast<Player*>(m_objectsList[i])->GetClient())
			//	if (dynamic_cast<Player*>(m_objectsList[i])->GetClient()->GetClientID() == i)
			//		if (kB->GetClient()->GetClientID() == i)
			//			keyBearer = true;
			if (m_playerList[i]->GetClient())
				if (m_playerList[i]->GetClient()->GetClientID() == i)
					if (kB->GetClient()->GetClientID() == i)
						keyBearer = true;

		}
	}

	for (int i = m_objectsList.size() - 2; i >= 0; --i)
	{
	
#if 0
		if (m_objectsList[i]->GetID() == MONSTER_ID)
		{
			if (dynamic_cast<Player*>(m_objectsList[i])->GetClient())
				if (dynamic_cast<Player*>(m_objectsList[i])->GetClient()->GetClientID() == i)
					continue;
		}
		//DEBUGGING
		if (m_objectsList[i]->GetID() == ITEM_ID)
		{
			if (dynamic_cast<BaseItem*>(m_objectsList[i])->GetItemType() == DOOR_ITEM)
				continue;
		}
		if (i == 24)
			int d = 3;
		if (m_objectsList[i]->GetID() == DEBUG_ID)
		{
			if (num < 4)
				m_objectsList[i]->SetPosition(_camera->GetFrustum().corners[num].x, _camera->GetFrustum().corners[num].y, _camera->GetFrustum().corners[num].z);
			num++;
		}

		 if (i == 24)
		 	int d = 3
		 if (m_objectsList[i]->GetID() == DEBUG_ID)
		 {
		 	if (num < 4)
		 		m_objectsList[i]->SetPosition(_camera->GetFrustum().corners[num].x, _camera->GetFrustum().corners[num].y, _camera->GetFrustum().corners[num].z);
		 	num++;
		 }
#endif
		 // Render the person with the key through walls
		if (m_playerHoldingKey && i == 22 && !keyBearer)
		{
			 Graphics::GetDeviceContext()->OMSetDepthStencilState(Graphics::GetDepthStncilFrontBuffer(), 0);
			 m_playerHoldingKey->Render();
			 Graphics::GetDeviceContext()->OMSetDepthStencilState(Graphics::GetDepthState(), 0);
		}

		// If it is piece of the level, always render it
		if (m_objectsList[i]->GetID() == LEVEL_ID || m_objectsList[i]->GetID() == MONSTERSTART_ID && m_objectsList[i]->GetTris()->size() == 0 || m_objectsList[i]->GetID() == RUNNERSTART_ID && m_objectsList[i]->GetTris()->size() == 0)
			m_objectsList[i]->Render();

		// Updating the items in Render because we have access to the camera
		if (m_objectsList[i]->GetID() == ITEM_ID)
		{
			BaseItem* item = dynamic_cast<BaseItem*>(m_objectsList[i]);
			if (item->GetItemType() == POCKET_SAND_ITEM)
			{
				PocketSand* pS = dynamic_cast<PocketSand*>(item);
				pS->SetCam(_camera->GetViewMatrix().viewMatrix);
			}
			else if (item->GetItemType() == BLAST_GRENADE_ITEM)
			{
				BlastGrenade* bG = dynamic_cast<BlastGrenade*>(item);
				bG->SetCam(_camera->GetViewMatrix().viewMatrix);
			}
			else if (item->GetItemType() == MAGNET_GRENADE_ITEM)
			{
				MagnetGranade* mG = dynamic_cast<MagnetGranade*>(item);
				mG->SetCam(_camera->GetViewMatrix().viewMatrix);
			}
		}
		if (m_objectsList[i]->GetID() != LEVEL_ID && m_objectsList[i]->GetID() != DEBUG_ID &&  m_objectsList[i]->GetID() != RUNNERSTART_ID &&  m_objectsList[i]->GetID() != MONSTERSTART_ID)
		{
			if (m_objectsList[i]->GetHitBox())
			{		//		if (m_objectsList[i]->GetHitBox()->type == CAPSULE_COL)
				{
					if (_camera->FrustumToCapsule(_camera->GetFrustum(), *dynamic_cast<Capsule*>(m_objectsList[i]->GetHitBox())))
						m_objectsList[i]->Render();

					continue;
				}
			}
		}

		// Frustum rendering
		if (m_objectsList[i]->GetID() != LEVEL_ID && m_objectsList[i]->GetID() != DEBUG_ID &&  m_objectsList[i]->GetID() != RUNNERSTART_ID &&  m_objectsList[i]->GetID() != MONSTERSTART_ID)
		{
			if (m_objectsList[i]->GetHitBox())
			{
				//		if (m_objectsList[i]->GetHitBox()->type == CAPSULE_COL)
				//{
				if (_camera->FrustumToCapsule(_camera->GetFrustum(), *dynamic_cast<Capsule*>(m_objectsList[i]->GetHitBox())))
					m_objectsList[i]->Render();
				continue;
				//}
			}
		}

		// Render everythng
		//m_objectsList[i]->Render();
	}
}

void ObjManager::Load(STATE _currState, CComPtr<ID3D11Device> _device)
{	
	// Loading Runners, they have to be first in list because of Networking
	XMFLOAT3 initPosition = XMFLOAT3{ -10.0f, 0.0f, 0.0f };

	vector<wchar_t*> mageMaps = { L"..\\NDEProject\\Assets\\Textures\\MageTexture.dds", L"..\\NDEProject\\Assets\\Textures\\DEF_CobaltMage.dds",
		L"..\\NDEProject\\Assets\\Textures\\DEF_OrangeMage.dds", L"..\\NDEProject\\Assets\\Textures\\DEF_VioletMage.dds",
		L"..\\NDEProject\\Assets\\Textures\\MageTexture.dds", L"..\\NDEProject\\Assets\\Textures\\MageTexture.dds", 
		L"..\\NDEProject\\Assets\\Textures\\MageTexture.dds" };

#if MONSTER 
   m_objectsList.push_back(new Monster(23.0f, 3.0f));

   XMFLOAT4X4 world;
   XMStoreFloat4x4(&world, m_objectsList[0]->GetWorldMatrix());
   m_objectsList[0]->InitializeInstances(&world, 2);

   m_objectsList[0]->InstantiateFBX(_device, "..\\NDEProject\\Assets\\FBX Files\\Monster\\*", initPosition, 1, ANIMATED);
   m_objectsList[0]->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\Monster\\Monster_Base_Color.dds", L"..\\NDEProject\\Assets\\Textures\\Monster\\Monster_Normal_OpenGL.dds", L"..\\NDEProject\\Assets\\Textures\\mageSpecularMap.dds");
   m_objectsList[0]->AttachHitBox(7.0f, 5.2f, 0.5f, CAPSULE_COL);

   m_playerList.push_back(dynamic_cast<Player*>(m_objectsList[0]));
	for (unsigned int i = 1; i < MAX_USERS; ++i)
	{
		XMFLOAT3 playerPos{ initPosition.x + i * 5, initPosition.y,initPosition.z };
	
		m_objectsList.push_back(new Runner(23.0f, 3.0f));

		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, m_objectsList[i]->GetWorldMatrix());
		m_objectsList[i]->InitializeInstances(&world, 2);

		m_objectsList[i]->InstantiateFBX(_device, "..\\NDEProject\\Assets\\FBX Files\\Runner\\*", playerPos, 1, ANIMATED);
		m_objectsList[i]->TextureObject(_device, mageMaps[i], L"..\\NDEProject\\Assets\\Textures\\NoNormalMap.dds", L"..\\NDEProject\\Assets\\Textures\\mageSpecularMap.dds");
		m_objectsList[i]->AttachHitBox(7.0f, 5.2f, 0.5f, CAPSULE_COL);

		m_playerList.push_back(dynamic_cast<Player*>(m_objectsList[i]));

	}

#else
	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
		XMFLOAT3 playerPos{ initPosition.x + i * 5, initPosition.y,initPosition.z };
		m_objectsList.push_back(new Runner(21.0f, 3.5f));

		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, m_objectsList[i]->GetWorldMatrix());
		m_objectsList[i]->InitializeInstances(&world, 2);

		m_objectsList[i]->InstantiateFBX(_device, "..\\NDEProject\\Assets\\FBX Files\\Runner\\*", playerPos, 1, ANIMATED);
		m_objectsList[i]->TextureObject(_device, mageMaps[i], L"..\\NDEProject\\Assets\\Textures\\NoNormalMap.dds", L"..\\NDEProject\\Assets\\Textures\\mageSpecularMap.dds");
		m_objectsList[i]->AttachHitBox(7.0f, 5.2f, 0.5f, CAPSULE_COL);
		m_playerList.push_back(dynamic_cast<Player*>(m_objectsList[i]));
	}
#endif

	// Loading Items
	LoadItems(_device);

	// For Debugging
#if 0 
	//BaseObject* BonePush = new BaseObject(DEBUG_ID, 0.0f, false);
	//XMFLOAT3 bonePushPos{ 0, 0, 0 };
	//BonePush->InstantiateModel(_device, "..\\NDEProject\\Assets\\boneSphere.obj", bonePushPos, 0, OBJECT);
	//BonePush->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	//m_objectsList.push_back(BonePush);
	//
	//BaseObject* BonePush1 = new BaseObject(DEBUG_ID, 0.0f, false);
	//XMFLOAT3 bonePushPos1{ 0, 0, 0 };
	//BonePush1->InstantiateModel(_device, "..\\NDEProject\\Assets\\boneSphere.obj", bonePushPos1, 0, OBJECT);
	//BonePush1->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	//m_objectsList.push_back(BonePush1);
	//
	//BaseObject* BoneTrip = new BaseObject(DEBUG_ID, 0.0f, false);
	//XMFLOAT3 boneTripPos{ 0, 0, 0 };
	//BoneTrip->InstantiateModel(_device, "..\\NDEProject\\Assets\\boneSphere.obj", boneTripPos, 0, OBJECT);
	//BoneTrip->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	//m_objectsList.push_back(BoneTrip);
	//
	//BaseObject* BoneTrip1 = new BaseObject(DEBUG_ID, 0.0f, false);
	//XMFLOAT3 boneTripPos1{ 0, 0, 0 };
	//BoneTrip1->InstantiateModel(_device, "..\\NDEProject\\Assets\\boneSphere.obj", boneTripPos1, 0, OBJECT);
	//BoneTrip1->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	//m_objectsList.push_back(BoneTrip1);
	//
	//dynamic_cast<Runner*>(m_objectsList[0])->SetBones(BonePush, BonePush1, BoneTrip, BoneTrip1);
	//dynamic_cast<Runner*>(m_objectsList[1])->SetBones(BonePush, BonePush1, BoneTrip, BoneTrip1);
#endif

	// Loading the visuals for the level
#if LEVEL_VISUALS
	vector<string> meshes = { "..\\NDEProject\\Assets\\OBJ Files\\Env_1v3_WIP.obj", "..\\NDEProject\\Assets\\OBJ Files\\Env2UV_low22.obj", "..\\NDEProject\\Assets\\OBJ Files\\Env3UV_low.obj", 
		"..\\NDEProject\\Assets\\OBJ Files\\Env4_lowUV.obj", "..\\NDEProject\\Assets\\OBJ Files\\Env5_UVlow.obj", "..\\NDEProject\\Assets\\OBJ Files\\Env6_UVLow.obj", 
		"..\\NDEProject\\Assets\\OBJ Files\\Env7_UVlow.obj", "..\\NDEProject\\Assets\\OBJ Files\\Env8_UVlow.obj", "..\\NDEProject\\Assets\\OBJ Files\\Env9UV_low.obj",
		"..\\NDEProject\\Assets\\OBJ Files\\Bridge_low.obj", "..\\NDEProject\\Assets\\OBJ Files\\Bridge2_low.obj", "..\\NDEProject\\Assets\\OBJ Files\\Bridge3_low.obj",
		"..\\NDEProject\\Assets\\OBJ Files\\Bridge4_low.obj", "..\\NDEProject\\Assets\\OBJ Files\\Factory.obj", "..\\NDEProject\\Assets\\OBJ Files\\FactoryFloorCeiling.obj",
		"..\\NDEProject\\Assets\\OBJ Files\\Silos_Naked.obj", "..\\NDEProject\\Assets\\OBJ Files\\Catwalk.obj",
		"..\\NDEProject\\Assets\\OBJ Files\\RunnerStartGate.obj", "..\\NDEProject\\Assets\\OBJ Files\\MonsterStartGate.obj",
		"..\\NDEProject\\Assets\\OBJ Files\\PlatformRocks.obj", "..\\NDEProject\\Assets\\OBJ Files\\BrickStone.obj", "..\\NDEProject\\Assets\\OBJ Files\\RockWallExit.obj" };

	vector<wchar_t*> difMaps = { L"..\\NDEProject\\Assets\\Textures\\Level\\Env_1v3_WIP_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env2UV_low_Diffuse.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env3UV_low__Albedo.dds", 
		L"..\\NDEProject\\Assets\\Textures\\Level\\Env4_lowUV_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env5_UVlow2_initialShadingGroup_Diffuse.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\ExtraPiece2_low_DefaultMaterial_Diffuse.dds", 
		L"..\\NDEProject\\Assets\\Textures\\Level\\ExportPiece3_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\ExtraPiece1_low_DefaultMaterial_Diffuse.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env9UV_low_Albedo.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\BridgeUV_low_DefaultMaterial_Diffuse.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Bridge2UV_low_DefaultMaterial_Diffuse.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Bridge3UV_low_DefaultMaterial_Diffuse.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\Bridge4_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Wall_SM_wall_AlbedoTransparency.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor2_AlbedoTransparency.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\Oil Silo_oil_silo_AlbedoTransparency.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor2_AlbedoTransparency.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor2_AlbedoTransparency.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Wall_SM_wall_AlbedoTransparency.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\RockPlatform1_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\StoneBrick_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\StoneBrick1_AlbedoBUTCHERED.dds" };

	vector<wchar_t*> normMaps = { L"..\\NDEProject\\Assets\\Textures\\Level\\Env_1v3_WIP_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env2UV_low2_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env3UV_low_Normal.dds", 
		L"..\\NDEProject\\Assets\\Textures\\Level\\Env4_lowUV_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env5_UVlow2_initialShadingGroup_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\ExtraPiece2_low_DefaultMaterial_Normal.dds", 
		L"..\\NDEProject\\Assets\\Textures\\Level\\ExportPiece3_low_DefaultMaterial_Normal.dds",  L"..\\NDEProject\\Assets\\Textures\\Level\\ExtraPiece1_low_DefaultMaterial_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Env9UV_low_initialShadingGroup_Normal.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\BridgeUV_low_DefaultMaterial_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Bridge2UV_low_DefaultMaterial_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\Bridge3UV_low_DefaultMaterial_Normal.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\Bridge4_low_DefaultMaterial_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Wall_SM_wall_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor_Normal.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\Oil Silo_oil_silo_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor_Normal.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Wall_SM_wall_Normal.dds",
		L"..\\NDEProject\\Assets\\Textures\\Level\\RockPlatform1_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\StoneBrick_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\StoneBrick_Normal.dds" };
	

	for (size_t i = 0; i < meshes.size(); i++)
	{
		BaseObject* level = new BaseObject(LEVEL_ID, 0.0f, false);
		if (meshes[i] == "..\\NDEProject\\Assets\\OBJ Files\\MonsterStartGate.obj")
			level->SetID(MONSTERSTART_ID);
		else if (meshes[i] == "..\\NDEProject\\Assets\\OBJ Files\\RunnerStartGate.obj")
			level->SetID(RUNNERSTART_ID);
		
		XMFLOAT3 levelPosition{ 0, 0, 0 };
		level->InstantiateModel(_device, meshes[i], levelPosition, 0, OBJECT);
		level->TextureObject(_device, difMaps[i], normMaps[i], L"..\\NDEProject\\Assets\\Textures\\NoEmission.dds");
		m_objectsList.push_back(level);
	}
#endif

	// Runners gate has to be third to last because of collision checks
	BaseObject* startGateRunner = new BaseObject(RUNNERSTART_ID, 0.0f, false);
	XMFLOAT3 sgRunPosition{ 0, 0, 0 };
	startGateRunner->InstantiateModel(_device, "..\\NDEProject\\Assets\\startGateRunner.obj", sgRunPosition, 0, OBJECT);
	startGateRunner->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor_AlbedoTransparency.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\Level\\SM_Floor_SM_Floor_SpecularSmoothness.dds");
	m_objectsList.push_back(startGateRunner);
	startGateRunner->TriInit();

	// Monsters gate has to be second to last because of collision checks
	BaseObject* startGateMonster = new BaseObject(MONSTERSTART_ID, 0.0f, false);
	XMFLOAT3 sgMonPosition{ 0, 0, 0 };
	startGateMonster->InstantiateModel(_device, "..\\NDEProject\\Assets\\startGateMonster.obj", sgMonPosition, 0, OBJECT);
	startGateMonster->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\SM_Floor_SM_Floor_AlbedoTransparency.dds", L"..\\NDEProject\\Assets\\Textures\\SM_Floor_SM_Floor_Normal.dds", L"..\\NDEProject\\Assets\\Textures\\SM_Wall_SM_wall_SpecularSmoothness.dds");
	m_objectsList.push_back(startGateMonster);
	startGateMonster->TriInit();
	
	// The level needs to be the last item in the list because of collision checks
	BaseObject* level = new BaseObject(LEVEL_ID, 0.0f, false);
	XMFLOAT3 levelPosition{ 0, 0, 0 };
	level->InstantiateModel(_device, "..\\NDEProject\\Assets\\level.obj", levelPosition, 0, OBJECT);
	level->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\StoneBrick1_AlbedoBUTCHERED.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	level->TriInit();
	m_objectsList.push_back(level);

	// Debugging fun test. Instancing stalagmites at random triangles
#if 0
	XMFLOAT4X4* mat = new XMFLOAT4X4[INSTANCES];
	
	for (unsigned int i = 0; i < INSTANCES; ++i)
	{
		XMStoreFloat4x4(&mat[i], XMMatrixIdentity());
		XMMATRIX newMat = XMMatrixIdentity();
		XMFLOAT3 pos = level->GetQuad()->m_tris[rand() % level->GetTris().size()]->m_center;
		XMVECTOR orien = XMLoadFloat3(&level->GetQuad()->m_tris[rand() % level->GetTris().size()]->m_normal);
		newMat.r[3].m128_f32[0] = pos.x;
		newMat.r[3].m128_f32[1] = pos.y;
		newMat.r[3].m128_f32[2] = pos.z;
	
		XMVECTOR angle = XMVector3AngleBetweenVectors(XMLoadFloat4x4(&mat[i]).r[2], orien);
		newMat = XMMatrixMultiply(XMMatrixRotationY(angle.m128_f32[0]), newMat);
	
		XMStoreFloat4x4(&mat[i], XMMatrixMultiply(newMat,XMLoadFloat4x4(&mat[i])));
	}
	m_objectsList[MAX_USERS + MAX_OBJECT + maxdoors]->InitializeInstances(mat, INSTANCES);
	delete mat;
#endif
}

void ObjManager::LoadWalls(CComPtr<ID3D11Device> _device)
{
	// Ground
	BaseObject *groundObj = new BaseObject(BASE_ID, 0.0f, false);
	XMFLOAT3 groundPosition{ 0, 0, 0 };
	groundObj->InstantiateModel(_device, "..\\NDEProject\\Assets\\ground.obj", groundPosition, 0, OBJECT);
	groundObj->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
	groundObj->AttachHitBox(AABB_COL);
	m_objectsList.push_back(groundObj);

	const int walls = 6;

	XMFLOAT3 xWallPositions[walls]
	{
	{ 30.0f, 0.0f, -75.0f },
	{ 0.0f, 0.0f, -55.0f },
	{ -45.0f,0.0f,-65.0f },
	{ 20.0f, 0.0f, 75.0f },
	{ 5.0f, 0.0f, 55.0f },
	{ 50.0f,0.0f, 60.0f }
	};

	XMFLOAT3 ZWallPositions[walls]
	{
	{ -45.0f, 0.0f, -25.0f },
	{ -35.0f, 0.0f, 25.0f },
	{ -15.0f,0.0f,0.0f },
	{ 45.0f, 0.0f, 25.0f },
	{ 35.0f, 0.0f, -25.0f },
	{ 30.0f, 0.0f, -75.0f }
	};

	// TODO:: REMOVE PARAMETER FOR POSITION
	BaseObject *wallObj;
	unsigned int i = 0;
	// X walls
	for (i = 0; i < walls; ++i)
	{
		wallObj = new BaseObject(BASE_ID, 0.0f, false);
		wallObj->InstantiateModel(_device, "..\\NDEProject\\Assets\\wall.obj", xWallPositions[i], 0, OBJECT);
		wallObj->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
		wallObj->AttachHitBox(AABB_COL);
		m_objectsList.push_back(wallObj);
	}

	// Z walls
	for (i = 0; i < walls; ++i)
	{
		wallObj = new BaseObject(BASE_ID, 0.0f, false);
		wallObj->InstantiateModel(_device, "..\\NDEProject\\Assets\\wall.obj", ZWallPositions[i], 0, OBJECT);
		wallObj->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\NORM_STONE_TEXTURE.dds", L"..\\NDEProject\\Assets\\Textures\\SPEC_STONE_TEXTURE.dds");
		wallObj->AttachHitBox(AABB_COL);
		// Rotating 90
		wallObj->SetWorldMatrix(XMMatrixMultiply(XMMatrixRotationY(1.5707f), wallObj->GetWorldMatrix()));
		m_objectsList.push_back(wallObj);
	}

}

void ObjManager::LoadItems(CComPtr<ID3D11Device> _device)
{
	// 5 = Key, 6-13 = boxes, 14-21 = items, then... doors?

	// Network ID
	int id = 0;

	// Load key item
	BaseItem* keyItem = new KeyItem(id);
	m_objectsList.push_back(keyItem);

	id++;

	//vector<string> SPs = { "..\\NDEProject\\Assets\\SpawnPoints\\SP1.obj", "..\\NDEProject\\Assets\\SpawnPoints\\SP2.obj", "..\\NDEProject\\Assets\\SpawnPoints\\SP3.obj", "..\\NDEProject\\Assets\\SpawnPoints\\SP4.obj",
	//	"..\\NDEProject\\Assets\\SpawnPoints\\SP5.obj", "..\\NDEProject\\Assets\\SpawnPoints\\SP6.obj", "..\\NDEProject\\Assets\\SpawnPoints\\SP7.obj", "..\\NDEProject\\Assets\\SpawnPoints\\SP8.obj" };
	
	for (unsigned int i = 0; i < 8; ++i)
	{
		BaseItem* item = new BaseItem(id);
		item->SetItemType(SPAWN_ITEM);
		item->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\Box.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 0, OBJECT);
		item->TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\DEF_SAND_TEXTURE.dds");
		item->SetIndex(m_objectsList.size());
		m_objectsList.push_back(item);
		id++;
	}
#if 0
	for (unsigned int i = 0; i < 8; ++i)
	{
		BaseItem* item = new PocketSand(id);
		m_objectsList.push_back(item);
		item->SetIndex(m_objectsList.size() - 1);
		item->SetIsAvailable(true);
		id++;
	}
#endif

#if 1
	for (unsigned int i = 0; i < 4; ++i)
	{
		BaseItem* item = new PocketSand(id);
		m_objectsList.push_back(item);
		item->SetIndex(m_objectsList.size() - 1);
		item->SetActive(false);
		id++;
	}

	for (unsigned int i = 0; i < 2; ++i)
	{
		BaseItem* item = new BlastGrenade(id);
		m_objectsList.push_back(item);
		item->SetIndex(m_objectsList.size() - 1);
		item->SetActive(false);
		id++;
	}
	
	for (unsigned int i = 0; i < 2; ++i)
	{
		BaseItem* item = new MagnetGranade(id);
		m_objectsList.push_back(item);
		item->SetIndex(m_objectsList.size() - 1);
		item->SetActive(false);
		id++;
		printf("%d\n", item->GetIndex());
	}
#endif

#if 0
	for (unsigned int i = 0; i < 8; ++i)
	{
		BaseItem* item = new MagnetGranade(id);
		m_objectsList.push_back(item);
		item->SetIndex(m_objectsList.size() - 1);
		item->SetIsAvailable(true);
		id++;
	}
#endif

	// Load doors
	const unsigned int maxdoors = 4;
	vector<string> doors = { "..\\NDEProject\\Assets\\Door1.obj", "..\\NDEProject\\Assets\\door2.obj", "..\\NDEProject\\Assets\\door3.obj", "..\\NDEProject\\Assets\\door4.obj" };
	XMFLOAT3 doorsPos[maxdoors] = { { XMFLOAT3(149.830f, 16.700f, -64.379f) },{ XMFLOAT3(47.600f, 93.300f, 16.400f) },{ XMFLOAT3(2.150f, 93.700f, -23.540f) },{ XMFLOAT3(28.555f, 74.349f, -172.004f) } };
	for (unsigned int i = 0; i < maxdoors; ++i)
	{
		// ID 777 means nothing
	   BaseItem* door = new BaseItem(777);
	   door->InstantiateModel(_device, doors[i], XMFLOAT3(0.0f, 0.0f, 0.0f), 1, OBJECT);
	   door->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\DEF_DOOR_TEXT.dds");
	  // door->AttachHitBox(15.0f, 5.2f, 0.5f, CAPSULE_COL);
	   XMMATRIX colPosition = XMMatrixIdentity();
	   colPosition.r[3] = XMLoadFloat3(&doorsPos[i]);
	   door->UpdateHitBox(&colPosition);
	   door->SetItemType(DOOR_ITEM);
	   m_objectsList.push_back(door);
	}

	 // Testing instancing
#if 0
	BaseObject* instanced = new BaseObject(LEVEL_ID, 0, false);
	XMFLOAT4X4* mat = new XMFLOAT4X4[INSTANCES];
	
	float x = -50.0f;
	float z = -10.0f;
	for (unsigned int i = 0; i < INSTANCES; ++i)
	{
		XMStoreFloat4x4(&mat[i], XMMatrixIdentity());
		mat[i]._41 = x;
		mat[i]._43 = z;
		x += 5.0f;
		if (x > 50.0f)
		{
			x = -50.0f;
			z += 5.0f;
		}
	
		float rndNum = float(rand() % 10);
		XMStoreFloat4x4(&mat[i], XMMatrixMultiply(XMMatrixScaling(rndNum, rndNum, rndNum), XMLoadFloat4x4(&mat[i])));
	}
	instanced->InitializeInstances(mat, INSTANCES);
	// CHANGE TO OBJECT to INSTANCED if you want to instance it
	instanced->InstantiateModel(_device, "..\\NDEProject\\Assets\\StalagmiteLow.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 1, INSTANCED);
	instanced->TextureObject(_device, L"..\\NDEProject\\Assets\\Textures\\Enviroment\\StalagimateLow_Albedo.dds", L"..\\NDEProject\\Assets\\Textures\\Enviroment\\StalagimateLow_Normal.dds");
	
	m_objectsList.push_back(instanced);
	delete mat;
#endif
}

void ObjManager::ClearObjects()
{
	for (unsigned int i = 0; i < m_objectsList.size(); ++i)
		delete m_objectsList[i];
	m_objectsList.clear();
}

void ObjManager::RefreshObjects()
{
	for (unsigned int i = 0; i < m_objectsList.size(); ++i)
	{
		OBJECT_ID id = m_objectsList[i]->GetID();
		if (id == PLAYER_ID || id == MONSTER_ID || id == RUNNER_ID)
		{
			Player *currPlayer = dynamic_cast<Player*>(m_objectsList[i]);
			currPlayer->SetIsStunned(false);
			currPlayer->SetActions(false);
			currPlayer->SetMousePos(960.0f);
			//currPlayer->SetClient(nullptr);
			currPlayer->SetAnimBusy(false);
			currPlayer->SetInAir(false);
			currPlayer->SetForce(XMFLOAT3(0.0f, 0.0f, 0.0f));
			currPlayer->SetActive(true);
			currPlayer->SetIsAlive(true);
			currPlayer->SetTargeted(false);
			if (id == RUNNER_ID)
			{
				Runner* currRunner = dynamic_cast<Runner*>(currPlayer);
				currRunner->SetItem(false);
				currRunner->SetAvailability(true);
				currPlayer->GetAnimations()[DEATH_ANIM]->SetIsDone(false);
			}
		}

		if (id == ITEM_ID)
		{
			BaseItem* currItem = dynamic_cast<BaseItem*>(m_objectsList[i]);
			if (currItem->GetItemType() != DOOR_ITEM)
			{
				currItem->SetIsCarried(false);
				currItem->SetParent(nullptr);
				currItem->SetDangerous(false);
				currItem->SetActive(true);
				currItem->SetInAir(true);
				currItem->SetTargeted(false);
				currItem->ResetExpireTime();
				currItem->SetRespawnTime(0.0f);
				if (currItem->GetItemType() == POCKET_SAND_ITEM || currItem->GetItemType() == MAGNET_GRENADE_ITEM || currItem->GetItemType() == BLAST_GRENADE_ITEM)
					currItem->SetActive(false);

				if (currItem->GetItemType() != KEY_ITEM)
					dynamic_cast<Capsule*>(currItem->GetHitBox())->m_Radius = 2.0f;
			}

		}
	}
}

void ObjManager::RefreshObject(unsigned int _index)
{
	OBJECT_ID id = m_objectsList[_index]->GetID();
	if (id == PLAYER_ID || id == MONSTER_ID || id == RUNNER_ID)
	{
		Player *currPlayer = dynamic_cast<Player*>(m_objectsList[_index]);
		currPlayer->SetIsStunned(false);
		currPlayer->SetActions(false);
		currPlayer->SetMousePos(960.0f);
		//currPlayer->SetClient(nullptr);
		currPlayer->SetAnimBusy(false);
		currPlayer->SetInAir(false);
		currPlayer->SetForce(XMFLOAT3(0.0f, 0.0f, 0.0f));
		currPlayer->SetActive(true);
		currPlayer->SetIsAlive(true);
		currPlayer->SetTargeted(false);
		
		if (id == RUNNER_ID)
		{
			Runner* currRunner = dynamic_cast<Runner*>(currPlayer);
			currRunner->SetItem(false);
			currRunner->SetAvailability(true);
			currPlayer->GetAnimations()[DEATH_ANIM]->SetIsDone(false);
		}
	}
	if (id == ITEM_ID)
	{
		BaseItem* currItem = dynamic_cast<BaseItem*>(m_objectsList[_index]);
		if (currItem->GetItemType() != DOOR_ITEM)
		{
			currItem->SetIsCarried(false);
			currItem->SetParent(nullptr);
			currItem->SetDangerous(false);
			currItem->SetActive(true);
			currItem->SetInAir(true);
			currItem->SetTargeted(false);
			currItem->ResetExpireTime();
			if (currItem->GetItemType() == POCKET_SAND_ITEM || currItem->GetItemType() == MAGNET_GRENADE_ITEM || currItem->GetItemType() == BLAST_GRENADE_ITEM)
				currItem->SetActive(false);

			if (currItem->GetItemType() != KEY_ITEM)
				dynamic_cast<Capsule*>(currItem->GetHitBox())->m_Radius = 2.0f;
		}
	}
	if (id == RUNNERSTART_ID || id == MONSTERSTART_ID)
		m_objectsList[_index]->SetWorldMatrix(XMMatrixIdentity());
}

BaseItem* ObjManager::GetAvailableItem(BaseItem* _item)
{
	// 5 = Key, 6-13 = boxes, 14-21 = items, then... doors?

	vector<BaseItem*> items;
	for (size_t i = 14; i < 22; i++)
	{
		BaseItem* newItem = dynamic_cast<BaseItem*>(m_objectsList[i]);
		if (!newItem->GetActive())
			items.push_back(newItem);
	}

	if (items.size() == 0)
		return nullptr;

	srand((unsigned int)time(NULL));
	int num = rand() % items.size();

	return items[num];
}

void ObjManager::RemoveObject(BaseItem* _item)
{
	_item->SetParent(nullptr);
	_item->SetIsCarried(false);
	_item->SetInAir(true);
	_item->SetToServerID(100);
	_item->GetHitBox()->m_active = false;

	//for (size_t i = 0; i < m_itemPool.size(); i++)
	//	if (!m_itemPool[i])
	//		m_itemPool[i] = _item;
}

bool ObjManager::CheckForItems()
{
	for (size_t i = 14; i < 22; i++)
	{
		BaseItem* newItem = dynamic_cast<BaseItem*>(m_objectsList[i]);
		if (!newItem->GetActive())
			return true;
	}
	return false;
}
