#pragma once
#ifndef Camera_H
#define Camera_H
#include "includes.h"
class BaseObject;
enum CAMERA_TYPE {FP_CAM,TP_CAM,RTP_CAM,MAX_CAM};

__declspec(align(16)) class Camera
{
	POINT                 m_oldMousePos;
	SCENE_TO_VRAM         m_viewToShader;
	CAMERA_TYPE           m_type;
	BaseObject            *m_parentObj;
	CComPtr<ID3D11Buffer> m_constBuffer;
	float                 m_rotation;
	XMFLOAT3              m_cameraOffset;
	float                 m_oldDist;
	Frustum               m_frustum;

public:
	Camera();
	Camera(CAMERA_TYPE _type);

	~Camera();

	void Initialize(CAMERA_TYPE _type);
	void Input();
	void Render();
	void SetParentObject(BaseObject* _parent) { m_parentObj = _parent; }
	SCENE_TO_VRAM GetViewMatrix() { return m_viewToShader; }
	void ResizeCameraProjection();
	XMFLOAT3 GetCameraOffset() { return m_cameraOffset; }
	Frustum& GetFrustum() { return m_frustum; }
	bool FrustumToCapsule(const Frustum& frustum, const Capsule& capsule);
	int ClassifyCapsuleToPlane(const Plane& plane, const Capsule& capsule);
	int ClassifySphereToPlane(const Plane& plane, const Sphere& sphere);
	

private:
	void MapCamera();
	void CreateConstantBuffer();
	void FPCamera(float _speed);
	void TPCamera();
	void RTPCamera();
	void SetCameraOffset(XMFLOAT3 _offset) { m_cameraOffset.x = _offset.x;  m_cameraOffset.y = _offset.y; m_cameraOffset.z = _offset.z;  }
	void WithinBounds();
	void BuildFrustum(Frustum& frustum, float fov, float nearDist, float farDist, float ratio, const XMFLOAT4X4& camXform);
public:
	// Alligning by 16 bytes so we don't get a warning 
	void* operator new(size_t i){return _mm_malloc(i,16);}
	void operator delete(void* p){_mm_free(p);}
};
#endif
