#pragma once
#ifndef Quad_H
#define Quad_H
#include "includes.h"

enum QUADRANT { BOTTOM_LEFT_QUAD, BOTTOM_RIGHT_QUAD, UP_LEFT_QUAD, UP_RIGHT_QUAD };
struct Quad
{
	vector<Quad*> d_list;
	Quad* d_base;
	string m_name;
	vector<Tri*> m_tris;
	AABB m_ALL, m_UL, m_UR, m_BL, m_BR;
	vector<Quad*> m_quadrants;

	void Divide(AABB _base);
	void Distribute();
	void SafetyCheck(Tri* _tri);
};
#endif