#pragma once
#include"Core/Utils/Singleton.h"
#include"Core/ECS/ECS.h"

#include"Renderer/Core/RenderTypes.h"
#include"Renderer/Core/IRendererBackend.h"
#include"Renderer/Resources/Material.h"

namespace PrCore::ECS {
	class MeshRendererComponent;
	class TransformComponent;
	class CameraComponent;
}


namespace PrRenderer::Core {

	using namespace PrCore;
	class IRendererBackend;

	class IRendererFrontend {
	public:
		explicit IRendererFrontend(IRendererBackend* p_rendererBackend):
		m_rendererBackend(p_rendererBackend)
		{
			m_frameData[0] = std::make_shared<FrameData>();
			m_frameData[1] = std::make_shared<FrameData>();
			m_currentFrame = m_frameData[0];
			m_previousFrame = m_frameData[1];
			m_currentFrameIndex = 0;
		}
		virtual ~IRendererFrontend() = default;

		//Render Entities
		virtual void AddCamera(ECS::CameraComponent* p_camera) = 0;
		virtual void AddMesh(ECS::Entity& p_entity) = 0;
		virtual void AddCubemap(Resources::MaterialPtr p_renderObject) = 0;
		///////////////////////////////////////

		//Frame
		inline const FrameInfo& GetCurrFrameInfo() const { return m_currentFrame->frameInfo; }
		inline const FrameInfo& GetPreviousFrameInfo() const  { return m_previousFrame->frameInfo; }
		inline const FrameData& GetCurrFrameData() const { return *m_currentFrame; }
		inline const FrameData& GetPreviousFrameData() const { return *m_previousFrame; }

		void SetRendererSettings(const RendererSettings& p_settings) const { m_rendererBackend->SetSettings(p_settings); }
		const RendererSettings& GetRendererSettings() const { return  m_rendererBackend->GetSettings(); }

		//More advanced functions
		void PushCommand(RenderCommandPtr p_command) const { m_rendererBackend->PushCommand(p_command); }

		//Draw calls
		virtual void PrepareFrame() = 0;
		virtual void BuildFrame() = 0;
		virtual void FillFrameWithColor() = 0;

		/// Debug
		//void FrawGizmo();
		//void DrawLine();

	protected:
		IRendererBackend* m_rendererBackend;

		std::array<FrameDataPtr, 2> m_frameData;
		FrameDataPtr m_currentFrame;
		FrameDataPtr m_previousFrame;
		size_t m_currentFrameIndex;
	};

	//RenderUtils
	namespace RenderUtils
	{
		inline size_t CalculateDepthValue(const PrCore::Math::vec3& p_position, Camera* p_camera)
		{
			auto camera = p_camera;
			auto distance = PrCore::Math::distance(p_position, camera->GetPosition());
			return (distance - camera->GetNear()) / (camera->GetFar() - camera->GetNear()) * 0xFFFFFF;
		}


	}
}