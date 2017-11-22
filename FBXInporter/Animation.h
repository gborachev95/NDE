#pragma once
#ifndef Animation_H
#define Animation_H

#include "KeyFrame.h"
#include <vector>

class BaseObject;
enum ANIM_TYPE
{
	IDLE_ANIM = 0, WALK_ANIM = 1, WALK_BACKWARD_ANIM = 2, RUN_ANIM = 3, JUMP_ANIM = 4, FALLING_ANIM = 5, LANDING_ANIM = 6, STRAFE_LEFT_ANIM = 7, STRAFE_RIGHT_ANIM = 8, // for everyone
	DEATH_ANIM = 9, PUSH_ANIM = 10, PUSHED_ANIM = 11, KICK_ANIM = 12, TRIPPING_ANIM = 13, GETTING_UP_ANIM = 14, // for runner
	POUNCE_CHARGE_ANIM = 9, POUNCE_START_ANIM = 10, POUNCE_IDLE_ANIM = 11, POUNCE_LANDING_ANIM = 12, CRAWL_FORWARD_ANIM = 13, CRAWL_BACKWARD_ANIM = 14, CRAWL_LEFT_ANIM = 15, CRAWL_RIGHT_ANIM = 16, // for monster
	SWIPE_ANIM = 17,
};
const float AnimationTime = 0.001f;

class Animation
{
public:
	std::vector<KeyFrame*> m_keyFrame;
	Animation();
	~Animation();

	void Interpolate(KeyFrame* current, KeyFrame* next, float delta, KeyFrame& k);
	void Process(float time, KeyFrame& k, bool& end);
	KeyFrame Blend();
	inline void AddTime(float timeToAdd) { currentTime += timeToAdd; }
	inline void SubtractTime(float timeToAdd) { currentTime -= timeToAdd; }
	inline void SetTime(float _currentTime) { currentTime = _currentTime; }
	inline float GetTime() { return currentTime; }
	inline ANIM_TYPE GetName() { return m_name; }
	inline float GetInToBlendTime() { return m_inToBlend; }
	inline void SetInToBlendTime(float _ble) { m_inToBlend = _ble; }
	inline unsigned int GetOutOfBlendFrame() { return m_outOfBlend; }
	inline void SetOutOfBlendFrame(unsigned int _ble) { m_outOfBlend = _ble; }
	inline bool GetReverse() { return m_reverse; }
	inline void SetReverse(bool _rev) { m_reverse = _rev; }

	static void Update(DirectX::XMMATRIX* _toVram, std::vector<Transform>& _bindPose, std::vector<BaseObject*>& _renderBones, KeyFrame& _keyFrame);

	// Setters
	void SetTotalTime(float _totalTime)
	{
		m_totalTime = _totalTime;
	}

	void SetFrame(unsigned int _kf) { m_currentFrame = m_keyFrame[_kf]; }

	// Getters
	float GetTotalTime()
	{
		return m_totalTime;
	}

	bool GetLooping()
	{
		return m_loop;
	}

	void SetLooping(bool _inLoop)
	{
		m_loop = _inLoop;
	}

	ANIM_TYPE GetAnimType()
	{
		return m_name;
	}

	void SetName(ANIM_TYPE _s)
	{
		m_name = _s;
	}

	void SetIsDone(bool _done)
	{
		m_animDone = _done;
	}

	bool GetIsDone()
	{
		return m_animDone;
	}

	KeyFrame* GetCurrFrame()
	{
		return m_currentFrame;
	}

	void SetDeath(bool _death)
	{
		m_death = _death;
	}

	bool GetDeath()
	{
		return m_death;
	}

private:
	float m_totalTime;
	bool m_loop;
	int m_frame;
	KeyFrame * m_currentFrame;
	float currentTime;
	ANIM_TYPE m_name;
	// m_inToBlend is the duration of the blend when you first enter
	float m_inToBlend;
	// m_outOfBlend is the frame in which you start blending to the next animation
	unsigned int m_outOfBlend;
	bool m_reverse;
	bool m_animDone;
	bool m_death;
};

#endif