#include "ChessFramework.h"

#include "SceneCreators/SceneCreator.h"
#include "SceneCreators/CreatorSystem.h"

#include "Systems/GameSystem.h"
#include "Systems/MainCamera.h"
#include "Systems/OverlaySystem.h"
#include "Systems/PieceSelector.h"

#include "Engine/Core/Utils/ILogger.h"

namespace ChessGame {
    void Game::OnInitalize()
    {
        PRLOG_INFO("This is game initalized");

        PrSystems::Get<PrCore::SceneManager>()->GetActiveScene()->RegisterSystem<CreatorSystem>();
        PrSystems::Get<PrCore::SceneManager>()->GetActiveScene()->RegisterSystem<MainCameraSystem>();
        PrSystems::Get<PrCore::SceneManager>()->GetActiveScene()->RegisterSystem<PieceSelectorSystem>();
        PrSystems::Get<PrCore::SceneManager>()->GetActiveScene()->RegisterSystem<OverlaySystem>();
        PrSystems::Get<PrCore::SceneManager>()->GetActiveScene()->RegisterSystem<GameSystem>();
    }

    bool Game::OnUpdate(float p_dt)
    {
        return true;
    }

    void Game::OnTerminate()
    {
        PRLOG_INFO("Terminating EngineCore");
    }
}