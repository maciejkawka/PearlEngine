#pragma once
#include "Core/Entry/AppContext.h"
#include "Renderer/Core/IRenderFrontend.h"
#include "Renderer/Resources/Shader.h"

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
		void CalculateFrustrum() override;

		virtual void DrawCube(const Math::mat4& p_transformMat, bool p_wireframe = true) override;
		virtual void DrawCube(const Math::vec3& p_center, const Math::vec3& p_size, bool p_wireframe = true) override;
		virtual void DrawSphere(const Math::vec3& p_center, float p_radius, bool p_wireframe = true) override;
		virtual void DrawLine(const Math::vec3& p_start, const Math::vec3& p_end) override;
		virtual void DrawFrustrum(const Math::vec3& p_center, float p_fov, float p_max, float p_min, float p_aspect, bool p_wireframe = true) override {};

		virtual void SetDebugColor(const Color& p_color) override;

		Camera* GetCamera() { return m_camera; }

	private:
		size_t InstanciateObjectsByMaterial(RenderObjectVector& p_renderObjects);
		size_t InstanciateObjectsByMesh(RenderObjectVector& p_renderObjects);

		//Lighting shadowing
		size_t m_nextPointLightPos;
		size_t m_nextSpotLightPos;
		size_t m_nextDirLightPos;

		size_t m_pointLightShadowNumber = 0;
		size_t m_spotLightShadowNumber = 0;
		size_t m_dirLightShadowNumber = 0;

		size_t m_pointLightNumber = 0;
		size_t m_spotLightNumber = 0;
		size_t m_dirLightNumber = 0;

		RenderObjectPtr m_cubemapObject;
		Resources::ShaderPtr m_debugShader;
		Resources::MaterialPtr m_debugMaterial;
		Camera* m_camera;
		Frustrum m_frustrum;
	};
}
