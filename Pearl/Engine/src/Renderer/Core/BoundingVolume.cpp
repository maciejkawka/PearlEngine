#include "Core/Common/pearl_pch.h"
#include "Renderer/Core/BoundingVolume.h"
#include "Renderer/Core/DeferredRendererFrontend.h"

using namespace PrRenderer::Core;

Frustrum::Frustrum(const PrCore::Math::mat4& m_transformMatrix)
{
	Calculate(m_transformMatrix);
}

Frustrum::Frustrum(const PrCore::Math::mat4& p_projMat, const PrCore::Math::mat4& p_viewMat)
{
	Calculate(p_projMat, p_viewMat);
}

void Frustrum::Calculate(const PrCore::Math::mat4& m_transformMatrix)
{
	auto xAxis = PrCore::Math::vec3(m_transformMatrix[0][0], m_transformMatrix[1][0], m_transformMatrix[2][0]);
	auto yAxis = PrCore::Math::vec3(m_transformMatrix[0][1], m_transformMatrix[1][1], m_transformMatrix[2][1]);
	auto zAxis = PrCore::Math::vec3(m_transformMatrix[0][2], m_transformMatrix[1][2], m_transformMatrix[2][2]);
	auto wAxis = PrCore::Math::vec3(m_transformMatrix[0][3], m_transformMatrix[1][3], m_transformMatrix[2][3]);

	//Right
	m_planes[0] = PrCore::Math_PrTypes::Plane(wAxis - xAxis, m_transformMatrix[3][3] - m_transformMatrix[3][0], true);
	//Left																
	m_planes[1] = PrCore::Math_PrTypes::Plane(wAxis + xAxis, m_transformMatrix[3][3] + m_transformMatrix[3][0], true);
	//Bottom										 				
	m_planes[2] = PrCore::Math_PrTypes::Plane(wAxis + yAxis, m_transformMatrix[3][3] + m_transformMatrix[3][1], true);
	//top															
	m_planes[3] = PrCore::Math_PrTypes::Plane(wAxis - yAxis, m_transformMatrix[3][3] - m_transformMatrix[3][1], true);
	//Near											 					
	m_planes[4] = PrCore::Math_PrTypes::Plane(wAxis + zAxis, m_transformMatrix[3][3] + m_transformMatrix[3][2], true);
	//Far														
	m_planes[5] = PrCore::Math_PrTypes::Plane(wAxis - zAxis, m_transformMatrix[3][3] - m_transformMatrix[3][2], true);
}

void Frustrum::Calculate(const PrCore::Math::mat4& p_projMat, const PrCore::Math::mat4& p_viewMat)
{
	//Gribb/Hartmann method
	//https://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
	auto mat = p_projMat * p_viewMat;
	auto xAxis = PrCore::Math::vec3(mat[0][0], mat[1][0], mat[2][0]);
	auto yAxis = PrCore::Math::vec3(mat[0][1], mat[1][1], mat[2][1]);
	auto zAxis = PrCore::Math::vec3(mat[0][2], mat[1][2], mat[2][2]);
	auto wAxis = PrCore::Math::vec3(mat[0][3], mat[1][3], mat[2][3]);

	//Right
	m_planes[0] = PrCore::Math_PrTypes::Plane(wAxis - xAxis, mat[3][3] - mat[3][0], true);
	//Left																
	m_planes[1] = PrCore::Math_PrTypes::Plane(wAxis + xAxis, mat[3][3] + mat[3][0], true);
	//Bottom										 				
	m_planes[2] = PrCore::Math_PrTypes::Plane(wAxis + yAxis, mat[3][3] + mat[3][1], true);
	//top															
	m_planes[3] = PrCore::Math_PrTypes::Plane(wAxis - yAxis, mat[3][3] - mat[3][1], true);
	//Near											 					
	m_planes[4] = PrCore::Math_PrTypes::Plane(wAxis + zAxis, mat[3][3] + mat[3][2], true);
	//Far														
	m_planes[5] = PrCore::Math_PrTypes::Plane(wAxis - zAxis, mat[3][3] - mat[3][2], true);
}

PrCore::Math_PrTypes::Plane Frustrum::GetPlane(int p_index) const
{
	PR_ASSERT(p_index <= 5 && p_index >= 0, "Frustrum has only planes, trying to access " + std::to_string(p_index) + " plane");
	return m_planes[p_index];
}

BoundingVolume::BoundingVolume():
	m_center(PrCore::Math::vec3(0.0f)),
	m_size(PrCore::Math::vec3(0.0f))
{}

BoundingVolume::BoundingVolume(PrCore::Math::vec3 p_center, PrCore::Math::vec3 p_size):
	m_center(p_center),
	m_size(p_size)
{}

SphereVolume::SphereVolume():
	BoundingVolume(),
	m_radius(1.0f)
{}

SphereVolume::SphereVolume(const std::vector<PrCore::Math::vec3>&p_vertices)
{
	PrCore::Math::vec3 max;
	PrCore::Math::vec3 min;

	auto yAxis = PrCore::Math::vec3(0.0f, 1.0f, 0.0f);
	auto maxDot = PrCore::Math::dot(yAxis, p_vertices[0]);
	auto minDot = PrCore::Math::dot(yAxis, p_vertices[0]);
	for (auto vertex : p_vertices)
	{
		auto dot = PrCore::Math::dot(yAxis, vertex);
		if (dot > maxDot)
		{
			max = vertex;
			maxDot = dot;
		}
		if (dot < minDot)
		{
			min = vertex;
			minDot = dot;
		}
	}

	m_center = (max + min) * 0.5f;
	m_radius = PrCore::Math::length(max - m_center);
	m_size = PrCore::Math::vec3(m_radius * 2.0f);

	//Test if all points are inside
	for(auto vertex : p_vertices)
	{
		//Point is outside :/
		if(PrCore::Math::pow(PrCore::Math::length(vertex - m_center),2) > m_radius * m_radius)
		{
			auto tangentSphere = m_center - m_radius * (vertex - m_center) / PrCore::Math::length(vertex - m_center);
			m_center = (tangentSphere + vertex) * 0.5f;
			m_radius = PrCore::Math::length(vertex - m_center);
		}
	}

	m_size = PrCore::Math::vec3(m_radius * 2.0f);

}

SphereVolume::SphereVolume(const PrCore::Math::vec3 & p_center, float p_radius) :
	BoundingVolume(p_center, PrCore::Math::vec3(p_radius)),
	m_radius(p_radius)
{}

bool SphereVolume::IsOnFrustrum(const Frustrum & p_frustrum, PrCore::Math::mat4 p_transform) const
{
	PrCore::Math::vec3 translatedCenter = p_transform * PrCore::Math::vec4(m_center, 1.0f);
	auto scale = PrCore::Math::vec3(
		PrCore::Math::length(PrCore::Math::vec3(p_transform[0][0], p_transform[1][0], p_transform[2][0])),
		PrCore::Math::length(PrCore::Math::vec3(p_transform[0][1], p_transform[1][1], p_transform[2][1])),
		PrCore::Math::length(PrCore::Math::vec3(p_transform[0][2], p_transform[1][2], p_transform[2][2]))
	);

	auto scaledRadiusVec = scale * m_radius;
	float scaledRadius = std::max({scaledRadiusVec.x, scaledRadiusVec.y, scaledRadiusVec.z});
	for (int i = 0; i < 6; i++)
	{
		auto plane = p_frustrum.GetPlane(i);
		auto distance = PrCore::Math::dot(translatedCenter, plane.GetNormal()) + plane.GetDistance();
		if (distance <= -scaledRadius)
			return false;
	}

	return true;
}

BoxVolume::BoxVolume():
	BoundingVolume(),
	m_min(PrCore::Math::vec3(0.0f)),
	m_max(PrCore::Math::vec3(0.0f)),
	m_extends(PrCore::Math::vec3(0.0f))
{}

BoxVolume::BoxVolume(const std::vector<PrCore::Math::vec3>&p_vertices)
{
	m_max = p_vertices[0];
	m_min = p_vertices[0];
	for(auto vertex : p_vertices)
	{
		if (vertex.x < m_min.x) m_min.x = vertex.x;
		if (vertex.x > m_max.x) m_max.x = vertex.x;

		if (vertex.y < m_min.y) m_min.y = vertex.y;
		if (vertex.y > m_max.y) m_max.y = vertex.y;

		if (vertex.z < m_min.z) m_min.z = vertex.z;
		if (vertex.z > m_max.z) m_max.z = vertex.z;
	}

	m_center = (m_max + m_min) * 0.5f;
	m_extends = PrCore::Math::vec3(
		PrCore::Math::max(m_max.x - m_center.x, 0.01f),
		PrCore::Math::max(m_max.y - m_center.y, 0.01f),
		PrCore::Math::max(m_max.z - m_center.z, 0.01f)
	);
	m_size = m_extends * 2.0f;
}

BoxVolume::BoxVolume(const PrCore::Math::vec3& p_min, const PrCore::Math::vec3 & p_max):
	m_min(p_min),
	m_max(p_min)
{
	m_center = (p_max + p_min) * 0.5f;
	m_extends = PrCore::Math::vec3(
		p_max.x - m_center.x,
		p_max.y - m_center.y,
		p_max.z - m_center.z
	);
	m_size = m_extends * 2.0f;
}

bool BoxVolume::IsOnFrustrum(const Frustrum& p_frustrum, PrCore::Math::mat4 p_transform) const
{
	// Calculate new AABB with transform applied
	PrCore::Math::vec3 translatedCenter = p_transform * PrCore::Math::vec4(m_center, 1.0f);

	PrCore::Math::vec3 corners[8];
	corners[0] = m_min;
	corners[1] = PrCore::Math::vec3(m_min.x, m_min.y, m_max.z);
	corners[2] = PrCore::Math::vec3(m_min.x, m_max.y, m_min.z);
	corners[3] = PrCore::Math::vec3(m_max.x, m_min.y, m_min.z);
	corners[4] = PrCore::Math::vec3(m_min.x, m_max.y, m_max.z);
	corners[5] = PrCore::Math::vec3(m_max.x, m_min.y, m_max.z);
	corners[6] = PrCore::Math::vec3(m_max.x, m_max.y, m_min.z);
	corners[7] = m_max;

	PrCore::Math::vec3 newMax{ -FLT_MAX };
	PrCore::Math::vec3 newMin{ FLT_MAX };

	for (int i = 0; i < 8; i++) {
		PrCore::Math::vec3 transformed = p_transform * PrCore::Math::vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
		newMin = PrCore::Math::min(newMin, transformed);
		newMax = PrCore::Math::max(newMax, transformed);
	}

	PrCore::Math::vec3 newExtends = PrCore::Math::vec3(
		newMax.x - translatedCenter.x,
		newMax.y - translatedCenter.y,
		newMax.z - translatedCenter.z
	);

	//PrRenderer::Core::DefferedRendererFrontend::GetInstance().DrawCube(translatedCenter, newExtends * 2.0f,  true);

	// Check if AABB in frustrum
	for (int i = 0; i < 6; i++)
	{
		auto plane = p_frustrum.GetPlane(i);
		auto planeNormal = plane.GetNormal();
		float radius = PrCore::Math::abs(planeNormal.x) * newExtends.x +
			PrCore::Math::abs(planeNormal.y) * newExtends.y +
			PrCore::Math::abs(planeNormal.z) * newExtends.z;

		float distance = PrCore::Math::dot(planeNormal, translatedCenter) + plane.GetDistance();

		if (distance <= -radius)
		{
			return false;
		}
	}

	return true;
}