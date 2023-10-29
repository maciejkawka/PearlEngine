#include "Core/Common/pearl_pch.h"

#include "Renderer/Core/CascadeShadowMapper.h"

using namespace PrRenderer::Core;

CascadeShadowMapper::CascadeShadowMapper(size_t p_cascadeLayers, size_t p_mapSize):
m_cascadeLayers(p_cascadeLayers),
m_mapSize(p_mapSize)
{
	m_borders = new float[m_cascadeLayers];
	m_lightMats = new PrCore::Math::mat4[m_cascadeLayers];
	m_cameraProjs = new PrCore::Math::mat4[m_cascadeLayers];
}

CascadeShadowMapper::~CascadeShadowMapper()
{
	delete[] m_borders;
	delete[] m_lightMats;
	delete[] m_cameraProjs;
}

void CascadeShadowMapper::SetCameraProj(const PrCore::Math::mat4& p_proj, size_t p_index)
{
	PR_ASSERT(p_index < m_cascadeLayers, "Index bigger than cascade layer");
	m_cameraProjs[p_index] = p_proj;
}

void CascadeShadowMapper::SetBorder(float p_border, size_t p_index)
{
	PR_ASSERT(p_index < m_cascadeLayers, "Index bigger than cascade layer");
	m_borders[p_index] = p_border;
}

const PrCore::Math::mat4& CascadeShadowMapper::GetCameraProj(size_t p_index) const
{
	PR_ASSERT(p_index < m_cascadeLayers, "Index bigger than cascade layer");
	return m_cameraProjs[p_index];
}

const PrCore::Math::mat4& CascadeShadowMapper::GetLightProj(size_t p_index) const
{
	PR_ASSERT(p_index < m_cascadeLayers, "Index bigger than cascade layer");
	return m_lightMats[p_index];
}

PrCore::Math::mat4 CascadeShadowMapper::ClaculateFrustrums(const PrCore::Math::vec3& p_lightDir, const PrCore::Math::mat4& p_cameraView, size_t p_index) const
{
	using namespace PrCore;

	PR_ASSERT(p_index < m_cascadeLayers, "Index bigger than cascade layer");

	auto invView = Math::inverse(m_cameraProjs[p_index] * p_cameraView);

	std::vector<Math::vec4> boundingVertices = {
			{-1.0f,	-1.0f,	-1.0f,	1.0f},
			{-1.0f,	-1.0f,	1.0f,	1.0f},
			{-1.0f,	1.0f,	-1.0f,	1.0f},
			{-1.0f,	1.0f,	1.0f,	1.0f},
			{1.0f,	-1.0f,	-1.0f,	1.0f},
			{1.0f,	-1.0f,	1.0f,	1.0f},
			{1.0f,	1.0f,	-1.0f,	1.0f},
			{1.0f,	1.0f,	1.0f,	1.0f}
	};

	//Frustrum corners Clip space to world space
	for (auto& vert : boundingVertices)
	{
		vert = invView * vert;
		vert /= vert.w;
	}

	//Center of the frustrum
	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : boundingVertices)
	{
		center += glm::vec3(v);
	}
	center /= boundingVertices.size();
	Math::mat4 lightView = Math::lookAt(center - p_lightDir, center, Math::vec3(0.0f, 1.0f, 0.0f));

	Math::vec3 boxA{ lightView * boundingVertices[0] };
	Math::vec3 boxB{ lightView * boundingVertices[0] };

	for (int i = 1; i < boundingVertices.size(); i++)
	{
		auto WorldVert = lightView * boundingVertices[i];

		boxA.x = std::min(WorldVert.x, boxA.x);
		boxB.x = std::max(WorldVert.x, boxB.x);
		boxA.y = std::min(WorldVert.y, boxA.y);
		boxB.y = std::max(WorldVert.y, boxB.y);
		boxA.z = std::min(WorldVert.z, boxA.z);
		boxB.z = std::max(WorldVert.z, boxB.z);
	}

	// Tune this parameter according to the scene
	constexpr float zMult = 10.0f;
	if (boxA.z < 0)
	{
		boxA.z *= zMult;
	}
	else
	{
		boxA.z /= zMult;
	}
	if (boxB.z < 0)
	{
		boxB.z /= zMult;
	}
	else
	{
		boxB.z *= zMult;
	}
	auto projMat = Math::ortho(boxA.x, boxB.x, boxA.y, boxB.y, boxA.z, boxB.z);

	m_lightMats[p_index] = projMat * lightView;

	return m_lightMats[p_index];
}

PrCore::Math::vec4 CascadeShadowMapper::CalculateShadowMapCoords(size_t p_index) const
{
	size_t width = m_mapSize;
	size_t height = m_mapSize;;

	size_t rowCount = m_cascadeLayers / 2.0f;
	size_t row = p_index % rowCount;
	size_t column = p_index / rowCount;

	return { width, height, row * m_mapSize, column * m_mapSize };
}


