#include "ChessFramework.h"

#include "Systems/CreatorSystem.h"
#include "Systems/GameSystem.h"
#include "Systems/MainCamera.h"
#include "Systems/OverlaySystem.h"
#include "Systems/PieceSelector.h"
#include "ChessEngine/BoardCoordinator.h"

#include "Engine/Core/ECS/SceneManager.h"

#include "Engine/Core/Utils/ILogger.h"

namespace ChessGame {
    void Game::OnInitalize()
    {
        PRLOG_INFO("Initializing Chess Game");

        auto scene = PrSystems::Get<PrCore::SceneManager>()->GetActiveScene();

        // Order matters, systems are created and updated in the registration order
        // CreatorSystem prepares the scene before GameSystem creates BoardCoordinator,
        // GameSystem is updated last to synchronize pieces moved in the same frame
        scene->RegisterSystem<CreatorSystem>();
        scene->RegisterSystem<MainCameraSystem>();
        scene->RegisterSystem<PieceSelectorSystem>();
        scene->RegisterSystem<OverlaySystem>();
        scene->RegisterSystem<GameSystem>();
    }

    bool Game::OnUpdate(float p_dt)
    {
        return true;
    }

    void Game::OnTerminate()
    {
        PRLOG_INFO("Terminating Chess Game");

        BoardCoordinator::Terminate();
    }
}