#pragma once
#include "Renderer/Core/IRenderFrontend.h"

namespace PrRenderer::Core {

	class DefferedRendererFrontend : public IRendererFrontend, public Utils::Singleton<DefferedRendererFrontend> {
	public:
		explicit DefferedRendererFrontend(RendererSettings& p_settings);

		void AddLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id) override;
		void SetCamera(Camera* p_camera) override { m_camera = p_camera; };
		void AddMesh(ECS::Entity& p_entity) override;
		void SetCubemap(Resources::MaterialPtr p_cubemapMat) override;

		void PrepareFrame() override;
		void BuildFrame() override;
		void FillFrameWithColor() override {}

	private:
		size_t InstanciateObjects(RenderObjectVector& p_renderObjects);

		//Lighting shadowing
		size_t m_nextPointLightPos;
		size_t m_nextSpotLightPos;
		size_t m_nextDirLightPos;

		size_t m_pointLightShadowNumber = 0;
		size_t m_spotLightShadowNumber = 0;
		size_t m_dirLightShadowNumber = 0;

		RenderObjectPtr m_cubemapObject;
		Camera* m_camera;
	};
}
