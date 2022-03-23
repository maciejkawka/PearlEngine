#pragma once
#include"Renderer/Resources/Texture.h"

namespace PrRenderer::OpenGL {

	//Textures
	unsigned int TextureFormatToGL(Resources::TextureFormat p_format);
	unsigned int TextureFormatToInternalGL(Resources::TextureFormat p_format);
	unsigned int TextureWrapToGL(Resources::TextureWrapMode p_wrap);
	unsigned int TextureFilterToGL(Resources::TextureFiltering p_filter);



}