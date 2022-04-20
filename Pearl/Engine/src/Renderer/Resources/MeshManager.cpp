#include"Core/Common/pearl_pch.h"
#include"Renderer/Resources/MeshManager.h"
#include"Renderer/OpenGL/GLMesh.h"

using namespace PrRenderer::Resources;

MeshManager* MeshManager::s_instance = nullptr;

MeshManager::MeshManager()
{
	PRLOG_INFO("Init Mesh Manager");
}

PrCore::Resources::Resource* MeshManager::CreateImpl(const std::string& p_name)
{
	return static_cast<Resources::Mesh*>(new OpenGL::GLMesh(p_name, NextResourceHandle()));
}

MeshManager& MeshManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new MeshManager();

	return *s_instance;
}

void MeshManager::Terminate()
{
	if (s_instance)
		delete s_instance;
}