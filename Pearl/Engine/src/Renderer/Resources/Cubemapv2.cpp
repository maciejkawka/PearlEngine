#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Cubemapv2.h"
#include "Renderer/OpenGL/GLCubemapv2.h"
#include "Renderer/Core/RendererAPI.h"

using namespace PrRenderer::Resources;

std::shared_ptr<Cubemapv2> Cubemapv2::Create()
{
	Cubemapv2Ptr texture;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLCubemapv2>(); break;

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		break;
	}
	}

	return texture;
}

std::shared_ptr<PrRenderer::Resources::Cubemapv2> Cubemapv2::Create(RendererID p_id, size_t p_width, size_t p_height, Resources::TextureFormat p_format)
{
	Cubemapv2Ptr texture;
	switch (Core::RendererAPI::GetGraphicsAPI())
	{
	case Core::GraphicsAPI::OpenGL: texture = std::make_shared<OpenGL::GLCubemapv2>(p_id, p_width, p_height, p_format); break;

	default:
	{
		PRLOG_ERROR("No such Graphics API");
		break;
	}
	}

	return texture;
}

std::shared_ptr<Cubemapv2> Cubemapv2::CreateUnitTex(const Core::Color& p_unitColor)
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

void Cubemapv2::SetData(void* p_data)
{
	m_rawDataArray = static_cast<void**>(p_data);
}
