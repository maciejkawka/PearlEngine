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

		//Lighting
		size_t m_nextPointLightPos;
		size_t m_nextSpotLightPos;
		size_t m_nextDirLightPos;

		size_t m_pointLightNumber = 0;
		size_t m_spotLightNumber = 0;
		size_t m_dirLightNumber = 0;
	};
}
