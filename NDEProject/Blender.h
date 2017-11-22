#pragma once
#ifndef Blender_H
#define Blender_H
#include <vector>
#include <DirectXMath.h>
#include "Transform.h"
#include "BaseObject.h"

class KeyFrame;
class Animation;
class Blender
{
	int m_num;
	bool m_active;
	float m_duration;
	float m_time;
	Animation *m_toAnim, *m_fromAnim, *m_falling;
	Blender *m_oldBlend;
	static vector<Blender*> m_blenders;	
public:
	Blender(int m_num);
	Blender(Animation** _fromAnim, Animation** _toAnim);
	Blender(Blender** _old, Animation** _toAnim);
	~Blender();

	float GetBlendDur() { return m_duration; }
	Animation* GetToAnim() { return m_toAnim; }
	void SetToAnim(Animation* _toAnim) { m_toAnim = _toAnim; }
	Animation* GetFromAnim() { return m_fromAnim; }
	void SetFromAnim(Animation* _fromAnim) { m_fromAnim = _fromAnim; }
	bool GetActive() { return m_active; }
	void Refresh();
	void SetBlender(Animation** _fromAnim, Animation** _toAnim, Animation** _falling);
	void SetBlender(Blender** _old, Animation** _toAnim, Animation** _falling);
	static Blender* GetFreeBlender();
	static void InitBlenders();
	static Blender* GetStartBlend(int _ind) { return m_blenders[_ind]; }
	int GetNum() { return m_num; }
	static void ClearBlenders();
	Blender* GetOldBlend() { return m_oldBlend; }
	void SetOldBlend(Blender* _blend) { m_oldBlend = _blend; }
	unsigned int GetNumFreeBlenders();

	KeyFrame& Blend(KeyFrame& finalFrame, float _processTime, float _depth, float deltaTime, bool& _end);
};
#endif

