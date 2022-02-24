#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/OpenGL/GLShader.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

ShaderManager* ShaderManager::m_instance = nullptr;

ShaderManager::ShaderManager()
{
	PRLOG_INFO("Init Shader Manager");
}

Resources* ShaderManager::CreateImpl(const std::string& p_name)
{
	return static_cast<OpenGL::GLShader*>(new OpenGL::GLShader(p_name, NextResourceID()));
}

ShaderManager& PrRenderer::Resources::ShaderManager::GetInstance()
{
	if (m_instance == nullptr)
		m_instance = new ShaderManager();
	
	return *m_instance;
}

void ShaderManager::Terminate()
{
	if (m_instance)
		delete m_instance;
}
