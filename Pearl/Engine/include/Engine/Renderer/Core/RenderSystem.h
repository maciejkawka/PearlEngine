#pragma once

#include"IRenderFrontend.h"

namespace PrRenderer::Core {

	inline std::unique_ptr<IRenderFrontend> renderSystem{ nullptr };
}