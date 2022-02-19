#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/MaterialManager.h"
#include"Renderer/Resources/Material.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

MaterialManager* MaterialManager::m_instance = nullptr;

MaterialManager::MaterialManager()
{
	PRLOG_INFO("Init Material Manager");
}

Resources* MaterialManager::CreateImpl(const std::string& p_name)
{
	return static_cast<Resources::Material*>(new Resources::Material(p_name, this, NextResourceID()));
}

MaterialManager& PrRenderer::Resources::MaterialManager::GetInstance()
{
	if (m_instance == nullptr)
		m_instance = new MaterialManager();

	return *m_instance;
}

void MaterialManager::Terminate()
{
	if (m_instance)
		delete m_instance;
}
