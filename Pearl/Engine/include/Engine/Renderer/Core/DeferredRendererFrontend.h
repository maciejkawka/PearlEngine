#pragma once
#include "Renderer/Core/IRenderFrontend.h"

namespace PrRenderer::Core {

	class DefferedRendererFrontend : public IRendererFrontend, public Utils::Singleton<DefferedRendererFrontend> {
	public:
		explicit DefferedRendererFrontend(const RendererSettings& p_settings);

		void AddLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id) override;
		void AddCamera(ECS::CameraComponent* p_camera) override;
		void AddCamera(Camera* p_camera) { m_currentFrame->camera = p_camera; };
		void AddMesh(ECS::Entity& p_entity) override;
		void AddCubemap(Resources::MaterialPtr p_cubemapMat) override;

		void PrepareFrame() override;
		void BuildFrame() override;
		void FillFrameWithColor() override {}

	private:
		void InstanciateObjects(RenderObjectVector& p_renderObjects);

		//Temporary
		Resources::ShaderPtr m_instancingShader;

		//Lighting
		size_t m_maxPShadowLights;
		size_t m_maxSDShadowLights;
		size_t m_nextPointLightID;
		size_t m_nextOtherLightsID;

		size_t m_pointLightNumber = 0;
		size_t m_otherLightNumber = 0;
	};
}
