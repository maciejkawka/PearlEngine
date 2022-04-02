#include"Editor/Components/TestFeatures.h"

#include"Core/Input/InputManager.h"

#include"Renderer/Core/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/MeshManager.h"

using namespace PrEditor::Components;

TestFeatures::TestFeatures(PrRenderer::Core::Renderer3D* p_renderer3D) :
	m_renderer3D(p_renderer3D)
{
	auto zombyMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("zombyMaterialNew.mat"));
	auto cubeMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("cubeMaterialNew.mat"));
	auto lightMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("lightMaterial.mat"));
	auto groundMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().Load("groundMaterial.mat"));
	auto zomby = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().Load("Zoombie.obj"));
	auto cube = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().Load("cube.obj"));
	auto ground = std::static_pointer_cast<PrRenderer::Resources::Mesh>(PrRenderer::Resources::MeshManager::GetInstance().Load("groundplane.obj"));
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{	
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT))
		dir += -1.0f;
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::RIGHT))
		dir += 1.0f;

	//Calculate Direction (Angles ot normal vector)
	auto rotationQuat = PrCore::Math::quat(PrCore::Math::vec3(PrCore::Math::radians(dir), PrCore::Math::radians(0.0f), PrCore::Math::radians(0.0f)));
	auto rotationDir = PrCore::Math::vec3(0, 0, 1) * rotationQuat;

	//PrRenderer::Core::Light light(PrRenderer::Core::LightType::Directional);
	//light.SetPosition({ 5,5,0 });
	//light.SetColor({ 1,1,1 });
	//light.SetDirection({dir,0.0f,0.0f});
	//light.SetAttenuation(0.0075f, 0.045f, 1.0);
	//light.SetRange(100.0f);
	//m_renderer3D->AddLight(light);

	//PrRenderer::Core::Light light1(PrRenderer::Core::LightType::Point);
	//light1.SetPosition({ 50,5,0 });
	//light1.SetAttenuation(0.0075f, 0.045f, 1.0);
	//light1.SetRange(100.0f);
	//m_renderer3D->AddLight(light1);

	//PrRenderer::Core::Light light2(PrRenderer::Core::LightType::Point);
	//light2.SetPosition({ 50,5,50 });
	//light2.SetAttenuation(0.0075f, 0.045f, 1.0);
	//light2.SetRange(100.0f);
	//m_renderer3D->AddLight(light2);

	PrRenderer::Core::Light light3(PrRenderer::Core::LightType::Spot);
	light3.SetPosition({ 0,5,-0 });
	light3.SetAttenuation(0.0075f, 0.045f, 1.0);
	light3.SetDirection(rotationDir);
	light3.SetRange(100.0f);
	light3.SetInnerCone(5.0);
	light3.SetOutterCone(10.0);
	m_renderer3D->AddLight(light3);


	auto zombyMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("zombyMaterialNew.mat"));
	auto cubeMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("cubeMaterialNew.mat"));
	auto lightMaterial = std::static_pointer_cast<PrRenderer::Resources::Material>(PrRenderer::Resources::MaterialManager::GetInstance().GetResource("lightMaterial.mat"));
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


	//if(PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::UP))
	//	lightPos.x += 0.1f;
	//if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::DOWN))
	//	lightPos.x += -0.1f;
	//if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT))
	//	lightPos.z += -0.1f;
	//if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::RIGHT))
	//	lightPos.z += 0.1f;
	//if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::RIGHT_CONTROL))
	//	lightPos.y+= -0.1f;
	//if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::RIGHT_SHIFT))
	//	lightPos.y += 0.1f;




	m_renderer3D->DrawMeshNow(cube, { 5,5,0 }, PrCore::Math::vec3(), PrCore::Math::vec3(0.5), lightMaterial);
}
