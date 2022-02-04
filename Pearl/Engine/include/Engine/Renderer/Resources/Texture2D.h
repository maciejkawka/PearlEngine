#pragma once
#include"Renderer/Resources/Texture.h"

namespace PrRenderer::Core {
	class Color;
}

namespace PrRenderer::Resources {
	
	class Texture2D : public Texture {
	public:

		//To implement in future
		//void SetPixel(unsigned int p_u, unsigned int p_v);
		//Core::Color GetPixel(unsigned int p_u, unsigned int p_v) const;

		//void SetPixels(const Core::Color p_colorArray[]);
		//const Core::Color* GetPixels();
	};
}
