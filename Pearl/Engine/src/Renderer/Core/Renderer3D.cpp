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
	m_color = PrCore::Math::vec3(0.0f);
}

void Renderer3D::Begin()
{
	Core::LowRenderer::EnableDepth(true);
	Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
	LowRenderer::ClearColor(0.1f, 0.1f, 0.8f, 1.0f);
}

void Renderer3D::AddLight(const Light& p_light)
{
	if (m_lightData.size() == 4)
	{
		PRLOG_WARN("Renderer supports 4 lights only");
		return;
	}

	m_lightData.push_back(p_light.GetPackedMatrix());
}

void Renderer3D::SetAmbientLight(PrRenderer::Core::Color p_ambientColor)
{
	m_color = p_ambientColor;
}

void PrRenderer::Core::Renderer3D::DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material)
{
	auto camera = PrRenderer::Core::Camera::GetMainCamera();
	const auto& VPMatrix = camera->RecalculateMatrices();
	auto modelMatrix = PrCore::Math::translate(PrCore::Math::mat4(1.0f), p_position) * PrCore::Math::scale(PrCore::Math::mat4(1), p_scale);
	
	if(p_material->HasProperty("camPos"))
		p_material->SetProperty("camPos", camera->GetPosition());
	
	if (p_material->HasProperty("VPMatrix"))
		p_material->SetProperty("VPMatrix", VPMatrix);
	
	if (p_material->HasProperty("modelMatrix"))
		p_material->SetProperty("modelMatrix", modelMatrix);

	if (p_material->HasProperty("MVP"))
		p_material->SetProperty("MVP", VPMatrix * modelMatrix);
	
	if (p_material->HasProperty("ambientColor"))
		p_material->SetProperty("ambientColor", m_color);

	if (!m_lightData.empty())
	{
		if (p_material->HasProperty("lightMat[0]"))
			p_material->SetPropertyArray("lightMat[0]", m_lightData.data(), m_lightData.size());

		if (p_material->HasProperty("lightNumber"))
			p_material->SetProperty("lightNumber", (int)m_lightData.size());
	}

	p_material->Bind();
	p_mesh->Bind();

	LowRenderer::Draw(p_mesh->GetVertexArray());
	p_material->Unbind();
}

void Renderer3D::Flush()
{
	m_lightData.clear();
}

void Renderer3D::OnWindowResize(PrCore::Events::EventPtr p_event)
{
	auto windowResizeEvent = std::static_pointer_cast<PrCore::Events::WindowResizeEvent>(p_event);
	auto width = windowResizeEvent->m_width;
	auto height = windowResizeEvent->m_height;

	LowRenderer::SetViewport(width, height);
}
