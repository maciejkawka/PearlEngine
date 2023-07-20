#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/Resources/ResourceLoader.h"

#include"Renderer/Core/Renderer3D.h"
#include"Renderer/Core/DeferredRendererFrontend.h"

using namespace PrCore::ECS;
using namespace PrRenderer::Core;

//PrCore::Math::mat4 MeshRendererSystem::GetPackedMatrix(const LightComponent* p_lightComponent, const TransformComponent* p_transform) const
//{
//	Math::mat4 mat;
//
//	auto position = p_transform->GetPosition();
//	mat[0][0] = position.x;
//	mat[0][1] = position.y;
//	mat[0][2] = position.z;
//	mat[0][3] = (float)p_lightComponent->GetType();
//
//	auto direction = p_transform->GetForwardVector();
//	mat[1][0] = direction.x;
//	mat[1][1] = direction.y;
//	mat[1][2] = direction.z;
//	mat[1][3] = Math::cos(Math::radians(p_lightComponent->GetInnerCone()));
//
//	auto color = p_lightComponent->GetColor();
//	mat[2][0] = color.x;
//	mat[2][1] = color.y;
//	mat[2][2] = color.z;
//	mat[2][3] = Math::cos(Math::radians(p_lightComponent->GetOutterCone()));
//
//	mat[3][0] = p_lightComponent->GetQuadraticAttenuation();
//	mat[3][1] = p_lightComponent->GetLinearAttenuation();
//	mat[3][2] = p_lightComponent->GetConstantAttenuation();
//	mat[3][3] = p_lightComponent->GetRange();
//
//	return mat;
//}

MeshRendererSystem::~MeshRendererSystem()
{
	delete m_camera;
}

void MeshRendererSystem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroup::Custom;
	m_camera = new Camera();
	auto cubemapMaterialHDR = Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat");
	Renderer3D::GetInstance().SetCubemap(cubemapMaterialHDR);
}

void MeshRendererSystem::OnEnable()
{
}

void MeshRendererSystem::OnDisable()
{
	Renderer3D::GetInstance().SetMainCamera(nullptr);
}

void MeshRendererSystem::OnUpdate(float p_dt)
{
	//Set First Camera that was found in the scene
	for (auto entity : m_entityViewer.EntitesWithComponents<CameraComponent, TransformComponent>())
	{
		auto camera = entity.GetComponent<CameraComponent>()->GetCamera();
		auto transform = entity.GetComponent<TransformComponent>();

		camera->SetPosition(transform->GetPosition());
		camera->SetRotation(transform->GetRotation());

		if (camera != Renderer3D::GetInstance().GetMainCamera())
			Renderer3D::GetInstance().SetMainCamera(camera);

		break;
	}

	DefferedRendererFrontend::GetInstance().AddCubemap(Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat"));

	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto light = entity.GetComponent<LightComponent>();

		DefferedRendererFrontend::GetInstance().AddLight(light, transform, entity.GetID().GetID());
		//auto mat = GetPackedMatrix(light, transform);
		//Renderer3D::GetInstance().AddLight(mat);
	}

	for(auto entity: m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto meshRenderer = entity.GetComponent<MeshRendererComponent>();
		auto material = meshRenderer->material;
		auto mesh = meshRenderer->mesh;

		MeshRenderObject object;
		object.mesh = mesh;
		object.material = material;
		object.worldMat = std::move(transform->GetWorldMatrix());
		object.position = std::move(transform->GetPosition());
		Renderer3D::GetInstance().AddMeshRenderObject(std::move(object));
		PrRenderer::Core::DefferedRendererFrontend::GetInstance().AddMesh(entity);
	}
}
