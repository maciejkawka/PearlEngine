#include"Editor/Components/TestFeatures.h"

#include"Core/Input/InputManager.h"

#include"Renderer/Core/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Cubemap.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Resources/Texture2D.h"
#include"Core/Resources/ResourceLoader.h"

using namespace PrEditor::Components;

TestFeatures::TestFeatures(PrRenderer::Core::Renderer3D* p_renderer3D) :
	m_renderer3D(p_renderer3D)
{
	m_mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);


	auto cubemapMaterialHDR = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat");

	auto mesh = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Mesh>("yo.obj");
	m_renderer3D->SetCubemap(cubemapMaterialHDR);
}

TestFeatures::~TestFeatures()
{}

void TestFeatures::Update(float p_deltaTime)
{
	m_renderer3D->SetAmbientLight({ 0.2f,0.2f,0.2f });
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::LEFT))
		dir += 1.0f;
	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::RIGHT))
		dir += -1.0f;


	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::Q))
		m_renderer3D->SetCubemap(nullptr);

	if (PrCore::Input::InputManager::IsKeyHold(PrCore::Input::PrKey::E))
	{
		auto cubemapMaterialHDR = PrCore::Resources::ResourceLoader::GetInstance().GetResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat");
		m_renderer3D->SetCubemap(cubemapMaterialHDR);
	}

	//Calculate Direction (Angles ot normal vector)
	auto rotationQuat = PrCore::Math::quat(PrCore::Math::vec3(PrCore::Math::radians(dir), PrCore::Math::radians(0.0f), PrCore::Math::radians(0.0f)));
	PrCore::Math::vec3 rotationDir = PrCore::Math::vec3(0, 0, 1) * rotationQuat;

	PrRenderer::Core::Light light1(PrRenderer::Core::LightType::Point);
	light1.SetPosition({ -0 + dir/10,2,10 });
	light1.SetDirection(rotationDir);
	light1.SetColor({ 300, 300, 300});
	m_renderer3D->AddLight(light1);

	PrRenderer::Core::Light light1_1(PrRenderer::Core::LightType::Point);
	light1_1.SetPosition({ -0 + dir / 10,2,5 });
	light1_1.SetDirection(rotationDir);
	light1_1.SetColor({ 300, 300, 300 });
	m_renderer3D->AddLight(light1_1);

	//PrRenderer::Core::Light light2(PrRenderer::Core::LightType::Spot);
	//light2.SetPosition({ -0, 10, 15});
	//light2.SetInnerCone(30);
	//light2.SetOutterCone(45);
	//light2.SetDirection(rotationDir);
	//light2.SetColor({ 300, 300, 300 });
	//m_renderer3D->AddLight(light2);

	//PrRenderer::Core::Light light3(PrRenderer::Core::LightType::Point);
	//light3.SetPosition({- 10 + dir,-10,10 });
	//light3.SetColor({ 300.0f, 5000, 300.0f });
	//m_renderer3D->AddLight(light3);

	//PrRenderer::Core::Light light4(PrRenderer::Core::LightType::Point);
	//light4.SetPosition({ 10 + dir,-10,10 });
	//light4.SetColor({ 300.0f, 300.0f, 300.0f });
	//m_renderer3D->AddLight(light4);

	auto cubemap = m_renderer3D->GetIRMap();
	auto lut = m_renderer3D->GetLUT();
	auto prefiltered = m_renderer3D->GetPrefiltered();


	auto pbr = PrCore::Resources::ResourceLoader::GetInstance().GetResource<PrRenderer::Resources::Material>("PBR_IR_UniversalBrick.mat");
	pbr->SetTexture("irradianceMap", cubemap);
	pbr->SetTexture("prefilterMap", prefiltered);
	pbr->SetTexture("brdfLUT", lut);
	pbr->SetProperty("normalMapping", true);
	//pbr->SetProperty("aoValue", 1.0f);
	pbr->SetProperty<PrCore::Math::vec3>("albedoValue", {0,0,0});
	//pbr->SetProperty("metallicValue", 0.5f);
	//pbr->SetProperty("roughnessValue", 0.05f);
	//pbr->SetProperty("albedoMap_scale", PrCore::Math::vec2(1));
	m_renderer3D->DrawMeshNow(m_mesh, PrCore::Math::vec3(2.0f), PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }), PrCore::Math::vec3(2.0f), pbr);

	pbr->SetProperty<PrCore::Math::vec3>("albedoValue", { 1,1,1 });
	m_renderer3D->DrawMeshNow(m_mesh, {-0 + dir/10, 2, 10}, PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }), PrCore::Math::vec3(0.5f), pbr);
	m_renderer3D->DrawMeshNow(m_mesh, {-0 + dir/10, 2, 5}, PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }), PrCore::Math::vec3(0.5f), pbr);


	auto pbrIR = PrCore::Resources::ResourceLoader::GetInstance().GetResource<PrRenderer::Resources::Material>("PBR_IR_Values.mat");
	pbrIR->SetProperty("albedoValue", PrCore::Math::vec3(1.0f, 0, 0));
	pbrIR->SetProperty("aoValue", 1.0f);
	float row = 7;
	float columns = 7;
	for (int i = 0; i < row; i++)
	{
		pbrIR->SetProperty("metallicValue", i / row);

		for (int j = 0; j < columns; j++)
		{
			pbrIR->SetProperty("roughnessValue", PrCore::Math::clamp((float)j / (float)columns, 0.05f, 1.0f));
			pbrIR->SetTexture("irradianceMap", cubemap);
			pbrIR->SetTexture("prefilterMap", prefiltered);
			pbrIR->SetTexture("brdfLUT", lut);
			m_renderer3D->DrawMeshNow(m_mesh,
				PrCore::Math::vec3((j - (columns / 2)) * 2, (i - (row / 2)) * 2, -2.0f),
				PrCore::Math::quat({ 0.f, 0.0f, 0.f}),
				PrCore::Math::vec3(1.0f),
				pbrIR);
		}
	}

	auto mesh = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Mesh>("yo.obj");
	auto gun = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("PBR_IR_UniversalGun.mat");
	gun->SetProperty("normalMapping", true);
	gun->SetProperty("aoValue", 1.0f);
	gun->SetTexture("irradianceMap", cubemap);
	gun->SetTexture("prefilterMap", prefiltered);
	gun->SetTexture("brdfLUT", lut);
	m_renderer3D->DrawMeshNow(mesh, { 40, 2, 10 }, PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }), PrCore::Math::vec3(0.05f), gun);


	mesh = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Mesh>("TV.obj");
	gun = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("PBR_IR_UniversalTV.mat");
	gun->SetProperty("normalMapping", true);
	gun->SetTexture("irradianceMap", cubemap);
	gun->SetTexture("prefilterMap", prefiltered);
	gun->SetTexture("brdfLUT", lut);
	m_renderer3D->DrawMeshNow(mesh, { 20, 2, 0 }, PrCore::Math::quat({ PrCore::Math::radians(0.0f), PrCore::Math::radians(0.f),PrCore::Math::radians(0.0f) }), PrCore::Math::vec3(0.05f), gun);
}
