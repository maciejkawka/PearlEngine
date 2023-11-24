#pragma once

#include"Core/Math/Math.h"

namespace PrRenderer::Core {

	struct CascadeShadowUtility
	{
		PrCore::Math::mat4               ClaculateFrustrums(float& p_cascadeShadowRadiusRatio, size_t p_index, const PrCore::Math::vec3& p_lightDir, const PrCore::Math::mat4& p_cameraView, size_t mapSize, float ZExtend = 10.0f) const;

		std::vector<PrCore::Math::mat4> m_cameraProjs;
		std::vector<float>              m_borders;
	};
}