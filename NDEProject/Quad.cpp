#include "Quad.h"
#include "Collision.h"

void Quad::Divide(AABB _base)
{
	if (m_quadrants.size() != 4)
	{
		m_quadrants.resize(4);
		
		for (size_t i = 0; i < 4; i++)
		{
			m_quadrants[i] = new Quad;
			m_quadrants[i]->d_base = d_base;
			d_base->d_list.push_back(m_quadrants[i]);
		}
	}

	m_BL.m_max.x = _base.m_min.x + (_base.m_max.x - _base.m_min.x) * 0.5f;
	m_BL.m_min.x = _base.m_min.x;
	m_BL.m_max.z = _base.m_min.z + (_base.m_max.z - _base.m_min.z) * 0.5f;
	m_BL.m_min.z = _base.m_min.z;
	m_quadrants[BOTTOM_LEFT_QUAD]->m_name = m_name + "_BL";
	m_quadrants[BOTTOM_LEFT_QUAD]->m_ALL = m_BL;

	m_BR.m_max.x = _base.m_max.x;
	m_BR.m_min.x = _base.m_max.x - (_base.m_max.x - _base.m_min.x) * 0.5f;
	m_BR.m_max.z = _base.m_min.z + (_base.m_max.z - _base.m_min.z) * 0.5f;
	m_BR.m_min.z = _base.m_min.z;
	m_quadrants[BOTTOM_RIGHT_QUAD]->m_name = m_name + "_BR";
	m_quadrants[BOTTOM_RIGHT_QUAD]->m_ALL = m_BR;

	m_UL.m_max.x = _base.m_min.x + (_base.m_max.x - _base.m_min.x) * 0.5f;
	m_UL.m_min.x = _base.m_min.x;
	m_UL.m_max.z = _base.m_max.z;
	m_UL.m_min.z = _base.m_max.z - (_base.m_max.z - _base.m_min.z) * 0.5f;
	m_quadrants[UP_LEFT_QUAD]->m_name = m_name + "_UL";
	m_quadrants[UP_LEFT_QUAD]->m_ALL = m_UL;

	m_UR.m_max.x = _base.m_max.x;
	m_UR.m_min.x = _base.m_max.x - (_base.m_max.x - _base.m_min.x) * 0.5f;
	m_UR.m_max.z = _base.m_max.z;
	m_UR.m_min.z = _base.m_max.z - (_base.m_max.z - _base.m_min.z) * 0.5f;
	m_quadrants[UP_RIGHT_QUAD]->m_name = m_name + "_UR";
	m_quadrants[UP_RIGHT_QUAD]->m_ALL = m_UR;
}

void Quad::Distribute()
{
	XMFLOAT3 verts[3];
	int numQuads = 0;
	Tri tri;

	for (size_t i = 0; i < m_tris.size(); i++)
	{
		verts[0] = XMFLOAT3(m_tris[i]->m_one.transform.x, m_tris[i]->m_one.transform.y, m_tris[i]->m_one.transform.z);
		verts[1] = XMFLOAT3(m_tris[i]->m_two.transform.x, m_tris[i]->m_one.transform.y, m_tris[i]->m_two.transform.z);
		verts[2] = XMFLOAT3(m_tris[i]->m_three.transform.x, m_tris[i]->m_one.transform.y, m_tris[i]->m_three.transform.z);

		for (size_t j = 0; j < 3; j++) // for each vert
		{
			if (verts[j].x < m_BL.m_max.x)
			{
				if (verts[j].z < m_BL.m_max.z)
				{
					if (m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size() == 0 || m_quadrants[BOTTOM_LEFT_QUAD]->m_tris[m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size() - 1] != m_tris[i])
					{
						m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.push_back(m_tris[i]);
						m_tris[i]->d_path.push_back(this->m_name + "_BL");
						numQuads++;
					}
				}
				else if (m_quadrants[UP_LEFT_QUAD]->m_tris.size() == 0 || m_quadrants[UP_LEFT_QUAD]->m_tris[m_quadrants[UP_LEFT_QUAD]->m_tris.size() - 1] != m_tris[i])
				{
					m_quadrants[UP_LEFT_QUAD]->m_tris.push_back(m_tris[i]);
					m_tris[i]->d_path.push_back(this->m_name + "_UL");
					numQuads++;
				}
			}
			else
			{
				if (verts[j].z < m_BR.m_max.z)
				{
					if (m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size() == 0 || m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris[m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size() - 1] != m_tris[i])
					{
						m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.push_back(m_tris[i]);
						m_tris[i]->d_path.push_back(this->m_name + "_BR");
						numQuads++;
					}
				}
				else if (m_quadrants[UP_RIGHT_QUAD]->m_tris.size() == 0 || m_quadrants[UP_RIGHT_QUAD]->m_tris[m_quadrants[UP_RIGHT_QUAD]->m_tris.size() - 1] != m_tris[i])
				{
					m_quadrants[UP_RIGHT_QUAD]->m_tris.push_back(m_tris[i]);
					m_tris[i]->d_path.push_back(this->m_name + "_UR");
					numQuads++;
				}
			}
		}

		// check for edge cases
		//XMFLOAT3 f;
		//tri = *m_tris[i];
		//tri.m_one.transform.y = 0;
		//tri.m_two.transform.y = 0;
		//tri.m_three.transform.y = 0;
		//
		//float t = 0.0f;
		////if (Collision::IntersectRayTriangle(XMLoadFloat3(&verts[0]), XMLoadFloat3(&verts[1]), XMLoadFloat3(&verts[2]), XMLoadFloat3(&m_tris[i]->m_normal), 
		////	XMLoadFloat3(&XMFLOAT3(m_BL.m_min.x, 0, m_BL.m_max.z)), XMLoadFloat3(&XMFLOAT3(m_BL.m_max.x, 0, m_BL.m_max.z)), t))
		////if (Collision::LineSegment2Triangle(f, &tri, XMFLOAT3(m_BL.m_min.x, 0, m_BL.m_max.z), XMFLOAT3(m_BL.m_max.x, 0, m_BL.m_max.z)))
		//{
		//	if (m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size() == 0 || m_quadrants[BOTTOM_LEFT_QUAD]->m_tris[m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_BL");
		//		numQuads++;
		//	}
		//	if (m_quadrants[UP_LEFT_QUAD]->m_tris.size() == 0 || m_quadrants[UP_LEFT_QUAD]->m_tris[m_quadrants[UP_LEFT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[UP_LEFT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_UL");
		//		numQuads++;
		//	}
		//}
		//if (Collision::IntersectRayTriangle(XMLoadFloat3(&verts[0]), XMLoadFloat3(&verts[1]), XMLoadFloat3(&verts[2]), XMLoadFloat3(&m_tris[i]->m_normal), 
		//	XMLoadFloat3(&XMFLOAT3(m_UL.m_max.x, 0, m_UL.m_max.z)), XMLoadFloat3(&XMFLOAT3(m_UL.m_max.x, 0, m_UL.m_min.z)), t))
		////if (Collision::LineSegment2Triangle(f, &tri, XMFLOAT3(m_UL.m_max.x, 0, m_UL.m_max.z), XMFLOAT3(m_UL.m_max.x, 0, m_UL.m_min.z)))
		//{
		//	if (m_quadrants[UP_LEFT_QUAD]->m_tris.size() == 0 || m_quadrants[UP_LEFT_QUAD]->m_tris[m_quadrants[UP_LEFT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[UP_LEFT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_UL");
		//		numQuads++;
		//	}
		//	if (m_quadrants[UP_RIGHT_QUAD]->m_tris.size() == 0 || m_quadrants[UP_RIGHT_QUAD]->m_tris[m_quadrants[UP_RIGHT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[UP_RIGHT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_UR");
		//		numQuads++;
		//	}
		//}
		//if (Collision::IntersectRayTriangle(XMLoadFloat3(&verts[0]), XMLoadFloat3(&verts[1]), XMLoadFloat3(&verts[2]), XMLoadFloat3(&m_tris[i]->m_normal),
		//	XMLoadFloat3(&XMFLOAT3(m_UR.m_max.x, 0, m_UR.m_min.z)), XMLoadFloat3(&XMFLOAT3(m_UR.m_min.x, 0, m_UR.m_min.z)), t))
		////if (Collision::LineSegment2Triangle(f, &tri, XMFLOAT3(m_UR.m_max.x, 0, m_UR.m_min.z), XMFLOAT3(m_UR.m_min.x, 0, m_UR.m_min.z)))
		//{
		//	if (m_quadrants[UP_RIGHT_QUAD]->m_tris.size() == 0 || m_quadrants[UP_RIGHT_QUAD]->m_tris[m_quadrants[UP_RIGHT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[UP_RIGHT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_UR");
		//		numQuads++;
		//	}
		//	if (m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size() == 0 || m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris[m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_BR");
		//		numQuads++;
		//	}
		//}
		//if (Collision::IntersectRayTriangle(XMLoadFloat3(&verts[0]), XMLoadFloat3(&verts[1]), XMLoadFloat3(&verts[2]), XMLoadFloat3(&m_tris[i]->m_normal),
		//	XMLoadFloat3(&XMFLOAT3(m_BR.m_min.x, 0, m_BR.m_min.z)), XMLoadFloat3(&XMFLOAT3(m_BR.m_min.x, 0, m_BR.m_max.z)), t))
		////if (Collision::LineSegment2Triangle(f, &tri, XMFLOAT3(m_BR.m_min.x, 0, m_BR.m_min.z), XMFLOAT3(m_BR.m_min.x, 0, m_BR.m_max.z)))
		//{
		//	if (m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size() == 0 || m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris[m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_BR");
		//		numQuads++;
		//	}
		//	if (m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size() == 0 || m_quadrants[BOTTOM_LEFT_QUAD]->m_tris[m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size() - 1] != m_tris[i])
		//	{
		//		m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.push_back(m_tris[i]);
		//		m_tris[i]->d_path.push_back(this->m_name + "_BL");
		//		numQuads++;
		//	}
		//}
		if (numQuads > 1)
			SafetyCheck(m_tris[i]);

		numQuads = 0;
	}
}

void Quad::SafetyCheck(Tri* _tri)
{
	unsigned int sizBL = m_quadrants[BOTTOM_LEFT_QUAD]->m_tris.size();
	unsigned int sizBR = m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris.size();
	unsigned int sizUL = m_quadrants[UP_LEFT_QUAD]->m_tris.size();
	unsigned int sizUR = m_quadrants[UP_RIGHT_QUAD]->m_tris.size();
	if (sizBL > 0 && sizUR > 0 && m_quadrants[BOTTOM_LEFT_QUAD]->m_tris[sizBL - 1] == _tri && m_quadrants[UP_RIGHT_QUAD]->m_tris[sizUR - 1] == _tri
		|| sizBR > 0 && sizUL > 0 &&  m_quadrants[BOTTOM_RIGHT_QUAD]->m_tris[sizBR - 1] == _tri && m_quadrants[UP_LEFT_QUAD]->m_tris[sizUL - 1] == _tri)
	{
		for (size_t l = 0; l < 4; l++)
		{
			if (m_quadrants[l]->m_tris.size() == 0 || m_quadrants[l]->m_tris[m_quadrants[l]->m_tris.size() - 1] != _tri)
			{
				m_quadrants[l]->m_tris.push_back(_tri);
				_tri->d_path.push_back(this->m_name + "_EDGE");
			}
		}
	}
}