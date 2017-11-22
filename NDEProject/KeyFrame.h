#pragma once
#include "Transform.h"
#include <vector>

class KeyFrame
{
public:
	std::vector<Transform> m_bones;
	float m_time;
	std::vector<unsigned int> m_boneIndex;
	unsigned int m_frameNum;
	KeyFrame* m_next, *m_prev;
	// Setters
};
