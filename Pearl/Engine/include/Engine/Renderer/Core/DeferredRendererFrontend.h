#pragma once
#include "Renderer/Core/IRenderFrontend.h"

namespace PrRenderer::Core {

	class DefferedRendererFrontend : public IRendererFrontend, public Utils::Singleton<DefferedRendererFrontend> {
	public:
		explicit DefferedRendererFrontend(IRendererBackend* p_rendererBackend);

		void AddCamera(ECS::CameraComponent* p_camera) override;
		void AddCamera(Camera* p_camera) { m_currentFrame->camera = p_camera; };
		void AddMesh(ECS::Entity& p_entity) override;
		void AddCubemap(Resources::MaterialPtr p_cubemapMat) override;

		void PrepareFrame() override;
		void BuildFrame() override;
		void FillFrameWithColor() override {}

	private:
		void InstanciateObjects(RenderObjectVector& p_renderObjects);


		Resources::ShaderPtr m_instancingShader;
	};
}
