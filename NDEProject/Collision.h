#pragma once
#ifndef Collision_H
#define Collision_H
#include "includes.h"
class BaseObject;


class Collision
{
public:
	Collision();
	~Collision();


	static float Clamp(float n, float min, float max);
	static float ClosestPtSegmentSegment(const Capsule& cap1, const Capsule& cap2, float &s, float &t, XMVECTOR &c1, XMVECTOR &c2);
	static bool CapsuleToCapsule(const Capsule& cap1, const Capsule& cap2);
	static XMFLOAT3* SphereToAABB(const Sphere& lhs, const AABB& rhs, XMFLOAT3 *cP);
	static bool CapsuleToSphere(const Capsule& capsule, const Sphere& sphere);
	static bool SphereToSphere(const Sphere& lhs, const Sphere& rhs);
	static void ComputePlane(Plane &plane, const XMFLOAT3& _vertA, const XMFLOAT3& _vertB, const XMFLOAT3 &_vertC);
	static bool IntersectRayTriangle(const XMVECTOR &vert0, const XMVECTOR &vert1, const XMVECTOR &vert2, const XMVECTOR &norm, const XMVECTOR &start, const XMVECTOR &d, float &t);
	static bool IntersectRaySphere(const XMVECTOR &p, const XMVECTOR &d, const XMVECTOR &center, float radius, float &t, XMVECTOR &q);
	static bool IntersectRayCylinder(const XMVECTOR &sa, const XMVECTOR &n, const XMVECTOR &p, const XMVECTOR &q, float r, float &t);
	static bool IntersectRayCapsule(const XMVECTOR &sa, const XMVECTOR &n, const XMVECTOR &p, const XMVECTOR &q, float r, float &t);
	static bool IntersectMovingSphereTriangle(const XMVECTOR &vert0, const XMVECTOR &vert1, const XMVECTOR &vert2, const XMVECTOR &norm, const XMVECTOR &start, const XMVECTOR &d, float r, float &t, XMVECTOR &outNormal);
	static float ImplicitLineEquation(XMFLOAT4 middlePoint, XMFLOAT4 startXY, XMFLOAT4 _endXY);
	static void Barycentric(Tri* positionPoints, XMFLOAT4 middlePoint, XMFLOAT3& _storeBYA);
	static bool LineSegment2Triangle(XMFLOAT3 &vOut, Tri *pTri, const XMFLOAT3 &vStart, const XMFLOAT3 &vEnd, float _rad);
	static bool LineSegment2Triangle(XMVECTOR &vOut, const XMVECTOR& one, const XMVECTOR& two, const XMVECTOR& three, const XMVECTOR& norm, const XMVECTOR &vStart, const XMVECTOR &vEnd, float _rad);
	static bool LineSegment2TriangleHacky(XMFLOAT3 &vOut, Tri *pTri, const XMFLOAT3 &vStart, const XMFLOAT3 &vEnd);
	static void WallCollision(BaseObject* _obj, float _deltaTime);
	static float DistanceFormula(XMFLOAT3 vec1, XMFLOAT3 vec2);
	static bool WallSafetyCheck(BaseObject* _obj, Tri* _tri);
	static void ClosestPointToLine(XMFLOAT3 _start, XMFLOAT3 _end, XMFLOAT3 _toVec, XMFLOAT3* _outVec);

	// ClassifyPointToPlane
	//int ClassifyPointToPlane(const Plane& plane, const vec3f& point)

	// ClassifySphereToPlane
	//int ClassifySphereToPlane(const Plane& plane, const Sphere& sphere)

	// ClassifyAabbToPlane
	//int ClassifyAabbToPlane(const Plane& plane, const AABB& aabb)

	// ClassifyCapsuleToPlane
	//int ClassifyCapsuleToPlane(const Plane& plane, const Capsule& capsule)

	// Calculates the corner points and planes of the frustum based upon input values.
	//void BuildFrustum(Frustum& frustum, float fov, float nearDist, float farDist, float ratio, const matrix4f& camXform)

	// Perform a Sphere-to-Frustum check. Returns true if the sphere is inside. False if not.
	//bool FrustumToSphere(const Frustum& frustum, const Sphere& sphere)

	// Perform a Aabb-to-Frustum check. Returns true if the aabb is inside. False if not.
	//bool FrustumToAABB(const Frustum& frustum, const AABB& aabb)

	// Perform a Capsule-to-Frustum check. Returns true if the Capsule is inside. False if not.
	//bool FrustumToCapsule(const Frustum& frustum, const Capsule& capsule)


	// Returns true if the AABBs collide. False if not.
	//bool AABBtoAABB(const AABB& lhs, const AABB& rhs)

	// Returns true if the Spheres collide. False if not.
	//bool SphereToSphere(const Sphere& lhs, const Sphere& rhs)

	// Returns true if the sphere collides with the AABB. False if not.
	//bool SphereToAABB(const Sphere& lhs, const AABB& rhs)

	// Returns true if the capsule collides with the sphere. False if not.
	//bool CapsuleToSphere(const Capsule& capsule, const Sphere& sphere);
};

#endif