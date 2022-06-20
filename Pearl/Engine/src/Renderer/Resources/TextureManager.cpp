#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/TextureManager.h"
#include"Renderer/OpenGL/GLTexture2D.h"
#include"Renderer/OpenGL/GLCubemap.h"
#include<string>

using namespace PrRenderer::Resources;

PrCore::Resources::Resource* TextureManager::CreateImpl(const std::string& p_name)
{
	return new OpenGL::GLTexture2D(p_name, NextResourceHandle());
}

TextureManager::TextureManager()
{
	PRLOG_INFO("Init Texture Manager");
}
