#include "Animation.h"
#include "BaseObject.h"

Animation::Animation()
{
	m_currentFrame = NULL;
	currentTime = 0;
	m_reverse = false;
	m_animDone = false;
}

Animation::~Animation()
{
	// WE CRASH HERE A LOT
	for (size_t i = 0; i < m_keyFrame.size(); i++)
		delete m_keyFrame[i];

	m_keyFrame.clear();
}

void Animation::Process(float time, KeyFrame& k, bool& end)
{
	if (m_currentFrame == m_keyFrame[m_keyFrame.size() - m_outOfBlend] && m_death)
	{
		if (k.m_bones.size() > 0)
			k.m_bones.clear();

		m_animDone = true;
			return;
	}

	if (m_currentFrame == NULL && !m_reverse)
		m_currentFrame = m_keyFrame[1];
	else if (m_currentFrame == NULL && m_reverse)
		m_currentFrame = m_keyFrame[m_keyFrame.size() - 1];

	// Get new time. This time is the delta from the main update loop which 
	// is being passed in the function "AddTime" is just adding that time to 
	// the currentTime data member of the interpolator class.
	if (!m_reverse)
		AddTime(time);
	else
		SubtractTime(time);

	float other;
	float finalTime;
	float cFrameTime;
	float nFrameTime;
	if (!m_reverse)
	{
		// The following lines are constructing a new time slot to interpolate
		// between the last and first frames of an animation.
		// "other" is grabbing the second to last frame's key time
		other = m_keyFrame[m_keyFrame.size() - 2]->m_time;
		// "finalTime" is grabbing the last frame's key time
		finalTime = m_keyFrame[m_keyFrame.size() - 1]->m_time;
		// "dif" is getting the difference between those two aforementioned
		// key times in order to get an idea of long we should be interpolating
		// bewteen the last and first frames
		float dif = finalTime - other;
		// I set finalTime to dif because it was a better name for what I wanted.
		finalTime = dif;
		// I then add the difference between the two frames to the last frame time
		// to get the new interpolation time for the last and first frame.
		// EX. the second to last frame time: 1.1, and the last 1.2. The new frame
		// time, finalTime, would then be 1.3, since the previous difference was .1.
		finalTime += m_keyFrame[m_keyFrame.size() - 1]->m_time;
		
		// I grab the current time and next times to interpolate between.
		cFrameTime = m_currentFrame->m_time;
		nFrameTime = m_currentFrame->m_next->m_time;
	}
	else
	{
		finalTime = m_keyFrame[0]->m_time;
		cFrameTime = m_currentFrame->m_time;
		nFrameTime = m_currentFrame->m_prev->m_time;
	}


	// If the currentTime is greater than the finalTime, the extra time slot we created,
	// then we reset everything back to the beginning.
	if (currentTime > finalTime && !m_reverse)
	{
		m_currentFrame = m_keyFrame[1];
		currentTime = m_currentFrame->m_time;
		cFrameTime = m_currentFrame->m_time;
		nFrameTime = m_currentFrame->m_next->m_time;
	}
	else if (currentTime < finalTime && m_reverse)
	{
		m_currentFrame = m_keyFrame[m_keyFrame.size() - 1];
		currentTime = m_currentFrame->m_time;
		cFrameTime = m_currentFrame->m_time;
		nFrameTime = m_currentFrame->m_prev->m_time;
	}


	// This check is to see if we are incrementing the frame. It's in a while loop
	// because depending on the delta being passed in we might need to increment
	// multiple times
	if (!m_reverse)
		while (currentTime > nFrameTime)
		{
			m_currentFrame = m_currentFrame->m_next;
			cFrameTime = nFrameTime;

			// This check is seeing if we are on the final frame. If we are, then we
			// set the next time to the finalTime we made earlier so we can interpolate
			// between the first and last frames. Otherwise just set it to the next
			// frames keytime.
			if (m_currentFrame->m_frameNum == m_keyFrame[m_keyFrame.size() - 1]->m_frameNum)
				nFrameTime = finalTime;
			else
				nFrameTime = m_currentFrame->m_next->m_time;
		}
	else
		while (currentTime < nFrameTime)
		{
			m_currentFrame = m_currentFrame->m_prev;
			cFrameTime = nFrameTime;

			if (m_currentFrame->m_frameNum == 1)
				nFrameTime = finalTime;
			else
				nFrameTime = m_currentFrame->m_prev->m_time;
		}

	// These variables are for finding the lambda bewteen the current and next frame.
	// I named it delta in this function, but it is ratio between the current and
	// next frame.
	float frameTime = nFrameTime - cFrameTime;
	float tweenTime = nFrameTime - currentTime;
	float t = (tweenTime / frameTime);
	float delta = 1 - t;

	if (m_currentFrame == m_keyFrame[m_keyFrame.size() - m_outOfBlend] && !m_loop)
		end = true;

	if (!m_reverse)
		Interpolate(m_currentFrame, m_currentFrame->m_next, delta, k);
	else
		Interpolate(m_currentFrame, m_currentFrame->m_prev, delta, k);
}

void Animation::Interpolate(KeyFrame* current, KeyFrame* next, float delta, KeyFrame& k)
{
	// These functions are from the animation slides on the google drive.
	// They are constructing a new matrix based on the delta we are sending in.
	k.m_bones.clear();
	for (size_t i = 0; i < current->m_bones.size(); i++)
	{
		XMVECTOR scaleCur, scaleNext, rotationCur, rotationNext, positionCur, positionNext;

		XMMatrixDecompose(&scaleCur, &rotationCur, &positionCur, current->m_bones[i].m_worldMatrix);
		XMMatrixDecompose(&scaleNext, &rotationNext, &positionNext, next->m_bones[i].m_worldMatrix);
		XMVECTOR rotNow = XMQuaternionSlerp(rotationCur, rotationNext, delta);
		XMVECTOR scaleNow = XMVectorLerp(scaleCur, scaleNext, delta);
		XMVECTOR posNow = XMVectorLerp(positionCur, positionNext, delta);
		XMMATRIX MatrixNow = XMMatrixAffineTransformation(scaleNow, XMVectorZero(), rotNow, posNow);
		Transform bone;
		bone.m_worldMatrix = MatrixNow;
		k.m_bones.push_back(bone);
	}

	int x = 0;
	//int x = 0;
	//for (size_t i = 0; i < current->m_bones.size(); i++)
	//{
	//	if (next->m_bones[i].m_worldMatrix.r[3].m128_f32[0] == current->m_bones[i].m_worldMatrix.r[3].m128_f32[0])
	//		if (next->m_bones[i].m_worldMatrix.r[3].m128_f32[1] == current->m_bones[i].m_worldMatrix.r[3].m128_f32[1])
	//			if (next->m_bones[i].m_worldMatrix.r[3].m128_f32[2] == current->m_bones[i].m_worldMatrix.r[3].m128_f32[2])
	//				if (next->m_bones[i].m_worldMatrix.r[2].m128_f32[0] == current->m_bones[i].m_worldMatrix.r[2].m128_f32[0])
	//					if (next->m_bones[i].m_worldMatrix.r[2].m128_f32[1] == current->m_bones[i].m_worldMatrix.r[2].m128_f32[1])
	//						if (next->m_bones[i].m_worldMatrix.r[2].m128_f32[2] == current->m_bones[i].m_worldMatrix.r[2].m128_f32[2])
	//							if (next->m_bones[i].m_worldMatrix.r[1].m128_f32[0] == current->m_bones[i].m_worldMatrix.r[1].m128_f32[0])
	//								if (next->m_bones[i].m_worldMatrix.r[1].m128_f32[1] == current->m_bones[i].m_worldMatrix.r[1].m128_f32[1])
	//									if (next->m_bones[i].m_worldMatrix.r[1].m128_f32[2] == current->m_bones[i].m_worldMatrix.r[1].m128_f32[2])
	//										if (next->m_bones[i].m_worldMatrix.r[0].m128_f32[0] == current->m_bones[i].m_worldMatrix.r[0].m128_f32[0])
	//											if (next->m_bones[i].m_worldMatrix.r[0].m128_f32[1] == current->m_bones[i].m_worldMatrix.r[0].m128_f32[1])
	//												if (next->m_bones[i].m_worldMatrix.r[0].m128_f32[2] == current->m_bones[i].m_worldMatrix.r[0].m128_f32[2])
	//													x += 1;
	//}
	//if (x == 28)
	//	x = 0;
}

void Animation::Update(DirectX::XMFLOAT4X4* _toVram, std::vector<Transform>& _bindPose, std::vector<BaseObject*>& _renderBones, KeyFrame& _keyFrame)
{
	for (size_t i = 0; i < _keyFrame.m_bones.size(); i++)
	{
		//Data that goes to the shader
		XMMATRIX bindPose = _bindPose[i].m_worldMatrix;

		XMMATRIX inverseBindpose = XMMatrixInverse(0, bindPose); // Getting inverse of bind pose
		XMMATRIX currBoneAtFrame = _keyFrame.m_bones[i].m_worldMatrix; // Getting current frame

	  	XMStoreFloat4x4(&_toVram[i], XMMatrixMultiply(inverseBindpose, currBoneAtFrame)); // Sends the data to the shader

		//_renderBones[i]->SetWorldMatrix(currBoneAtFrame);
	}
}

KeyFrame Animation::Blend()
{
	KeyFrame k;
	return k;
}