#include "Blender.h"
#include "Animation.h"

// Statics
vector<Blender*> Blender::m_blenders;


Blender::Blender(int ind)
{
	m_num = ind;
	m_active = false;
	m_toAnim = nullptr;
	m_fromAnim = nullptr;
	m_oldBlend = nullptr;
}

Blender::Blender(Animation** _fromAnim, Animation** _toAnim)
{
	m_toAnim = (*_toAnim);
	m_fromAnim = (*_fromAnim);
	m_oldBlend = NULL;
}

Blender::Blender(Blender** _old, Animation** _toAnim)
{
	m_toAnim = (*_toAnim);
	m_fromAnim = NULL;
	m_oldBlend = (*_old);
}

Blender::~Blender()
{
}

KeyFrame& Blender::Blend(KeyFrame& finalFrame,float _processTime, float _depth, float deltaTime, bool& _end)
{
	KeyFrame currAniFrame;
	bool end = false;

	if (m_oldBlend && m_oldBlend->m_active)
		currAniFrame = m_oldBlend->Blend(finalFrame,_processTime, _depth * 1.5f, deltaTime, end);
	else
		m_fromAnim->Process(_processTime, currAniFrame, end);

	m_duration -= deltaTime * _depth;



	float tweenTime = m_time - m_duration;
	float lambda = (tweenTime / m_time);

	KeyFrame nextAniFrame = *m_toAnim->m_keyFrame[m_toAnim->m_keyFrame.size() - 1];
	m_toAnim->Process(_processTime, nextAniFrame, _end);

	m_toAnim->Interpolate(&currAniFrame, &nextAniFrame, lambda, finalFrame);
	
	if (m_oldBlend && m_oldBlend->m_active && m_oldBlend->m_duration <= 0)
	{
		m_fromAnim = m_oldBlend->m_toAnim;
		m_oldBlend->Refresh();
		m_oldBlend = nullptr;
	}

	return finalFrame;
}

void Blender::SetBlender(Animation** _fromAnim, Animation** _toAnim, Animation** _falling)
{
	m_active = true;
	m_duration = (*_toAnim)->GetInToBlendTime();
	m_time = (*_toAnim)->GetInToBlendTime();
	m_toAnim = (*_toAnim);
	m_fromAnim = (*_fromAnim);
	if (m_oldBlend)
		m_oldBlend->m_active = false;
	m_falling = (*_falling);
}

void Blender::SetBlender(Blender** _old, Animation** _toAnim, Animation** _falling)
{
	m_active = true;
	m_duration = (*_toAnim)->GetInToBlendTime();
	m_time = (*_toAnim)->GetInToBlendTime();
	m_toAnim = (*_toAnim);
	m_oldBlend = (*_old);
	m_falling = (*_falling);
}
void Blender::Refresh()
{
	m_active = false;
	m_duration = 0;
	m_toAnim = nullptr;
	m_fromAnim = nullptr;

	if (m_oldBlend)
		m_oldBlend->m_active = false;

	m_oldBlend = nullptr;
}

Blender* Blender::GetFreeBlender()
{
	for (unsigned i = 0; i < m_blenders.size(); ++i)
		if (m_blenders[i]->m_active == false)			
			return m_blenders[i];

	return nullptr;
}
void Blender::InitBlenders()
{
	if (m_blenders.size() == 0)
	{
		// Presizing the blenders
		m_blenders.resize(40);

		for (unsigned i = 0; i < m_blenders.size(); ++i)
			m_blenders[i] = new Blender(i);
	}
}

void Blender::ClearBlenders()
{
	for (unsigned int i = 0; i < m_blenders.size(); ++i)
		delete m_blenders[i];
	m_blenders.clear();
}

unsigned int Blender::GetNumFreeBlenders()
{
	unsigned int num = 0;

	for (unsigned i = 0; i < m_blenders.size(); ++i)
		if (m_blenders[i]->m_active == false)
			num++;

	return num;
}