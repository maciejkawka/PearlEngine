#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/MeshRendererSystem.h"
#include "Core/Resources/ResourceSystem.h"
#include "Renderer/Core/IRenderFrontend.h"

using namespace PrCore::ECS;
using namespace PrRenderer;

MeshRendererSystem::~MeshRendererSystem()
{
}

void MeshRendererSystem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroup::Custom;
}

void MeshRendererSystem::OnEnable()
{
	PrSystems::Get<IRenderFrontend>()->SetFlag(RendererFlag::CameraPerspectiveRecalculate);
}

void MeshRendererSystem::OnDisable()
{
}

void MeshRendererSystem::OnUpdate(float p_dt)
{
	auto pRenderer = PrSystems::Get<IRenderFrontend>();
	pRenderer->CalculateFrustrum();

	for (auto [entity, light, transform] : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		pRenderer->SubmitLight(light, transform, entity.GetID().GetID());
	}

	for (auto [entity, mesh, transform] : m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		pRenderer->SubmitMesh(entity);
	}
}