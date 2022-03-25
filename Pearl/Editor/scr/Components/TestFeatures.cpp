#include"Editor/Components/TestFeatures.h"

#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/MeshManager.h"

using namespace PrEditor::Components;

TestFeatures::TestFeatures(PrRenderer::Core::Renderer3D* p_renderer3D) :
	m_renderer3D(p_renderer3D)
{
	auto zombyMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("zombyMaterial.mat"));
	auto cubeMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("cubeMaterial.mat"));
	auto groundMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("groundMaterial.mat"));
	auto zomby = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().Load("Zoombie.obj"));
	auto cube = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().Load("cube.obj"));
	auto ground = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().Load("groundplane.obj"));
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{	
	auto zombyMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("zombyMaterial.mat"));
	auto cubeMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("cubeMaterial.mat"));
	auto groundMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("groundMaterial.mat"));
	auto zomby = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().GetResource("Zoombie.obj"));
	auto cube = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().GetResource("cube.obj"));
	auto ground = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().GetResource("groundplane.obj"));

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			m_renderer3D->DrawMeshNow(zomby, PrCore::Math::vec3(i * 5, 0, j * 4), PrCore::Math::vec3(), PrCore::Math::vec3(1), zombyMaterial);
			m_renderer3D->DrawMeshNow(cube, PrCore::Math::vec3(i * 5, 1.f, j * 4 + 1.f), PrCore::Math::vec3(), PrCore::Math::vec3(0.5), cubeMaterial);
		}
	}

	m_renderer3D->DrawMeshNow(ground, PrCore::Math::vec3(0), PrCore::Math::vec3(0), PrCore::Math::vec3(1), groundMaterial);
}
