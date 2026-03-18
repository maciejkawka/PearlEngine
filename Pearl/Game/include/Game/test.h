#pragma once

#include "Engine/Core/Entry/GameFramework.h"
#include "Engine/Core/Utils/SystemProvider.h"

#include <memory>

class Game : public PrCore::IGameFramework
{
public:
    void OnInitalize() override;
    bool OnUpdate(float p_dt) override;
    void OnTerminate() override;
};

inline PrCore::IGameFramework* CreateGameFramework()
{
    static std::unique_ptr<Game> gamePtr = std::make_unique<Game>();
    return gamePtr.get();
}
