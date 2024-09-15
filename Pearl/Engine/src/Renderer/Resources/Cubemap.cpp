#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Cubemap.h"
#include "Renderer/OpenGL/GLCubemap.h"
#include "Renderer/Core/RendererAPI.h"

using namespace PrRenderer::Resources;

std::shared_ptr<Cubemap> Cubemap::Create()
{
	CubemapPtr texture;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLCubemap>(); break;

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		break;
	}
	}

	return texture;
}

std::shared_ptr<PrRenderer::Resources::Cubemap> Cubemap::Create(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format)
{
	CubemapPtr texture;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLCubemap>(p_id, p_width, p_height, p_format); break;

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		break;
	}
	}

	return texture;
}

std::shared_ptr<Cubemap> Cubemap::CreateUnitTex(const Core::Color& p_unitColor)
{
	// Create empty texture and fills it with unit data
	auto texture = Create();

	unsigned char* rawImage = new unsigned char[4];

	//Pink color
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

void Cubemap::SetData(void* p_data)
{
	m_rawDataArray = static_cast<void**>(p_data);
}
