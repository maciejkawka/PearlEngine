#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer3D.h"
#include"Renderer/Core/LowRenderer.h"
#include"Renderer/Core/Camera.h"

#include"Core/Events/WindowEvents.h"
#include"Core/Events/EventManager.h"

using namespace PrRenderer::Core;

PrRenderer::Core::Renderer3D::Renderer3D()
{
	PrCore::Events::EventListener windowResizedListener;
	windowResizedListener.connect<&Renderer3D::OnWindowResize>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
}

void Renderer3D::Begin()
{
	Core::LowRenderer::EnableDepth(true);
	Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
	LowRenderer::ClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void PrRenderer::Core::Renderer3D::DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material)
{
	auto camera = PrRenderer::Core::Camera::GetMainCamera();
	auto MVP = camera->RecalculateMatrices();
	MVP *= PrCore::Math::translate(PrCore::Math::mat4(1.0f), p_position) * PrCore::Math::scale(PrCore::Math::mat4(1), p_scale);

	p_material->SetProperty("MVP", MVP);
	p_material->Bind();
	p_mesh->Bind();

	LowRenderer::Draw(p_mesh->GetVertexArray());
}

void Renderer3D::Flush()
{
}

void Renderer3D::OnWindowResize(PrCore::Events::EventPtr p_event)
{
	auto windowResizeEvent = std::static_pointer_cast<PrCore::Events::WindowResizeEvent>(p_event);
	auto width = windowResizeEvent->m_width;
	auto height = windowResizeEvent->m_height;

	LowRenderer::SetViewport(width, height);
}
