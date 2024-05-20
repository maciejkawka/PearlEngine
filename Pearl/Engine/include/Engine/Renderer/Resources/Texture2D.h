#pragma once
#include"Renderer/Resources/Texture.h"

namespace PrRenderer::Core {
	class Color;
}

namespace PrRenderer::Resources {

	class Texture2D : public Texture {
	public:
		Texture2D() = default;

		//Constructor for managed resource
		Texture2D(const std::string& p_name, PrCore::Resources::ResourceHandle p_handle) :
			Texture(p_name, p_handle)
		{}

		static std::shared_ptr<Texture2D> Create();
		static TexturePtr GenerateBlackTexture();
		static TexturePtr GenerateWhiteTexture();
		static TexturePtr GenerateRedTexture();

		//To implement in future
		//void SetPixel(unsigned int p_u, unsigned int p_v);
		//Core::Color GetPixel(unsigned int p_u, unsigned int p_v) const;

		virtual void SetData(void* p_data) {}

		//void SetPixels(const Core::Color p_colorArray[]);
		//const Core::Color* GetPixels();

	private:
		static TexturePtr GenerateUnitTexture(Core::Color p_color);

	};

	typedef std::shared_ptr<Texture2D> Texture2DPtr;
}