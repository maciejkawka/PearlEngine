#include "Core/Common/pearl_pch.h"

#include "Renderer/Core/CascadeShadowMapper.h"

using namespace PrRenderer::Core;

PrCore::Math::mat4 CascadeShadowUtility::ClaculateFrustrums(size_t p_index, const PrCore::Math::vec3& p_lightDir,
	const PrCore::Math::mat4& p_cameraView, size_t mapSize, float ZExtend) const
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
	if (boxA.z < 0)
		boxA.z *= ZExtend;
	else
		boxA.z /= ZExtend;

	if (boxB.z < 0)
		boxB.z /= ZExtend;
	else
		boxB.z *= ZExtend;

	auto projMat = Math::ortho(boxA.x, boxB.x, boxA.y, boxB.y, boxA.z, boxB.z);
	return projMat * lightView;
}