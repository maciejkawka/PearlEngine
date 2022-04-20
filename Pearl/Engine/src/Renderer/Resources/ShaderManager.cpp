#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/ShaderManager.h"
#include"Renderer/OpenGL/GLShader.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

ShaderManager::ShaderManager()
{
	PRLOG_INFO("Init Shader Manager");
}

Resource* ShaderManager::CreateImpl(const std::string& p_name)
{
	return static_cast<OpenGL::GLShader*>(new OpenGL::GLShader(p_name, NextResourceHandle()));
}
