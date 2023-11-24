#include "Core/Common/pearl_pch.h"

#include "Renderer/Core/CascadeShadowMapper.h"

using namespace PrRenderer::Core;

PrCore::Math::mat4 CascadeShadowUtility::ClaculateFrustrums(float& p_cascadeShadowRadiusRatio, size_t p_index, const PrCore::Math::vec3& p_lightDir,
	const PrCore::Math::mat4& p_cameraView, size_t p_mapSize, float ZExtend) const
{
	using namespace PrCore;

	PR_ASSERT(p_index < m_cameraProjs.size(), "Index bigger than cascade layer");

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

	float radius = Math::length(boundingVertices[2] - boundingVertices[5]) / 2.0f;
	float texelsPerUnit = p_mapSize / (2.0f * radius);
	auto scale = Math::scale(Math::mat4(1.0f), Math::vec3(texelsPerUnit));

	auto lookView = Math::lookAt(Math::vec3(0.0f) - p_lightDir, Math::vec3(0.0f), Math::vec3(0.0f, 1.0f, 0.0f));
	lookView = scale * lookView;
	auto lookViewInv = Math::inverse(lookView);

	Math::vec4 centerVec4 = lookView * Math::vec4(center, 1.0f);
	center = centerVec4 / centerVec4.w;
	center.x = Math::floor(center.x);
	center.y = Math::floor(center.y);

	centerVec4 = lookViewInv * Math::vec4(center, 1.0f);
	center = centerVec4 / centerVec4.w;

	Math::vec3 eye = center - (p_lightDir * radius * 2.0f);

	auto viewMat = Math::lookAt(eye, center, Math::vec3(0.0f, 1.0f, 0.0f));
	auto projMat = Math::ortho(-radius, radius,-radius, radius, -radius * ZExtend, radius * ZExtend);

	p_cascadeShadowRadiusRatio = radius;
	return projMat * viewMat;
}