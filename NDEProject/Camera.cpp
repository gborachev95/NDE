#include "Camera.h"
#include "BaseObject.h"
#include "Graphics.h"
#include "Monster.h"
#include "ObjManager.h"
#include "Collision.h"

enum FrustumCorners { FTL = 0, FBL, FBR, FTR, NTL, NTR, NBR, NBL };
enum FrustumPlanes { NEAR_PLANE = 0, FAR_PLANE, LEFT_PLANE, RIGHT_PLANE, TOP_PLANE, BOTTOM_PLANE };

Camera::Camera()
{
}

Camera::Camera(CAMERA_TYPE _type)
{
	Initialize(_type);
}

Camera::~Camera()
{
}

void Camera::Initialize(CAMERA_TYPE _type)
{
	GetCursorPos(&m_oldMousePos);
	m_type = _type;
	CreateConstantBuffer();
	float fov = 45;
	float ratio = WINDOW_WIDTH  / WINDOW_HEIGHT ;

	// Setting the projection matrix
	XMStoreFloat4x4(&m_viewToShader.projectionMatrix, XMMatrixPerspectiveFovLH(fov, ratio, SCREEN_ZNEAR, SCREEN_ZFAR));

	// Setting the View matrix
	XMStoreFloat4x4(&m_viewToShader.viewMatrix, XMMatrixIdentity());
	m_viewToShader.viewMatrix._42 = 5.0f;
	m_viewToShader.viewMatrix._43 = 10.0f;

	// Setting Camera position 
	m_viewToShader.cameraPosition.x = m_viewToShader.viewMatrix._41;
	m_viewToShader.cameraPosition.y = m_viewToShader.viewMatrix._42;
	m_viewToShader.cameraPosition.z = m_viewToShader.viewMatrix._43;

	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewToShader.viewMatrix);

	XMMATRIX oneEightyRotMatrix = XMMatrixMultiply(XMMatrixRotationY(3.141f), viewMatrix);
	viewMatrix = oneEightyRotMatrix;

	// Inversing the camera
	XMStoreFloat4x4(&m_viewToShader.viewMatrix,XMMatrixInverse(nullptr, viewMatrix));

	m_parentObj = nullptr;
	m_rotation = 5853.0f;

	m_cameraOffset.x = 0.0f;
	m_cameraOffset.y = 10.0f;
	m_cameraOffset.z = 6.0f;
}

void Camera::Input()
{
	switch (m_type)
	{
	case FP_CAM:
	{
		FPCamera(float(Graphics::GetTime().Delta()) * 10.0f);
		break;
	}
	case TP_CAM:
	{
		if (m_parentObj)
			TPCamera();
		break;
	}
	case RTP_CAM:
	{
		if (m_parentObj)
			RTPCamera();
	}

	default:
	{
		break;
	}
	}
}

void Camera::Render()
{
	MapCamera();
}

void Camera::MapCamera()
{
	if (Graphics::GetIsResized())
		ResizeCameraProjection();
	// Set constant buffers
	Graphics::GetDeviceContext()->VSSetConstantBuffers(1, 1, &m_constBuffer.p);
	Graphics::GetDeviceContext()->PSSetConstantBuffers(5, 1, &m_constBuffer.p);

	D3D11_MAPPED_SUBRESOURCE mapSubresource;
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	Graphics::GetDeviceContext()->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_viewToShader, sizeof(SCENE_TO_VRAM));
	Graphics::GetDeviceContext()->Unmap(m_constBuffer, NULL);

}

void Camera::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(SCENE_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Graphics::GetDevice()->CreateBuffer(&constBufferDesc, NULL, &m_constBuffer.p);
}

void Camera::FPCamera(float _speed)
{
	// Get the cursor position
	POINT mousePos;
	ZeroMemory(&mousePos, sizeof(POINT));
	GetCursorPos(&mousePos);

	// Calculating the difference of the mouse position
	SetCursorPos(960, 500);

	// Calculating the difference of the mouse position
	float deltaX = float(960 - mousePos.x) * 0.1f;;// float(m_oldMousePos.x - mousePos.x);
	float deltaY = float(500 - mousePos.y) * 0.1f;// float(m_oldMousePos.y - mousePos.y);

	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewToShader.viewMatrix);
	// Rotating when holding left key
	//if (Graphics::single_mouse->GetState().leftButton)
	//{
		viewMatrix = XMMatrixInverse(0, viewMatrix);
		XMVECTOR storePosition;
		storePosition.m128_f32[0] = viewMatrix.r[3].m128_f32[0];
		storePosition.m128_f32[1] = viewMatrix.r[3].m128_f32[1];
		storePosition.m128_f32[2] = viewMatrix.r[3].m128_f32[2];

		viewMatrix.r[3].m128_f32[0] = 0;
		viewMatrix.r[3].m128_f32[1] = 0;
		viewMatrix.r[3].m128_f32[2] = 0;

		viewMatrix = XMMatrixMultiply(XMMatrixRotationX(-deltaY*0.005f),viewMatrix);
		viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixRotationY(-deltaX*0.005f));

		viewMatrix.r[3].m128_f32[0] = storePosition.m128_f32[0];
		viewMatrix.r[3].m128_f32[1] = storePosition.m128_f32[1];
		viewMatrix.r[3].m128_f32[2] = storePosition.m128_f32[2];

		viewMatrix = XMMatrixInverse(0, viewMatrix);
	//}

	m_oldMousePos = mousePos;


	auto kb = Graphics::single_keyboard->GetState();
	if (kb.LeftShift)
	{
		_speed *= 4;
	}

	if (kb.W)
		viewMatrix.r[3].m128_f32[2] -= _speed;
	else if (kb.S)
		viewMatrix.r[3].m128_f32[2] += _speed;
	if (kb.D)
		viewMatrix.r[3].m128_f32[0] -= _speed;
	else if (kb.A)
		viewMatrix.r[3].m128_f32[0] += _speed;

	if (kb.Q)
		viewMatrix.r[3].m128_f32[1] += _speed;
	else if (kb.E)
		viewMatrix.r[3].m128_f32[1] -= _speed;

	XMStoreFloat4x4(&m_viewToShader.viewMatrix,viewMatrix);
}

void Camera::TPCamera()
{
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewToShader.viewMatrix);
	viewMatrix = XMMatrixInverse(0, m_parentObj->GetWorldMatrix());
	viewMatrix.r[3].m128_f32[1] = viewMatrix.r[3].m128_f32[1] - 10.0f;
	viewMatrix.r[3].m128_f32[2] = viewMatrix.r[3].m128_f32[2] - 6.0f;

	// Changing the angle
	viewMatrix = XMMatrixInverse(0, viewMatrix);
	XMMATRIX oneEightyRotMatrix = XMMatrixMultiply(XMMatrixRotationY(3.141f), viewMatrix);
	viewMatrix = oneEightyRotMatrix;
	viewMatrix = XMMatrixMultiply(XMMatrixRotationX(0.5853f),viewMatrix);
	viewMatrix = XMMatrixInverse(0, viewMatrix);

	XMStoreFloat4x4(&m_viewToShader.viewMatrix, viewMatrix);
}

void Camera::RTPCamera()
{
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewToShader.viewMatrix);
	viewMatrix = XMMatrixInverse(0, m_parentObj->GetWorldMatrix());
	// Set Position
	//if (m_parentObj->GetID() == MONSTER_ID)
	//	m_viewToShader.viewMatrix.r[3].m128_f32[1] = m_viewToShader.viewMatrix.r[3].m128_f32[1] - 5.0f;
	//else
	//{
		viewMatrix.r[3].m128_f32[0] = viewMatrix.r[3].m128_f32[0] - m_cameraOffset.x;
		viewMatrix.r[3].m128_f32[1] = viewMatrix.r[3].m128_f32[1] - m_cameraOffset.y;
		viewMatrix.r[3].m128_f32[2] = viewMatrix.r[3].m128_f32[2] - m_cameraOffset.z;
	//}

		m_viewToShader.cameraPosition.x = viewMatrix.r[3].m128_f32[0];
		m_viewToShader.cameraPosition.y = viewMatrix.r[3].m128_f32[1];
		m_viewToShader.cameraPosition.z = viewMatrix.r[3].m128_f32[2];


	// Rotating when holding left key
	// Mouse input
	POINT mousePos;
	ZeroMemory(&mousePos, sizeof(POINT));
	GetCursorPos(&mousePos);
	// Calculating the difference of the mouse position
	float deltaY = float(500 - mousePos.y);
	m_rotation -= deltaY * 2.0f;
	SetCursorPos(960, 500);

	// Lowering the camera
	m_cameraOffset.y -= deltaY * 0.001f;
	if (m_cameraOffset.y < 5.5f)
		m_cameraOffset.y = 5.5f;
	else if (m_cameraOffset.y > 10.0f)
		m_cameraOffset.y = 10.0f;

	// Capping the rot 
	if (m_rotation > 12000.0f)
		m_rotation = 12000.0f;
	else if (m_rotation < -2800.0f)
		m_rotation = -2800.0f;


	// Changing the angle
	viewMatrix = XMMatrixInverse(0, viewMatrix);
	XMMATRIX oneEightyRotMatrix = XMMatrixMultiply(XMMatrixRotationY(3.141f), viewMatrix);
	viewMatrix = oneEightyRotMatrix;
	// Rotate	 
	viewMatrix = XMMatrixMultiply(XMMatrixRotationX(m_rotation*0.0001f), viewMatrix);
	viewMatrix = XMMatrixInverse(0, viewMatrix);

	XMFLOAT4X4 v;
	XMStoreFloat4x4(&v, XMMatrixInverse(0, viewMatrix));
	// Get the camera Y rotation
	dynamic_cast<Player*>(m_parentObj)->SetView(v);

	XMStoreFloat4x4(&m_viewToShader.viewMatrix, viewMatrix);


	// Set frustum
	if (m_parentObj)
	{
		float fov = 55.0f;
		float ratio = float(Graphics::GetBackBufferWidth() /Graphics::GetBackBufferHeight());
		XMMATRIX m = XMLoadFloat4x4(&m_viewToShader.viewMatrix);
		m = XMMatrixInverse(0, m);
		m = XMMatrixMultiply(XMMatrixRotationY(3.145f), m);
		XMFLOAT4X4 mat;
		XMStoreFloat4x4(&mat, m);
		
		BuildFrustum(m_frustum, fov, SCREEN_ZNEAR, SCREEN_ZFAR, ratio, mat);
	}

	// Check if the camera is in the bounds of the level
	WithinBounds();
}

void Camera::ResizeCameraProjection()
{
	// Setting the projection matrix
    XMStoreFloat4x4(&m_viewToShader.projectionMatrix, XMMatrixPerspectiveFovLH(45, float(Graphics::GetBackBufferWidth()) / float(Graphics::GetBackBufferHeight()), SCREEN_ZNEAR, SCREEN_ZFAR));
}

void Camera::WithinBounds()
{
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_viewToShader.viewMatrix);
	XMMATRIX camPosition = XMMatrixInverse(0, viewMatrix);

	XMMATRIX parentPosition = m_parentObj->GetWorldMatrix();

	parentPosition.r[3].m128_f32[1] += 5.4f;
	parentPosition.r[3].m128_f32[2] += parentPosition.r[2].m128_f32[2] * 1.1f;

	XMFLOAT3 startPoint, endPoint;

	XMStoreFloat3(&startPoint, parentPosition.r[3]);
	XMStoreFloat3(&endPoint, camPosition.r[3]);

	// Finding the current quadrant that you are in
	Quad* currQuad = ObjManager::GetObjects()[ObjManager::GetObjects().size() - 1]->GetQuad();
	while (currQuad->m_quadrants.size() != 0)
	{
		if (camPosition.r[3].m128_f32[0] < currQuad->m_BL.m_max.x)
		{
			if (camPosition.r[3].m128_f32[2] < currQuad->m_BL.m_max.z)
				currQuad = currQuad->m_quadrants[BOTTOM_LEFT_QUAD];
			else
				currQuad = currQuad->m_quadrants[UP_LEFT_QUAD];
		}
		else
		{
			if (camPosition.r[3].m128_f32[2] < currQuad->m_BR.m_max.z)
				currQuad = currQuad->m_quadrants[BOTTOM_RIGHT_QUAD];
			else
				currQuad = currQuad->m_quadrants[UP_RIGHT_QUAD];
		}
	}
	
	// Checking for collision in to the quadrants triangles
	XMFLOAT3 collisionPoint = { -10000, -10000, -10000 };
	vector<Tri*> tris = currQuad->m_tris;

	// Find all collision points
	float oldDist = 99999;
	for (unsigned int i = 0; i < tris.size(); i++)
	{
		Tri* currTri = tris[i];
		if (Collision::LineSegment2TriangleHacky(collisionPoint, currTri, startPoint, endPoint))
		{
			XMVECTOR dist = XMVector3Length(XMVectorSubtract(XMLoadFloat3(&collisionPoint), camPosition.r[3]));

			if (dist.m128_f32[0] < oldDist)
			{
				oldDist = dist.m128_f32[0];
			
				// Find the closest collision point
				camPosition.r[3].m128_f32[0] = collisionPoint.x + tris[i]->m_normal.x * 0.5f;
				camPosition.r[3].m128_f32[1] = collisionPoint.y;
				camPosition.r[3].m128_f32[2] = collisionPoint.z;


			}
		}
	}

	viewMatrix = XMMatrixInverse(0, camPosition);
	XMStoreFloat4x4(&m_viewToShader.viewMatrix, viewMatrix);

}

void Camera::BuildFrustum(Frustum& frustum, float fov, float nearDist, float farDist, float ratio, const XMFLOAT4X4& camXform)
{
	// TO DO:
	// Calculate the 8 corner points of the frustum and store them in the frustum.corners[] array.
	// Use the FrustumCorners enum in CollisionLibrary.h to index into the corners array.

	XMMATRIX camX = XMLoadFloat4x4(&camXform);

	XMVECTOR nc = camX.r[3] - camX.r[2] * nearDist;
	XMVECTOR fc = camX.r[3] - camX.r[2] * farDist;

	float Hnear = 2 * tan(fov / 2) * nearDist;
	float Hfar = 2 * tan(fov / 2) * farDist;
	float Wnear = Hnear * ratio;
	float Wfar = Hfar * ratio;
	
	XMVECTOR ftl = fc + camX.r[1] * (Hfar * .5f) - camX.r[0] * (Wfar * .5f);
	XMStoreFloat3(&frustum.corners[FTL], ftl);
	XMVECTOR ftr = fc + camX.r[1] * (Hfar * .5f) + camX.r[0] * (Wfar * .5f);
	XMStoreFloat3(&frustum.corners[FTR], ftr);
	XMVECTOR fbl = fc - camX.r[1] * (Hfar * .5f) - camX.r[0] * (Wfar * .5f);
	XMStoreFloat3(&frustum.corners[FBL], fbl);
	XMVECTOR fbr = fc - camX.r[1] * (Hfar * .5f) + camX.r[0] * (Wfar * .5f);
	XMStoreFloat3(&frustum.corners[FBR], fbr);

	XMVECTOR ntl = nc + camX.r[1] * (Hnear * .5f) - camX.r[0] * (Wnear * .5f);
	XMStoreFloat3(&frustum.corners[NTL], ntl);
	XMVECTOR ntr = nc + camX.r[1] * (Hnear * .5f) + camX.r[0] * (Wnear * .5f);
	XMStoreFloat3(&frustum.corners[NTR], ntr);
	XMVECTOR nbl = nc - camX.r[1] * (Hnear * .5f) - camX.r[0] * (Wnear * .5f);
	XMStoreFloat3(&frustum.corners[NBL], nbl);
	XMVECTOR nbr = nc - camX.r[1] * (Hnear * .5f) + camX.r[0] * (Wnear * .5f);
	XMStoreFloat3(&frustum.corners[NBR], nbr);

	// Use the corner points to calculate the frustum planes.
	// This step is completed for you.
	Collision::ComputePlane(frustum.planes[NEAR_PLANE], frustum.corners[NBR], frustum.corners[NBL], frustum.corners[NTL]);
	Collision::ComputePlane(frustum.planes[FAR_PLANE], frustum.corners[FBL], frustum.corners[FBR], frustum.corners[FTR]);
	Collision::ComputePlane(frustum.planes[LEFT_PLANE], frustum.corners[NBL], frustum.corners[FBL], frustum.corners[FTL]);
	Collision::ComputePlane(frustum.planes[RIGHT_PLANE], frustum.corners[FBR], frustum.corners[NBR], frustum.corners[NTR]);
	Collision::ComputePlane(frustum.planes[TOP_PLANE], frustum.corners[NTR], frustum.corners[NTL], frustum.corners[FTL]);
	Collision::ComputePlane(frustum.planes[BOTTOM_PLANE], frustum.corners[NBL], frustum.corners[NBR], frustum.corners[FBR]);
}

bool Camera::FrustumToCapsule(const Frustum& frustum, const Capsule& capsule)
{
	for (size_t i = 0; i < 6; i++)
		if (ClassifyCapsuleToPlane(frustum.planes[i], capsule) == 2)
			return false;

	return true;
}

int Camera::ClassifyCapsuleToPlane(const Plane& plane, const Capsule& capsule)
{
	Sphere end, start;
	end.m_Center = capsule.m_Segment.m_End;
	end.m_Radius = capsule.m_Radius;

	start.m_Center = capsule.m_Segment.m_Start;
	start.m_Radius = capsule.m_Radius;

	//vec3f capsuleOffset = dot_product(plane.normal, capsule.m_Segment.)
	if (ClassifySphereToPlane(plane, end) == 1 && ClassifySphereToPlane(plane, start) == 1)
		return 1;
	else if (ClassifySphereToPlane(plane, end) == 2 && ClassifySphereToPlane(plane, start) == 2)
		return 2;
	else
		return 3;
}

int Camera::ClassifySphereToPlane(const Plane& plane, const Sphere& sphere)
{
	XMVECTOR cen = XMLoadFloat3(&sphere.m_Center);
	XMVECTOR norm = XMLoadFloat3(&plane.m_normal);

	XMVECTOR sphereOffset = XMVector3Dot(norm, cen);

	if (plane.m_offset <= sphereOffset.m128_f32[0] - sphere.m_Radius)
		return 1;
	else if (plane.m_offset > sphereOffset.m128_f32[0] + sphere.m_Radius)
		return 2;
	else
		return 3;
}