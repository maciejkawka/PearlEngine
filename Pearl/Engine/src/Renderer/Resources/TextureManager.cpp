#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/TextureManager.h"
#include"Renderer/OpenGL/GLTexture2D.h"

using namespace PrRenderer::Resources;

TextureManager* TextureManager::m_instance = nullptr;

TextureManager& PrRenderer::Resources::TextureManager::GetInstance()
{
	if (m_instance == nullptr)
		m_instance = new TextureManager();

	return *m_instance;
}

void TextureManager::Terminate()
{
	if (m_instance)
		delete m_instance;
}

PrCore::Resources::Resources* TextureManager::CreateImpl(const std::string& p_name)
{
	return static_cast<OpenGL::GLTexture2D*>(new OpenGL::GLTexture2D(p_name, NextResourceID()));
}

TextureManager::TextureManager()
{
	PRLOG_INFO("Init Texture Manager");
}
