#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Texture2D.h"
#include "Renderer/OpenGL/GLTexture2D.h"
#include "Renderer/Core/RendererAPI.h"

using namespace PrRenderer::Resources;

TexturePtr Texture2D::GenerateUnitTexture(PrRenderer::Core::Color p_color)
{
	Resources::Texture2DPtr texture;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLTexture2D>(); break;

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		break;
	}
	}

	texture->LoadUnitTexture(p_color);

	return std::static_pointer_cast<Texture>(texture);
}

TexturePtr Texture2D::GenerateBlackTexture()
{
	return GenerateUnitTexture(Core::Color::Black);
}

TexturePtr Texture2D::GenerateWhiteTexture()
{
	return GenerateUnitTexture(Core::Color::White);
}

TexturePtr Texture2D::GenerateRedTexture()
{
	return GenerateUnitTexture(Core::Color::Red);
}
