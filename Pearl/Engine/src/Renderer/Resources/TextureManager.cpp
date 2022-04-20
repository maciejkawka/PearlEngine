#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/TextureManager.h"
#include"Renderer/OpenGL/GLTexture2D.h"

using namespace PrRenderer::Resources;

PrCore::Resources::Resource* TextureManager::CreateImpl(const std::string& p_name)
{
	return static_cast<OpenGL::GLTexture2D*>(new OpenGL::GLTexture2D(p_name, NextResourceHandle()));
}

TextureManager::TextureManager()
{
	PRLOG_INFO("Init Texture Manager");
}
