#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/Material.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

MaterialManager::MaterialManager()
{
	PRLOG_INFO("Init {0}", typeid(MaterialManager).name());
}

Resource* MaterialManager::CreateImpl(const std::string& p_name)
{
	return static_cast<Resources::Material*>(new Material(p_name, NextResourceHandle()));
}