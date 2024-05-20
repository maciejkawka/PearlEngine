#pragma once
#include"Core/ECS/ECS.h"

#include"Renderer/Core/RenderTypes.h"
#include"Renderer/Core/IRenderBackend.h"
#include"Renderer/Resources/Material.h"

namespace PrCore::ECS {
	class MeshRendererComponent;
	class TransformComponent;
	class CameraComponent;
	class LightComponent;
}

namespace PrRenderer::Core {

	using namespace PrCore;

	class IRenderFrontend {
	public:
		explicit IRenderFrontend(RendererSettings& s_settings)
		{
			m_renderSettings = std::make_shared<RendererSettings>(s_settings);
			m_frameData[0] = std::make_shared<FrameData>();
			m_frameData[1] = std::make_shared<FrameData>();
			m_currentFrame = m_frameData[0];
			m_previousFrame = m_frameData[1];
			m_currentFrameIndex = 0;
		}
		virtual ~IRenderFrontend() = default;

		//Render Entities
		virtual void SubmitLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id) = 0;
		virtual void SetCamera(Camera* p_camera) = 0;
		virtual void SubmitMesh(ECS::Entity& p_entity) = 0;
		virtual void SetCubemap(Resources::MaterialPtr p_cubemap) = 0;
		///////////////////////////////////////

		//Frame
		inline const FrameInfo& GetPreviousFrameInfo() const  { return m_previousFrame->frameInfo; }
		inline const FrameData& GetPreviousFrameData() const { return *m_previousFrame; }

		RendererSettingsPtr GetSettingsPtr() const { return  m_rendererBackend->GetSettingsPtr(); }
		RendererSettings& GetSettings() const { return  m_rendererBackend->GetSettings(); }
		virtual Camera* GetCamera() = 0;

		//More advanced functions
		void PushCommand(RenderCommandPtr p_command) const { m_rendererBackend->PushCommand(p_command); }
		void SetFlag(RendererFlag p_flag) const { m_currentFrame->renderFlag |= RendererFlag::CameraPerspectiveRecalculate; }

		//Draw calls
		virtual void PrepareFrame() = 0;
		virtual void BuildFrame() = 0;
		virtual void FillFrameWithColor() = 0;
		virtual void CalculateFrustrum() = 0;

		//Get Backend
		RendererBackendPtr GetRendererBackend() { return m_rendererBackend; }

		/// Debug
		virtual void DrawDebugCube(const Math::mat4& p_transformMat, bool p_wireframe) = 0;
		virtual void DrawDebugCube(const Math::vec3& p_center, const Math::vec3& p_size, bool p_wireframe = true) = 0;
		virtual void DrawDebugSphere(const Math::vec3& p_center, float p_radius, bool p_wireframe) = 0;
		virtual void DrawDebugLine(const Math::vec3& p_start, const Math::vec3& p_end) = 0;
		virtual void DrawDebugFrustrum(const Math::vec3& p_center, float p_fov, float p_max, float p_min, float p_aspect, bool p_wireframe) = 0;

		virtual void SetDebugColor(const Color& p_color) = 0;
		virtual const Color& GetDebugColor() const { return m_debugColor; }

	protected:
		RendererBackendPtr m_rendererBackend;
		RendererSettingsPtr m_renderSettings;

		std::array<FrameDataPtr, 2> m_frameData;
		FrameDataPtr m_currentFrame;
		FrameDataPtr m_previousFrame;
		size_t m_currentFrameIndex;

		Color m_debugColor;
	};
}