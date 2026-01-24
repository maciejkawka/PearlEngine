#pragma once

#include "Renderer/Resources/Texture.h"

namespace PrRenderer {

	class Texture2D : public Texture {
	public:
		// Texture factories
		static std::shared_ptr<Texture2D> Create();
		static std::shared_ptr<Texture2D> CreateUnitTex(const Color& p_unitColor);
	};

	REGISTRER_RESOURCE_HANDLE(Texture2D);
	using Texture2DPtr = std::shared_ptr<Texture2D>;
}
