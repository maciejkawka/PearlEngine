#include "ChessFramework.h"

#include "Systems/CreatorSystem.h"
#include "Systems/GameSystem.h"
#include "Systems/GameOverSystem.h"
#include "Systems/MainCamera.h"
#include "Systems/OverlaySystem.h"
#include "Systems/PieceSelector.h"
#include "ChessEngine/BoardCoordinator.h"

#include "Engine/Core/ECS/SceneManager.h"
#include "Engine/Core/ECS/Systems/AudioSystem.h"

#include "Engine/Core/Utils/ILogger.h"
#include "Engine/Audio/Resources/ISoundBank.h"
#include "Engine/Audio/Core/IAudioSystem.h"

namespace ChessGame {
    void Game::OnInitalize()
    {

        PrSystems::Get<PrCore::ResourceSystem>()->Load<PrAudio::ISoundBank>("Chess/audio/Master.bank");
        PrSystems::Get<PrCore::ResourceSystem>()->Load<PrAudio::ISoundBank>("Chess/audio/Master.strings.bank");
        
        m_pSoundEvent = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Ambient");
        if (m_pSoundEvent)
        {
            m_pSoundEvent->Play();
        }

        PRLOG_INFO("Initializing Chess Game");

        auto scene = PrSystems::Get<PrCore::SceneManager>()->GetActiveScene();

        // Order matters, systems are created and updated in the registration order
        // CreatorSystem prepares the scene before GameSystem creates BoardCoordinator,
        // GameSystem is updated last to synchronize pieces moved in the same frame
        scene->RegisterSystem<CreatorSystem>();
        scene->RegisterSystem<MainCameraSystem>();
        scene->RegisterSystem<PieceSelectorSystem>();
        scene->RegisterSystem<OverlaySystem>();
        scene->RegisterSystem<GameOverSystem>();
        scene->RegisterSystem<GameSystem>();
        scene->RegisterSystem<PrCore::AudioSystem>();
    }

    bool Game::OnUpdate(float p_dt)
    {
        return true;
    }

    void Game::OnTerminate()
    {
        if (m_pSoundEvent)
            m_pSoundEvent->Stop(PrAudio::StopMode::Immediate);

        PrSystems::Get<PrCore::ResourceSystem>()->UnloadAll<PrAudio::ISoundBank>();
        PRLOG_INFO("Terminating Chess Game");

        BoardCoordinator::Terminate();
    }
}