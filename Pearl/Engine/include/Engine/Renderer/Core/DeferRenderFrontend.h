#pragma once
#include "Renderer/Core/IRenderFrontend.h"
#include "Renderer/Resources/Shader.h"

namespace PrRenderer::Core {

	class DeferRenderFrontend : public IRenderFrontend {
	public:
		explicit DeferRenderFrontend(RendererSettings& p_settings);

		void SubmitLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id) override;
		void SetCamera(Camera* p_camera) override { m_camera = p_camera; };
		void SubmitMesh(ECS::Entity& p_entity) override;
		void SetCubemap(Resources::Materialv2Ptr p_cubemapMat) override;

		void PrepareFrame() override;
		void BuildFrame() override;
		void FillFrameWithColor() override {}
		void CalculateFrustrum() override;

		virtual void DrawDebugCube(const Math::mat4& p_transformMat, bool p_wireframe = true) override;
		virtual void DrawDebugCube(const Math::vec3& p_center, const Math::vec3& p_size, bool p_wireframe = true) override;
		virtual void DrawDebugSphere(const Math::vec3& p_center, float p_radius, bool p_wireframe = true) override;
		virtual void DrawDebugLine(const Math::vec3& p_start, const Math::vec3& p_end) override;
		virtual void DrawDebugFrustrum(const Math::vec3& p_center, float p_fov, float p_max, float p_min, float p_aspect, bool p_wireframe = true) override {};

		virtual void SetDebugColor(const Color& p_color) override;

		Camera* GetCamera() override { return m_camera; }

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
		Resources::Shaderv2Ptr m_debugShader;
		Resources::Materialv2Ptr m_debugMaterial;
		Camera* m_camera;
		Frustrum m_frustrum;
	};
}