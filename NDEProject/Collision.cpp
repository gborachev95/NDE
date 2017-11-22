#include "Collision.h"
#include "BaseObject.h"
#include <DirectXMath.h>
#include "Monster.h"

using namespace DirectX;
#define SameSign(a,b) ( ((*(unsigned int*)&(a)) & 0x80000000) == ((*(unsigned int*)&(b)) & 0x80000000) )

Collision::Collision()
{
}

Collision::~Collision()
{
}

float Collision::Clamp(float n, float min, float max)
{
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

float Collision::ClosestPtSegmentSegment(const Capsule& cap1, const Capsule& cap2, float &s, float &t, XMVECTOR &c1, XMVECTOR &c2)
{
	XMVECTOR start1 = XMLoadFloat3(&cap1.m_Segment.m_Start);
	XMVECTOR start2 = XMLoadFloat3(&cap2.m_Segment.m_Start);
	XMVECTOR end1 = XMLoadFloat3(&cap1.m_Segment.m_End);
	XMVECTOR end2 = XMLoadFloat3(&cap2.m_Segment.m_End);
	XMVECTOR dir1 = end1 - start1;
	XMVECTOR dir2 = end2 - start2;
	XMVECTOR r = start1 - start2;
	XMVECTOR tempA = XMVector3Dot(dir1, dir1);
	XMVECTOR tempE = XMVector3Dot(dir2, dir2);
	XMVECTOR tempF = XMVector3Dot(dir2, r);

	float a = tempA.m128_f32[0];
	float e = tempE.m128_f32[0];
	float f = tempF.m128_f32[0];

	// Check if either or both segments degenerate into points
	if (a <= FLT_EPSILON && e <= FLT_EPSILON)
	{
		// Both segments degenerate into points
		s = t = 0.0f;
		XMVECTOR c1 = start1;
		XMVECTOR c2 = start2;

		XMVECTOR fin = XMVector3Dot(c1 - c2, c1 - c2);
		return fin.m128_f32[0];
	}
	else if (a <= FLT_EPSILON)
	{
		// First segment degenerates into a point
		s = 0.0f;
		t = f / e;
		t = Clamp(t, 0.0f, 1.0f);
	}
	else
	{
		XMVECTOR tempC = XMVector3Dot(dir1, r);
		float c = tempC.m128_f32[0];
		if (e <= FLT_EPSILON)
		{
			t = 0.0f;
			s = Clamp(-c / a, 0.0f, 1.0f);
		}
		else
		{
			XMVECTOR tempB = XMVector3Dot(dir1, dir2);
			float b = tempB.m128_f32[0];
			float denom = a*e - b*b;

			if (denom != 0.0f)
				s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f);
			else
				s = 0.0f;
			t = (b*s + f) / e;

			if (t < 0.0f)
			{
				t = 0.0f;
				s = Clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = Clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}

	c1 = start1 + dir1 * s;
	c2 = start2 + dir2 * t;

	XMVECTOR fin = XMVector3Dot(c1 - c2, c1 - c2);
	return fin.m128_f32[0];
}

bool Collision::CapsuleToCapsule(const Capsule& cap1, const Capsule& cap2)
{
	float s, t;
	XMVECTOR C1, C2;

	float dist2 = ClosestPtSegmentSegment(cap1, cap2, s, t, C1, C2);
	float radius = cap1.m_Radius + cap2.m_Radius;

	return dist2 <= radius * radius;
}

XMFLOAT3* Collision::SphereToAABB(const Sphere& lhs, const AABB& rhs, XMFLOAT3 *cP)
{
	*cP = lhs.m_Center;

	if (lhs.m_Center.x < rhs.m_min.x)
		cP->x = rhs.m_min.x;
	else if (lhs.m_Center.x > rhs.m_max.x)
		cP->x = rhs.m_max.x;
	if (lhs.m_Center.y < rhs.m_min.y)
		cP->y = rhs.m_min.y;
	else if (lhs.m_Center.y > rhs.m_max.y)
		cP->y = rhs.m_max.y;
	if (lhs.m_Center.z < rhs.m_min.z)
		cP->z = rhs.m_min.z;
	else if (lhs.m_Center.z > rhs.m_max.z)
		cP->z = rhs.m_max.z;

	float dist = sqrt(pow(cP->x - lhs.m_Center.x, 2) + pow(cP->y - lhs.m_Center.y, 2) + pow(cP->z - lhs.m_Center.z, 2));

	if (dist <= lhs.m_Radius)
		return cP;

	return NULL;
}

// Returns true if the Spheres collide. False if not.
bool Collision::SphereToSphere(const Sphere& lhs, const Sphere& rhs)
{
	XMFLOAT3 direction;
	direction.x = lhs.m_Center.x - rhs.m_Center.x;
	direction.y = lhs.m_Center.y - rhs.m_Center.y;
	direction.z = lhs.m_Center.z - rhs.m_Center.z;

	float distance = sqrt(pow(direction.x, 2) + pow(direction.y, 2) + pow(direction.z, 2));

	if (distance <= (lhs.m_Radius + rhs.m_Radius))
		return true;

	return false;
}

bool Collision::CapsuleToSphere(const Capsule& capsule, const Sphere& sphere)
{

	XMFLOAT3 lineVector;
	lineVector.x = capsule.m_Segment.m_End.x - capsule.m_Segment.m_Start.x;
	lineVector.y = capsule.m_Segment.m_End.y - capsule.m_Segment.m_Start.y;
	lineVector.z = capsule.m_Segment.m_End.z - capsule.m_Segment.m_Start.z;
	XMStoreFloat3(&lineVector, XMVector3Normalize(XMLoadFloat3(&lineVector)));

	XMFLOAT3 collisionSphere;
	collisionSphere.x = sphere.m_Center.x - capsule.m_Segment.m_Start.x;
	collisionSphere.y = sphere.m_Center.y - capsule.m_Segment.m_Start.y;
	collisionSphere.z = sphere.m_Center.z - capsule.m_Segment.m_Start.z;

	XMVECTOR ratio = XMVector3Dot(XMLoadFloat3(&lineVector), XMLoadFloat3(&collisionSphere));
	
	if (ratio.m128_f32[0] < 0) 
		ratio.m128_f32[0] = 0;
	if (ratio.m128_f32[0]  > 1)
		ratio.m128_f32[0] = 1;
	
	XMVECTOR collisionPoint = XMLoadFloat3(&capsule.m_Segment.m_Start) + (XMLoadFloat3(&lineVector) * ratio);
	
	Sphere mySphere;
	XMStoreFloat3(&mySphere.m_Center,collisionPoint);
	mySphere.m_Radius = capsule.m_Radius;

	return SphereToSphere(mySphere, sphere);
}

void Collision::ComputePlane(Plane &plane, const XMFLOAT3& _vertA, const XMFLOAT3& _vertB, const XMFLOAT3 &_vertC)
{
	XMVECTOR ABVector = XMVectorSubtract( XMLoadFloat3(&_vertB), XMLoadFloat3(&_vertA));
	XMVECTOR BCVector = XMVectorSubtract(XMLoadFloat3(&_vertC), XMLoadFloat3(&_vertB));
	XMVECTOR normal = XMVector3Cross(ABVector, BCVector);
	normal = XMVector3Normalize(normal);
	XMStoreFloat3(&plane.m_normal, normal);
	plane.m_offset = XMVector3Dot(XMLoadFloat3(&_vertA), normal).m128_f32[0];
}

bool Collision::IntersectRayTriangle(const XMVECTOR &vert0, const XMVECTOR &vert1, const XMVECTOR &vert2, const XMVECTOR &norm, const XMVECTOR &start, const XMVECTOR &d, float &t)
{
	// TODO: Read the header file comments for this function!

	// IntersectRayTriangle
	//
	// In:
	//		vert0 - First vertex of the triangle
	//		vert1 - Second vertex of the triangle
	//		vert2 - Third vertex of the triangle
	//		norm - Normal of the triangle
	//		start - Start of the ray
	//		d - direction of the ray (normalized)
	//
	// Out:
	//		t - Time of intersection, if any
	//
	// Return:
	//		bool - True if intersection, false if not
	//
	// TODO:
	//		If the ray starts behind the triangle or the dot product of the ray normal and tri normal is greater than ED_EPSILON, return false.
	//		Implement the algorithm presented in "Intersecting Line to Triangle 2.ppt"
	//		Assume that the vertices are already sorted properly.
	//		 

	XMVECTOR staNor = XMVector3Dot(start, norm);
	XMVECTOR v0Nor = XMVector3Dot(vert0, norm);
	XMVECTOR dirNor = XMVector3Dot(d, norm);

	if (staNor.m128_f32[0] - v0Nor.m128_f32[0] < 0 || dirNor.m128_f32[0] > sqrt(FLT_EPSILON))
		return false;

	XMVECTOR sa = XMVectorSubtract(vert0, start);
	XMVECTOR sb = XMVectorSubtract(vert1, start);
	XMVECTOR sc = XMVectorSubtract(vert2, start);

	
	XMVECTOR N1; N1 = XMVector3Cross(sc, sb);
	XMVECTOR N2; N2 = XMVector3Cross(sa, sc);
	XMVECTOR N3; N3 = XMVector3Cross(sb, sa);

	// TODO: Complete this function
	// Tip: Use the SameSign() macro

	XMVECTOR r1 = XMVector3Dot(d, N1);
	XMVECTOR r2 = XMVector3Dot(d, N2);
	XMVECTOR r3 = XMVector3Dot(d, N3);

	if (r1.m128_f32[0] == 0 && r2.m128_f32[0] == 0 && r3.m128_f32[0] == 0)
		t = 0;
	else if (SameSign(r1.m128_f32[0], r2.m128_f32[0]) && SameSign(r1.m128_f32[0], r3.m128_f32[0]))
	{
		XMVECTOR offset = XMVector3Dot(norm, vert0);
		XMVECTOR offsetStaNorDirNor = (offset - staNor) / dirNor;
		t = offsetStaNorDirNor.m128_f32[0];
		return true;
	}

	// *Skip testing against backfacing triangles*
	//	If the ray starts behind the triangle plane OR the angle between ray direction and tri normal is greater than 90 degrees
	//		Stop testing

	return false;
}

bool Collision::IntersectRaySphere(const XMVECTOR &p, const XMVECTOR &d, const XMVECTOR &center, float radius, float &t, XMVECTOR &q)
{
	// IntersectRaySphere
	//
	// In:
	//		p - start of the ray
	//		d - direction of the ray (normalized)
	//		center - center point of the sphere
	//		radius - radius of the sphere
	//
	// Out:
	//		t - Time of intersection, if any
	//		q - point of intersection, if any
	//
	// Return:
	//		bool - True if intersection, false if not
	//
	// TODO:
	//		Implement the algorithm presented in "Advanced Ray to Sphere.ppt"
	//		Adjust the algorithm so that the test stops and returns false if the ray points away from the sphere, regardless of ray start position.
	//		This adjustment will make it so that a point moving away from or out of the sphere will not intersect.

	// TODO: Read the header file comments for this function!

	// TODO: Complete this function
	//		 BE SURE TO MODIFY THE ALGORITHM AS SPECIFIED IN THE HEADER FILE COMMENTS

	XMVECTOR v = p - center;
	XMVECTOR b = XMVector3Dot(v, d);
	XMVECTOR h = XMVector3Dot(v, v);
	h.m128_f32[0] -= radius * radius;

	if (h.m128_f32[0] > 0.0f && b.m128_f32[0] > 0.0f)
		return false;

	float discr = b.m128_f32[0] * b.m128_f32[0] - h.m128_f32[0];

	if (discr < 0.0f)
		return false;

	t = -b.m128_f32[0] - sqrt(discr);

	if (t < 0.0f)
		t = 0.0f;

	q = p + d * t;

	return true;
}

bool Collision::IntersectRayCylinder(const XMVECTOR &sa, const XMVECTOR &n, const XMVECTOR &p, const XMVECTOR &q, float r, float &t)
{
	// TODO: Read the header file comments for this function!

	// IntersectRayCylinder
	//
	// In:
	//		sa - start of the ray
	//		n - direction of the ray (normalized)
	//		p - First (base) point on the cylinder segment
	//		q - Second (top) point on the cylinder segment
	//		r - Radius of the cylinder
	//		
	// Out:
	//		t - Time of intersection, if any
	//
	// Return:
	//		bool - True if intersection, false if not
	//
	// TODO:
	//		Optimization - If the ray starts outside the top or bottom planes and points away, there can be no intersection.
	//
	//		Use the quadratic formula to determine if and when the ray intersects the cylinder.
	//      Components (a,b,h) for the quadratic formula are given in the function body.
	//		As with ray to sphere, if the discriminant is less than zero then there is no intersection.
	//		If the time of intersection is negative then return no intersection.
	//		If the point of intersection is not between the top and bottom planes of the cylinder, return no intersection.
	//
	//		This test will only detect intersection with the visible section of the cylinder.
	//		There is no intersection with the endcaps and no intersection with the backfacing surface.
	//
	//		For an in depth explanation of a similar algorithm, see 
	//		"(5.3.7) Intersecting Ray or Segment Against Cylinder" in "Real-Time Collision Detection"
	//		As presented in the book, the algorithm works with finite segments and not rays so it is not all 100% applicable.
	//		(Meaning, if you copy from the book you will likely screw yourself)

	XMVECTOR d = q - p; // vector from first point on cylinder segment to the end point on cylinder segment
	XMVECTOR m = sa - p; // vector from first point on cylinder segment to start point of ray

					  // Values used to calculate coefficients of quadratic formula.
					  // You do not necessarily have to use any of these directly for the rest of the algorithm.
	XMVECTOR dd = XMVector3Dot(d, d); // dot product of d with d (squared magnitude of d)
	XMVECTOR nd = XMVector3Dot(n, d); // dot product of ray normal (n) with d
	XMVECTOR mn = XMVector3Dot(m, n);
	XMVECTOR md = XMVector3Dot(m, d);
	XMVECTOR mm = XMVector3Dot(m, m);

	// TODO: Optimization by early out
	//		 If the ray starts outside the top or bottom planes and points away, there can be no intersection.

	//dot_product(start, norm) - dot_product(vert0, norm) < 0

	// auto hits when you are on the left side
	XMVECTOR saPd = XMVector3Dot(sa - p, d);
	XMVECTOR nD = XMVector3Dot(n, d);

	if (saPd.m128_f32[0] < 0 && nD.m128_f32[0] < 0)
		return false;
	// auto hits when on the right side
	XMVECTOR saQd = XMVector3Dot(sa - q, d);
	if (saQd.m128_f32[0] > 0 && nD.m128_f32[0] > 0)
		return false;

	// Coefficients for the quadratic formula
	float a = dd.m128_f32[0] - nd.m128_f32[0] * nd.m128_f32[0];
	float b = dd.m128_f32[0] * mn.m128_f32[0] - nd.m128_f32[0] * md.m128_f32[0];
	float h = dd.m128_f32[0] * (mm.m128_f32[0] - r * r) - md.m128_f32[0] * md.m128_f32[0];

	// If a is approximately 0.0 then the ray is parallel to the cylinder and can't intersect
	if (abs(a) < FLT_EPSILON)
		return false;

	// TODO: Find time of intersection, if any
	//		 Use the quadratic formula to solve for t. Reference "Advanced Ray to Sphere.ppt" for an example.
	//		 As with "Advanced Ray to Sphere", the 2s and 4 in the formula ( x = (-b - sqrt(b*b - 4ac)) / 2a )
	//		 are cancelled out, resulting in a simplified form.

	//t = (-b - sqrt(b*b - a*h)) / a;
	t = b*b - a*h;

	if (t < 0)
		return false;

	t = (-b - sqrt(t)) / a;

	XMVECTOR v = sa + n * t;

	XMVECTOR sT = v - p;
	XMVECTOR eT = v - q;

	XMVECTOR sTQp = XMVector3Dot(sT, q - p);
	XMVECTOR eTPq = XMVector3Dot(eT, p - q);
	if (sTQp.m128_f32[0] < 0 || eTPq.m128_f32[0] < 0)
		return false;

	return true;
}

bool Collision::IntersectRayCapsule(const XMVECTOR &sa, const XMVECTOR &n, const XMVECTOR &p, const XMVECTOR &q, float r, float &t)
{
	// TODO: Read the header file comments for this function!

	float fTime = FLT_MAX;
	t = FLT_MAX;
	bool bReturn = false;

	// TODO: Complete this function

	if (IntersectRayCylinder(sa, n, p, q, r, t))
		return true;

	XMVECTOR pOI, qOI;

	float t1 = 0, t2 = 0;
	if (IntersectRaySphere(sa, n, p, r, t1, pOI))
	{
		bReturn = true;
		t = t1;
	}

	if (IntersectRaySphere(sa, n, q, r, t2, qOI))
	{
		bReturn = true;
		t = t2;
	}

	if (t1 != 0 && t2 != 0)
		t = fmin(t1, t2);

	return bReturn;
}

bool Collision::IntersectMovingSphereTriangle(const XMVECTOR &vert0, const XMVECTOR &vert1, const XMVECTOR &vert2, const XMVECTOR &norm, const XMVECTOR &start, const XMVECTOR &d, float r, float &t, XMVECTOR &outNormal)
{
	// TODO: Read the header file comments for this function!

	bool bReturn = false;
	float fTime = FLT_MAX;
	t = FLT_MAX;

	XMVECTOR verts[3] = { vert0, vert1, vert2 };

	// TODO: Complete this function	

	// IntersectMovingSphereTriangle
	//
	//	In:
	//		vert0 - First vertex of the triangle
	//		vert1 - Second vertex of the triangle
	//		vert2 - Third vertex of the triangle
	//		norm - Normal of the triangle
	//		start - Start point of the moving sphere
	//		d - direction of the moving sphere (normalized)
	//		r - radius of the sphere
	//
	//	Out:
	//		t - Time of intersection, if any
	//		outNormal - normal of the surface where the intersection occured, if any
	//
	// Return:
	//		bool - True if intersection, false if not
	//
	//	TODO:
	//		Offset the vertices of the triangle in the direction of the triangle normal by the sphere radius
	//		and perform a ray to triangle test. Assume the verts are already properly sorted.
	//		If there was an intersection, stop testing and return the triangle normal as outNormal.
	//		Else, find the earliest time of intersection (if any) between the ray and the edge capsules of the triangle.
	//		(Edge capsules are based off of the original non-offset verts)
	//			If there is an edge capsule intersection, calculate the vector from the closest point on the edge to the point of intersection.
	//			Normalize this vector and return it as outNormal.

	for (size_t i = 0; i < 3; i++)
		verts[i] += (norm * r);

	if (!IntersectRayTriangle(verts[0], verts[1], verts[2], norm, start, d, t))
	{
		//outNormal = norm;
		//return true;
		return false;
	}

	float t0 = FLT_MAX, t1 = FLT_MAX, t2 = FLT_MAX;

	bool r0 = IntersectRayCapsule(start, d, vert0, vert1, r, t0);
	bool r1 = IntersectRayCapsule(start, d, vert1, vert2, r, t1);
	bool r2 = IntersectRayCapsule(start, d, vert2, vert0, r, t2);

	if (r0 || r1 || r2)
	{
		t = fmin(fmin(t0, t1), t2);
		XMVECTOR v = start + (norm * t);
		v = XMVector3Normalize(v);
		outNormal = v;
		//return true;
		//return false;
	}
	
		return bReturn;

	return 0;
}

float Collision::ImplicitLineEquation(XMFLOAT4 _middlePoint, XMFLOAT4 _startXY, XMFLOAT4 _endXY)
{
	float answer = ((_startXY.y - _endXY.y)*_middlePoint.x) + ((_endXY.x - _startXY.x)*_middlePoint.y) + ((_startXY.x * _endXY.y) - (_startXY.y * _endXY.x));
	return answer;
}

void Collision::Barycentric(Tri* _positionPoints, XMFLOAT4 _middlePoint, XMFLOAT3& _storeBYA)
{
	XMFLOAT4 posThree = _positionPoints->m_three.transform;
	XMFLOAT4 posOne = _positionPoints->m_one.transform;
	XMFLOAT4 posTwo = _positionPoints->m_two.transform;
	//posThree.y = 0.1f;
	//posOne.y = 0.1f;
	//posTwo.y = 0.1f;

	// Finds the BYA coordinates
	XMFLOAT3 BetaGamaAlpha
	{
		ImplicitLineEquation(posThree, posOne, posTwo),
		ImplicitLineEquation(posOne, posTwo, posThree),
		ImplicitLineEquation(posTwo, posThree, posOne) 
	};

	XMFLOAT4 middleThree = _middlePoint;
	XMFLOAT4 middleOne = _middlePoint;
	XMFLOAT4 middleTwo = _middlePoint;
	//middleThree.y = 0.1f;
	//middleOne.y = 0.1f;
	//middleTwo.y = 0.1f;
	//middleThree.y = _positionPoints->m_three.transform.y;
	//middleOne.y = _positionPoints->m_one.transform.y;
	//middleTwo.y = _positionPoints->m_two.transform.y;
	
	XMFLOAT3 BYA
	{
		ImplicitLineEquation(middleThree, _positionPoints->m_one.transform, _positionPoints->m_two.transform),
		ImplicitLineEquation(middleOne, _positionPoints->m_two.transform, _positionPoints->m_three.transform),
		ImplicitLineEquation(middleTwo, _positionPoints->m_three.transform, _positionPoints->m_one.transform)
	};
	
	_storeBYA.x = BYA.x / BetaGamaAlpha.x;
	_storeBYA.y = BYA.y / BetaGamaAlpha.y;
	_storeBYA.z = BYA.z / BetaGamaAlpha.z;
}

bool Collision::LineSegment2Triangle(XMFLOAT3 &vOut, Tri *pTri, const XMFLOAT3 &vStart, const XMFLOAT3 &vEnd, float _rad)
{
	float mult = 1.8f;
	XMVECTOR norm = { pTri->m_normal.x, pTri->m_normal.y, pTri->m_normal.z };
	
	XMVECTOR start = { vStart.x - _rad * norm.m128_f32[0] * mult, vStart.y - _rad * norm.m128_f32[1] * mult, vStart.z - _rad * norm.m128_f32[2] * mult };
	XMVECTOR end = { vEnd.x, vEnd.y, vEnd.z };
	XMVECTOR one = { pTri->m_one.transform.x, pTri->m_one.transform.y, pTri->m_one.transform.z };
	XMVECTOR two = { pTri->m_two.transform.x, pTri->m_two.transform.y, pTri->m_two.transform.z };
	XMVECTOR three = { pTri->m_three.transform.x, pTri->m_three.transform.y, pTri->m_three.transform.z };
	XMVECTOR d = XMVector3Dot(start, norm);
	XMVECTOR d1 = XMVector3Dot(one, norm);
	
	if (d.m128_f32[0] - d1.m128_f32[0] > 1.0f)
		return false;
	
	d = XMVector3Dot(end, norm);
	d1 = XMVector3Dot(one, norm);
	
	if (d.m128_f32[0] - d1.m128_f32[0] < -1.0f)
		return false;

	d = XMVector3Dot(norm, start);
	float D0 = d.m128_f32[0];
	d = XMVector3Dot(norm, one);
	float D1 = d.m128_f32[0];
	float D2 = D0 - D1;
	XMVECTOR L = end - start;
	d = XMVector3Dot(norm, L);
	float D3 = d.m128_f32[0];
	float DF = -(D2 / D3);
	XMVECTOR CP = start + L * fabs(DF);

	XMVECTOR Edge0 = two - one;
	XMVECTOR Normal0 = XMVector3Cross(Edge0, norm);
	XMVECTOR Vec0 = CP - one;
	d = XMVector3Dot(Normal0, Vec0);
	float Dis0 = d.m128_f32[0];
	
	if (_rad > 0)
	{
		if (Dis0 - _rad > 0)
			return false;
	}
	else if (Dis0> 0)
		return false;
	
	XMVECTOR Edge1 = three - two;
	XMVECTOR Normal1 = XMVector3Cross(Edge1, norm);
	XMVECTOR Vec1 = CP - two;
	d = XMVector3Dot(Normal1, Vec1);
	float Dis1 = d.m128_f32[0];
	
	if (_rad > 0)
	{
		if (Dis1 - _rad > 0)
			return false;
	}
	else if (Dis1> 0)
		return false;
	
	XMVECTOR Edge2 = one - three;
	XMVECTOR Normal2 = XMVector3Cross(Edge2, norm);
	XMVECTOR Vec2 = CP - three;
	d = XMVector3Dot(Normal2, Vec2);
	float Dis2 = d.m128_f32[0];
	
	if (_rad > 0)
	{
		if (Dis2 - _rad > 0)
			return false;
	}
	else if (Dis2> 0)
		return false;

	if (_rad == 0)
	{
		if (CP.m128_f32[1] > vOut.y && CP.m128_f32[1] <= end.m128_f32[1])
		{
			XMStoreFloat3(&vOut, CP);
			return true;
		}
		else
			return false;
	}
	else
		return true;

}

bool Collision::LineSegment2Triangle(XMVECTOR &vOut, const XMVECTOR& one, const XMVECTOR& two, const XMVECTOR& three, const XMVECTOR& norm, const XMVECTOR &vStart, const XMVECTOR &vEnd, float _rad)
{
	float mult = 1.0f;

	XMVECTOR start = { vStart.m128_f32[0] - _rad * norm.m128_f32[0] * mult, vStart.m128_f32[1] - _rad * norm.m128_f32[1] * mult, vStart.m128_f32[2] - _rad * norm.m128_f32[2] * mult };
	XMVECTOR d = XMVector3Dot(start, norm);
	XMVECTOR d1 = XMVector3Dot(one, norm);

	if (d.m128_f32[0] - d1.m128_f32[0] > 1.0f)
		return false;

	d = XMVector3Dot(vEnd, norm);
	d1 = XMVector3Dot(one, norm);

	if (d.m128_f32[0] - d1.m128_f32[0] < -1.0f)
		return false;

	d = XMVector3Dot(norm, start);
	float D0 = d.m128_f32[0];
	d = XMVector3Dot(norm, one);
	float D1 = d.m128_f32[0];
	float D2 = D0 - D1;
	XMVECTOR L = vEnd - start;
	d = XMVector3Dot(norm, L);
	float D3 = d.m128_f32[0];
	float DF = -(D2 / D3);
	XMVECTOR CP = start + L * fabs(DF);

	XMVECTOR Edge0 = two - one;
	XMVECTOR Normal0 = XMVector3Cross(Edge0, norm);
	XMVECTOR Vec0 = CP - one;
	d = XMVector3Dot(Normal0, Vec0);
	float Dis0 = d.m128_f32[0];

	if (_rad > 0)
	{
		if (Dis0 - _rad > 0)
			return false;
	}
	else if (Dis0> 0)
		return false;

	XMVECTOR Edge1 = three - two;
	XMVECTOR Normal1 = XMVector3Cross(Edge1, norm);
	XMVECTOR Vec1 = CP - two;
	d = XMVector3Dot(Normal1, Vec1);
	float Dis1 = d.m128_f32[0];

	if (_rad > 0)
	{
		if (Dis1 - _rad > 0)
			return false;
	}
	else if (Dis1> 0)
		return false;

	XMVECTOR Edge2 = one - three;
	XMVECTOR Normal2 = XMVector3Cross(Edge2, norm);
	XMVECTOR Vec2 = CP - three;
	d = XMVector3Dot(Normal2, Vec2);
	float Dis2 = d.m128_f32[0];

	if (_rad > 0)
	{
		if (Dis2 - _rad > 0)
			return false;
	}
	else if (Dis2> 0)
		return false;

	if (_rad == 0)
	{
		if (CP.m128_f32[1] > vOut.m128_f32[1] && CP.m128_f32[1] <= vEnd.m128_f32[1])
		{
			vOut = CP;
			return true;
		}
		else
			return false;
	}
	else
	{
		vOut = CP;
		return true;
	}

}

bool Collision::LineSegment2TriangleHacky(XMFLOAT3 &vOut, Tri *pTri, const XMFLOAT3 &vStart, const XMFLOAT3 &vEnd)
{
	XMVECTOR norm = { pTri->m_normal.x, pTri->m_normal.y, pTri->m_normal.z };
	XMVECTOR start = { vStart.x, vStart.y, vStart.z };
	XMVECTOR end = { vEnd.x, vEnd.y, vEnd.z };
	XMVECTOR one = { pTri->m_one.transform.x, pTri->m_one.transform.y, pTri->m_one.transform.z };
	XMVECTOR two = { pTri->m_two.transform.x, pTri->m_two.transform.y, pTri->m_two.transform.z };
	XMVECTOR three = { pTri->m_three.transform.x, pTri->m_three.transform.y, pTri->m_three.transform.z };

	XMVECTOR d = XMVector3Dot(start, norm);
	float D0 = d.m128_f32[0];
	d = XMVector3Dot(norm, one);
	float D1 = d.m128_f32[0];
	float D2 = D0 - D1;
	XMVECTOR L = end - start;
	d = XMVector3Dot(norm, L);
	float D3 = d.m128_f32[0];
	float DF = -(D2 / D3);
	XMVECTOR CP = start + L * fabs(DF);

	XMVECTOR Edge0 = two - one;
	XMVECTOR Normal0 = XMVector3Cross(Edge0, norm);
	XMVECTOR Vec0 = CP - one;
	d = XMVector3Dot(Normal0, Vec0);
	float Dis0 = d.m128_f32[0];

	if (Dis0 > 0)
		return false;

	XMVECTOR Edge1 = three - two;
	XMVECTOR Normal1 = XMVector3Cross(Edge1, norm);
	XMVECTOR Vec1 = CP - two;
	d = XMVector3Dot(Normal1, Vec1);
	float Dis1 = d.m128_f32[0];

	if (Dis1 > 0)
		return false;

	XMVECTOR Edge2 = one - three;
	XMVECTOR Normal2 = XMVector3Cross(Edge2, norm);
	XMVECTOR Vec2 = CP - three;
	d = XMVector3Dot(Normal2, Vec2);
	float Dis2 = d.m128_f32[0];

	if (Dis2 > 0)
		return false;

	if (CP.m128_f32[1] > vOut.y && CP.m128_f32[1] <= end.m128_f32[1])
	{
		XMStoreFloat3(&vOut, CP);
		return true;
	}

	return false;
}

void Collision::WallCollision(BaseObject* _obj, float _deltaTime)
{
	if (!_obj->GetWall())
		return;

	// Gets the current velocity of the playr
	XMFLOAT3 f = _obj->GetForce();

	// Gets the vector from the collider to the body
	XMVECTOR col = { _obj->GetWall()->m_center.x, _obj->GetWall()->m_center.y, _obj->GetWall()->m_center.z };
	XMVECTOR norm = { _obj->GetWall()->m_normal.x, _obj->GetWall()->m_normal.y, _obj->GetWall()->m_normal.z };
	XMVECTOR colliderToBody = XMVectorSubtract(_obj->GetWorldMatrix().r[3], col);

	float maxRange = 3.0f;
	float minX = _obj->GetWall()->m_normal.x * 2.0f;
	float minZ = _obj->GetWall()->m_normal.z * 2.0f;

	XMVECTOR reflect = 0.25f * (-2 * XMVector3Dot(XMLoadFloat3(&f), norm) * norm + XMLoadFloat3(&f));
	XMStoreFloat3(&f, reflect);
	f.x += minX;
	f.z += minZ;

	float totalForce = fabs(f.x) + fabs(f.z);

	if (f.x > maxRange)
		f.x = maxRange;
	else if (f.x < -maxRange)
		f.x = -maxRange;
	if (f.z > maxRange)
		f.z = maxRange;
	else if (f.z < -maxRange)
		f.z = -maxRange;

	//float minRange = 3.0f, maxRange = 4.2f;
	//if (_obj->GetGround() && fabs(f.x) + fabs(f.z) > minRange)
	//{
	//	XMVECTOR reflect = 1.0f * (-2 * XMVector3Dot(XMLoadFloat3(&f), norm) * norm + XMLoadFloat3(&f));
	//	XMStoreFloat3(&f, reflect);
	//}
	//else
	//{
	//	f.x = _obj->GetWall()->m_normal.x * 2.0f;
	//	f.z = _obj->GetWall()->m_normal.z * 2.0f;
	//	//f.x += norm.m128_f32[0] * 1.2f;
	//	//f.z += norm.m128_f32[2] * 1.2f;
	//}




	//if ((f.x < minRange && f.x > -minRange) && (f.z < minRange && f.z > -minRange))
	//{
	//	f.x = _obj->GetWall()->m_normal.x * 2.8f;
	//	f.z = _obj->GetWall()->m_normal.z * 2.8f;
	//}

	//if (norm.m128_f32[0] > 0 && f.x < norm.m128_f32[0])
	//	f.x = norm.m128_f32[0];
	//else if (norm.m128_f32[0] < 0 && f.x > -norm.m128_f32[0])
	//	f.x = -norm.m128_f32[0];
	//if (norm.m128_f32[2] > 0 && f.z < norm.m128_f32[2])
	//	f.z = norm.m128_f32[2];
	//else if (norm.m128_f32[2] < 0 && f.z > -norm.m128_f32[2])
	//	f.z = -norm.m128_f32[2];

	//if (fabs(f.x) + fabs(f.z) < minRange)
	//{
	//	f.x = _obj->GetWall()->m_normal.x * 2.0f;
	//	f.z = _obj->GetWall()->m_normal.z * 2.0f;
	//}
	

	// MAX FORCE
	if (fabs(f.x) + fabs(f.z) > maxRange * 2 && _obj->GetInAir())
	{
		f.x *= .4f;
		f.z *= .4f;
	}

	if (!_obj->GetInAir())
		f.y = 0;
	
	//if (f.x > maxRange)
	//	f.x  = maxRange;
	//else if (f.x < -maxRange)
	//	f.x = -maxRange;
	//if (f.z > maxRange)
	//	f.z = maxRange;
	//else if (f.z < -maxRange)
	//	f.z = -maxRange;

	XMFLOAT3 pos = { _obj->GetWorldMatrix().r[3].m128_f32[0], _obj->GetWorldMatrix().r[3].m128_f32[1], _obj->GetWorldMatrix().r[3].m128_f32[2] };
	if (norm.m128_f32[1] < -.7f && _obj->GetInAir())
	{
		float dis = DistanceFormula(pos, _obj->GetWall()->m_center);

		f.y = -6.0;
	}
	else
		f.y = _obj->GetForce().y;

	//if (f.z < .5f && f.z > -.5f)

	//f.x = _obj->GetWall()->m_normal.x * 1;
	//if (f.y > 0)
	//	f.y = 0;
	//f.z = _obj->GetWall()->m_normal.z * 1;

	// TODO:: REFLECTION VECTOR,MAYBE?

	//// Decides on which side of the body we are located so we can set the correct sigh
	//if (colliderToBody.m128_f32[0] > 0.0f) sideMultiplierX = 1.0f;
	//else sideMultiplierX = -1.0f;
	//if (colliderToBody.m128_f32[2] > 0.0f) sideMultiplierZ = 1.0f;
	//else sideMultiplierZ = -1.0f;
	//
	//XMFLOAT3 collidingNormal;
	//collidingNormal.x = fabs(_obj->GetWall()->m_normal.x);
	//collidingNormal.y = fabs(_obj->GetWall()->m_normal.y);
	//collidingNormal.z = fabs(_obj->GetWall()->m_normal.z);
	//
	//// Prevents sticking when no force
	//if ((f.x < forceMinimum) && (f.x > -forceMinimum))
	//	f.x += forceMinimum * collidingNormal.x * sideMultiplierX;
	//if ((f.z < forceMinimum) && (f.z > -forceMinimum))
	//	f.z += forceMinimum * collidingNormal.z * sideMultiplierZ;
	//
	//f.x -= f.x * collidingNormal.x * _obj->GetMass() * float(_deltaTime) * bounceMultiplier;
	//f.z -= f.z * collidingNormal.z * _obj->GetMass() * float(_deltaTime) * bounceMultiplier;

	_obj->SetForce(f);

	if (_obj->GetID() == MONSTER_ID)
	{
		Monster* mon = dynamic_cast<Monster*>(_obj);
		mon->SetIsDeadly(false);
	}
}

float Collision::DistanceFormula(XMFLOAT3 vec1, XMFLOAT3 vec2)
{
	float dist = sqrt(pow((vec1.x - vec2.x), 2) + pow((vec1.y - vec2.y), 2) + pow((vec1.z - vec2.z), 2));
	return dist;
}

bool Collision::WallSafetyCheck(BaseObject* _obj, Tri* _tri)
{
	float maxX = -FLT_MAX, minX = FLT_MAX, maxY = -FLT_MAX, minY = FLT_MAX, maxZ = -FLT_MAX, minZ = FLT_MAX;

	vector<VERTEX*> verts;
	verts.resize(3);
	verts[0] = &_tri->m_one;
	verts[1] = &_tri->m_two;
	verts[2] = &_tri->m_three;

	for (size_t i = 0; i < 3; i++)
	{
		if (verts[i]->transform.x > maxX)
			maxX = verts[i]->transform.x;
		if (verts[i]->transform.x < minX)
			minX = verts[i]->transform.x;

		if (verts[i]->transform.y > maxY)
			maxY = verts[i]->transform.y;
		if (verts[i]->transform.y < minY)
			minY = verts[i]->transform.y;

		if (verts[i]->transform.z > maxZ)
			maxZ = verts[i]->transform.z;
		if (verts[i]->transform.z < minZ)
			minZ = verts[i]->transform.z;
	}

	XMFLOAT4X4 pos;
	XMStoreFloat4x4(&pos, _obj->GetWorldMatrix());
	float rad = dynamic_cast<Capsule*>(_obj->GetHitBox())->m_Radius;
	//if (pos._41 - rad * _tri->m_normal.x < minX)
	//	return false;
	//else if (pos._41 + rad * _tri->m_normal.x > maxX)
	//	return false;
	//else if (pos._43 - rad * _tri->m_normal.z < minZ)
	//	return false;
	//else if (pos._43 + rad * _tri->m_normal.z > maxZ)
	//	return false;

	if (pos._41 + rad < minX)
		return false;
	else if (pos._41 - rad > maxX)
		return false;
	else if (pos._43 + rad < minZ)
		return false;
	else if (pos._43 - rad > maxZ)
		return false;

	return true;
}

void Collision::ClosestPointToLine(XMFLOAT3 _start, XMFLOAT3 _end, XMFLOAT3 _toVec, XMFLOAT3* _outVec)
{
	XMVECTOR line = { _end.x - _start.x, _end.y - _start.y, _end.z - _start.z };
	XMVECTOR norm = XMVector3Normalize(line);
	XMVECTOR vec = XMLoadFloat3(&_toVec);
	XMVECTOR dot = XMVector3Dot(norm, vec);
	XMVECTOR normP = norm * dot;
	XMVECTOR closest = XMLoadFloat3(&_start) + normP;
	XMStoreFloat3(_outVec, closest);
}