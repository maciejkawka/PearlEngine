#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/Material.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

MaterialManager::MaterialManager()
{
	PRLOG_INFO("Init Material Manager");
}

Resource* MaterialManager::CreateImpl(const std::string& p_name)
{
	return static_cast<Resources::Material*>(new Resources::Material(p_name, NextResourceHandle()));
}
