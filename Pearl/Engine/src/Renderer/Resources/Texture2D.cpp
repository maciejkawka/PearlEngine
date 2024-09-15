#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Texture2D.h"
#include "Renderer/OpenGL/GLTexture2D.h"
#include "Renderer/Core/RendererAPI.h"

using namespace PrRenderer::Resources;

std::shared_ptr<Texture2D> Texture2D::Create()
{
	Texture2Dv2Ptr texture = nullptr;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLTexture2D>(); break;

	default:
	{
		PR_ASSERT(false, "No such Graphics API");
		break;
	}
	}

	return texture;
}

std::shared_ptr<Texture2D> Texture2D::CreateUnitTex(const Core::Color& p_unitColor)
{
	// Create empty texture and fills it with unit data
	auto texture = Create();

	unsigned char* rawImage = new unsigned char[4];
	rawImage[0] = p_unitColor.r;
	rawImage[1] = p_unitColor.g;
	rawImage[2] = p_unitColor.b;
	rawImage[3] = p_unitColor.a;

	texture->SetData(rawImage);
	texture->SetFormat(TextureFormat::RGBA32);
	texture->SetHeight(1);
	texture->SetWidth(1);
	texture->SetReadable(false);
	texture->SetMipMap(false);

	texture->Apply();
	texture->SetData(nullptr);

	delete[] rawImage;

	return texture;
}
