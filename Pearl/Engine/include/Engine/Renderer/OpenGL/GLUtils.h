#pragma once
#include"Renderer/Resources/Texture.h"
#include"Renderer/Buffers/Framebuffer.h"
#include"Renderer/Core/RendererAPI.h"

namespace PrRenderer::OpenGL {

	//Textures
	unsigned int TextureFormatToGL(Resources::TextureFormat p_format);
	unsigned int TextureFormatToInternalGL(Resources::TextureFormat p_format);
	unsigned int TextureWrapToGL(Resources::TextureWrapMode p_wrap);
	unsigned int TextureFilterToGL(Resources::TextureFiltering p_filter);
	unsigned int TextureFormatToDataTypeGL(Resources::TextureFormat p_format);

	//Buffers
	unsigned int FramebufferMaskToGL(Buffers::FramebufferMask p_mask);

	//Renderer
	unsigned int PrimitiveToGL(Core::Primitives p_primitives);
	unsigned int ComparaisonToGL(Core::ComparaisonAlgorithm p_algorythm);
	unsigned int BlendingToGL(Core::BlendingAlgorithm p_algorythm);
	unsigned int CullFaceToGL(Core::CullFaceMode p_mode);

	//General
	unsigned int BoolToGL(bool p_bool);
}