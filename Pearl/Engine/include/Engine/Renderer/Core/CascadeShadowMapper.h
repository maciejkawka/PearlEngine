#pragma once

#include"Core/Math/Math.h"

namespace PrRenderer::Core {

	class CascadeShadowMapper {
	public:
		CascadeShadowMapper() = delete;
		CascadeShadowMapper(size_t p_cascadeLayers, size_t p_mapSize);
		~CascadeShadowMapper();

		void SetCameraProj(const PrCore::Math::mat4& p_proj, size_t p_index);
		void SetBorder(float p_border, size_t p_index);
		void SetMapSize(size_t p_size) { m_mapSize = p_size; }

		const PrCore::Math::mat4& GetCameraProj(size_t p_index) const;
		const PrCore::Math::mat4& GetLightProj(size_t p_index) const;

		inline size_t                    GetMapSize() const { return m_mapSize; }
		inline const PrCore::Math::mat4* GetProjMats() const { return m_cameraProjs; }
		inline const PrCore::Math::mat4* GetLightMats() const { return m_lightMats; }
		inline const float*              GetBorders() const { return m_borders; }

		PrCore::Math::mat4               ClaculateFrustrums(const PrCore::Math::vec3& p_lightDir, const PrCore::Math::mat4& p_cameraView, size_t p_index) const;
		PrCore::Math::vec4               CalculateShadowMapCoords(size_t p_index) const;

	private:
		size_t              m_cascadeLayers;
		size_t              m_mapSize;

		float*              m_borders;
		PrCore::Math::mat4* m_lightMats;
		PrCore::Math::mat4* m_cameraProjs;

	};


	struct CascadeShadowUtility
	{
		PrCore::Math::mat4               ClaculateFrustrums(size_t p_index, const PrCore::Math::vec3& p_lightDir, const PrCore::Math::mat4& p_cameraView, size_t mapSize, float ZExtend = 10.0f) const;
		PrCore::Math::vec4               CalculateShadowMapCoords(size_t p_index, size_t subMapSize, size_t mapSize) const;

		std::vector<PrCore::Math::mat4> m_cameraProjs;
		std::vector<float>              m_borders;
	};
}