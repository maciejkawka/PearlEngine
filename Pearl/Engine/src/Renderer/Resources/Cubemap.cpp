#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Cubemap.h"
#include"Renderer/OpenGL/GLCubemap.h"

#include "Renderer/Core/RendererAPI.h"

using namespace PrRenderer::Resources;

TexturePtr Cubemap::GenerateBlackTexture()
{
	return GenerateUnitTexture(Core::Color::Black);
}

TexturePtr Cubemap::GenerateWhiteTexture()
{
	return GenerateUnitTexture(Core::Color::White);
}

TexturePtr Cubemap::GenerateRedTexture()
{
	return GenerateUnitTexture(Core::Color::Red);
}

TexturePtr Cubemap::GenerateUnitTexture(Core::Color p_color)
{
	Resources::CubemapPtr texture;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLCubemap>(); break;

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		break;
	}
	}

	texture->LoadUnitTexture(p_color);

	return std::static_pointer_cast<Texture>(texture);
}
