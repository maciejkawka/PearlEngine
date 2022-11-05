#include"Core/Common/pearl_pch.h"
#include"Renderer/Resources/MeshManager.h"
#include"Renderer/OpenGL/GLMesh.h"

using namespace PrRenderer::Resources;

MeshManager::MeshManager()
{
	PRLOG_INFO("Init {0}", typeid(MeshManager).name());
}

PrCore::Resources::Resource* MeshManager::CreateImpl(const std::string& p_name)
{
	return static_cast<Resources::Mesh*>(new OpenGL::GLMesh(p_name, NextResourceHandle()));
}