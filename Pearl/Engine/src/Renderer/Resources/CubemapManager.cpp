#include"Core/Common/pearl_pch.h"
#include"Renderer/Resources/CubemapManager.h"
#include"Renderer/OpenGL/GLCubemap.h"

using namespace PrRenderer::Resources;

CubemapManager::CubemapManager()
{
	PRLOG_INFO("Init {0}", typeid(CubemapManager).name());
}

PrCore::Resources::Resource* CubemapManager::CreateImpl(const std::string& p_name)
{
	return static_cast<Cubemap*>(new OpenGL::GLCubemap(p_name, NextResourceHandle()));
}