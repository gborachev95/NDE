#pragma once
#ifndef Includes_H
#define Includes_H
//#include "FBXInporter.h"

// Standart Includes
#include <vector>
#include <DirectXMath.h>
#include <d3d11.h> 
// For smart pointers
#include <atlbase.h>
// Application includes
#include "..\FBXInporter\ExporterHeader.h"

enum COL_TYPE { AABB_COL, SPHERE_COL, CAPSULE_COL, PUSH_COL, COL_MAX };

// Defines
#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH  1200

const float SCREEN_ZFAR = 300.0f;
const float SCREEN_ZNEAR = 0.01f;

// Namespaces
using namespace DirectX;
using namespace std;

// Loading libraries
#pragma comment (lib, "d3d11.lib") 

// Enums

enum OBJECT_ID {BASE_ID, PLAYER_ID, RUNNER_ID, MONSTER_ID,ITEM_ID,LEVEL_ID, DEBUG_ID, RUNNERSTART_ID, MONSTERSTART_ID};

// Structures
struct VERTEX
{
	XMFLOAT4 transform;
	XMFLOAT4 normals;
	XMFLOAT4 uv;
	XMFLOAT4 tangents;
	XMFLOAT4 bitangents;
	XMFLOAT4 skinIndices;
	XMFLOAT4 skinWeights;
};

struct COLOR
{
	union 
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		float color[4];
	};

	float* GetColor()
	{
		return color;
	}
	void SetColor(float _r, float _g, float _b, float _a)
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
};

struct SM_VERTEX
{
	XMFLOAT4 position;
	XMFLOAT4 color;
	XMFLOAT4 uv;
};

struct P_VERTEX
{
	XMFLOAT4 side;
	XMFLOAT4 up;
	XMFLOAT4 forward;
	XMFLOAT4 position;
	XMFLOAT4 color;
	XMFLOAT4 uv;
};

struct POST_DATA_TO_VRAM
{
	float data[4];
};

struct OBJECT_TO_VRAM
{
	XMFLOAT4X4 worldMatrix;
};

struct SCENE_TO_VRAM
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4 cameraPosition;
};

struct BONES_TO_VRAM
{
	XMFLOAT4X4 bones[59];
	XMFLOAT4X4 positionOffset;
};

struct LIGHT_TO_VRAM
{
	XMFLOAT4 transform;
	XMFLOAT3 direction;
	float    radius;
	COLOR    color;
};

struct PARTICLE_TO_VRAM
{
	XMFLOAT4 position;
	XMFLOAT4 color;
    float    velocity;
	bool     active;
};

struct MOD_TO_VRAM
{
	XMFLOAT4 color;
	XMFLOAT4 uvTranslation;
};

class ParticleOBJ
{
public:
	PARTICLE_TO_VRAM pToV;
	XMFLOAT3 m_trajectory;
	float m_age;
	XMFLOAT4X4 world;
	XMFLOAT4X4 toShader;
};

class HitBox
{
public:
	COL_TYPE type;
	bool m_active;
	HitBox() {};
	virtual ~HitBox() {};
};

class Segment
{
public:
	XMFLOAT3 m_Start;
	XMFLOAT3 m_End;
};

class Sphere : public HitBox
{
public:
	XMFLOAT3 m_Center;
	float m_Radius;
};

class Capsule : public HitBox
{
public:
	Segment m_Segment;
	float m_Radius;

	void SetHitbox(Capsule* _hB)
	{
		m_Radius = _hB->m_Radius;
		m_Segment.m_Start.x = _hB->m_Segment.m_Start.x;
		m_Segment.m_Start.y = _hB->m_Segment.m_Start.y;
		m_Segment.m_Start.z = _hB->m_Segment.m_Start.z;
		m_Segment.m_End.x = _hB->m_Segment.m_End.x;
		m_Segment.m_End.y = _hB->m_Segment.m_End.y;
		m_Segment.m_End.z = _hB->m_Segment.m_End.z;
	}
};

class AABB : public HitBox
{
public:
	XMFLOAT3 m_min;
	XMFLOAT3 m_max;
};

struct Plane
{
	XMFLOAT3 m_normal;
	float m_offset;
};

struct Tri
{
	string d_name;
	vector<string> d_path;
	XMFLOAT3 m_center;
	XMFLOAT3 m_normal;
	VERTEX m_one, m_two, m_three;
};

struct Frustum
{
	Plane planes[6];
	XMFLOAT3 corners[8];
};

struct BUFFER_STRUCT
{
    int index;
	float color[4];
};

#endif
