#pragma once
#include"Renderer/Resources/Texture.h"

namespace PrRenderer::Core {
	class Color;
}

namespace PrRenderer::Resources {
	
	class Texture2D : public Texture {
	public:
		Texture2D() = default;
		Texture2D(const std::string& p_name, PrCore::Resources::ResourceID p_ID) :
			Texture(p_name, p_ID)
		{}

		//To implement in future
		//void SetPixel(unsigned int p_u, unsigned int p_v);
		//Core::Color GetPixel(unsigned int p_u, unsigned int p_v) const;

		//void SetPixels(const Core::Color p_colorArray[]);
		//const Core::Color* GetPixels();
	};

	typedef std::shared_ptr<Texture2D> Texture2DPtr;
}
