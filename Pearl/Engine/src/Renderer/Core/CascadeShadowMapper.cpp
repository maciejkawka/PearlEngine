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

	//Frustrum corners clip space to world space
	for (auto& vert : boundingVertices)
	{
		vert = invView * vert;
		vert /= vert.w;
	}

	//Center of the frustrum
	Math::vec3 center = Math::vec3(0.0f, 0.0f, 0.0f);
	for (const auto& v : boundingVertices)
	{
		center += Math::vec3(v);
	}
	center /= boundingVertices.size();

	// Compute bounding sphere radius
	float radius = 0.0f;
	for (const auto& v : boundingVertices)
	{
		float distance = Math::length(Math::vec3(v) - center);
		radius = Math::max(radius, distance);
	}

	radius = Math::ceil(radius * p_mapSize) / p_mapSize;
	float texelsPerUnit = static_cast<float>(p_mapSize) / (2.0f * radius);

	// Snap frustum center to shadow map texel grid
	Math::vec4 snappedCenter = Math::vec4(center, 1.0f);
	auto lightView = Math::lookAt(Math::vec3(0.0f) - p_lightDir, Math::vec3(0.0f), Math::vec3(0.0f, 1.0f, 0.0f));
	snappedCenter = lightView * snappedCenter;
	snappedCenter.x = Math::floor(snappedCenter.x * texelsPerUnit) / texelsPerUnit;
	snappedCenter.y = Math::floor(snappedCenter.y * texelsPerUnit) / texelsPerUnit;
	snappedCenter = Math::inverse(lightView) * snappedCenter;

	Math::vec3 lightEye = Math::vec3(snappedCenter) - p_lightDir * radius * 2.0f;
	auto viewMat = Math::lookAt(lightEye, Math::vec3(snappedCenter), Math::vec3(0.0f, 1.0f, 0.0f));
	auto projMat = Math::ortho(-radius, radius, -radius, radius, -radius * ZExtend, radius * ZExtend);

	p_cascadeShadowRadiusRatio = radius;
	return projMat * viewMat;
}