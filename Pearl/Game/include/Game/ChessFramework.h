#pragma once

#include "Engine/Core/Entry/GameFramework.h"
#include "Engine/Core/Utils/SystemProvider.h"

#include <memory>

namespace ChessGame {
    class Game : public PrCore::IGameFramework
    {
    public:
        void OnInitalize() override;
        bool OnUpdate(float p_dt) override;
        void OnTerminate() override;
    };
}

inline PrCore::IGameFramework* CreateGameFramework()
{
    static std::unique_ptr<ChessGame::Game> gamePtr = std::make_unique<ChessGame::Game>();
    return gamePtr.get();
}
