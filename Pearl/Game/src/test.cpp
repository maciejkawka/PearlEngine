#include "test.h"

#include "Engine/Core/Math/Math.h"
#include "Engine/Core/Utils/ILogger.h"
#include "Engine/Core/Input/InputManager.h"
#include "Engine/Core/ECS/SceneManager.h"
#include "Engine/Core/ECS/Scene.h"
#include "Engine/Core/ECS/Components/CoreComponents.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Systems/AudioSystem.h"

#include "Engine/Renderer/Core/IRenderFrontend.h"
#include "Engine/Renderer/Resources/Texture2D.h"
#include "Engine/Renderer/Resources/Material.h"

#include "Engine/Audio/Resources/ISoundBank.h"

void Game::OnInitalize()
{
    PRLOG_INFO("This is game initalized");

    auto scene101 = PrSystems::Get<PrCore::SceneManager>()->LoadScene("scene/physics_test.pearl");
    scene101->RegisterSystem<PrCore::AudioSystem>();

    PrSystems::Get<PrCore::ResourceSystem>()->Load<PrAudio::ISoundBank>("Master Bank.bank");
    PrSystems::Get<PrCore::ResourceSystem>()->Load<PrAudio::ISoundBank>("Master Bank.strings.bank");

    //scene101->RegisterSystem<PrCore::PhysicsUpdateSystem>();
    return;
}

bool Game::OnUpdate(float p_dt)
{
    //PR_ASSERT(false);

    if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::B))
    {
        auto scene = PrSystems::Get<PrCore::SceneManager>()->GetActiveScene();
        auto physicsPtr = PrSystems::Get<PrPhysics::PhysicsSystem>();
        auto pRenderer = PrSystems::Get<PrRenderer::IRenderFrontend>();
        auto camera = pRenderer->GetCamera();

        auto entity = scene->CreateEntity("Test_Pan");
        auto entityCam = scene->GetEntityByName("CameraCube12");
        auto camTransform = entityCam.GetComponent<PrCore::TransformComponent>();

        auto logoTransform = entity.AddComponent<PrCore::TransformComponent>();
        auto logoMesh = entity.AddComponent<PrCore::MeshRendererComponent>();
        logoTransform->SetPosition(camera->GetPosition() + camTransform->GetForwardVector() * 2.0f);
        logoTransform->SetLocalScale(PrCore::Math::vec3(0.05f));
        auto newMaterial = std::make_shared<PrRenderer::Material>(*PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Material>("stress_test/capsule.mat").GetData());
        newMaterial->SetProperty("roughnessValue", 0.9f);
        newMaterial->SetProperty("emissionColor", PrRenderer::Color::Red);
        newMaterial->SetTexture("emissionMap", PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Texture>("texture/checkerboard.png"));
        logoMesh->mainMaterial = newMaterial;
        logoMesh->mesh = PrSystems::Get<PrCore::ResourceSystem>()->Load<PrRenderer::Mesh>("stress_test/pan.obj");
        logoMesh->shadowCaster = false;


        // Physics
        PrPhysics::Material material;
        material.staticFriction = 0.0f;
        material.dynamicFriction = 0.1f;
        material.restitution = .1f;

        auto physcomponent = entity.AddComponent<PrCore::RigidBodyDynamicComponent>();

        auto rigidBody = physcomponent->rigidBody;
        auto shape = physicsPtr->CreateShape(PrPhysics::SphereGeometry{ 0.5f }, material);
        rigidBody->AttachShape(shape);
    }

    return true;
}

void Game::OnTerminate()
{
    PRLOG_INFO("Terminating EngineCore");
}