#include "Editor/Core/EditorContext.h"
#include "Editor/Assets/Model/ModelResource.h"
#include "Editor/Assets/Model/ModelResourceLoader.h"

#include "Engine/Core/Resources/ResourceDatabase.h"
#include "Engine/Core/Utils/Logger.h"

using namespace PrEditor::Core;
EditorContext::EditorContext():
	AppContext()
{
	PRLOG_INFO("Building EditorContext");

	// Register Editor Resources
	auto modelDatabase = std::make_unique<PrCore::Resources::ResourceDatabase>();
	modelDatabase->RegisterLoader(".glb", std::make_unique<Assets::ModelResourceLoader>());
	modelDatabase->RegisterLoader(".fbx", std::make_unique<Assets::ModelResourceLoader>());
	modelDatabase->RegisterLoader(".gltf", std::make_unique<Assets::ModelResourceLoader>());
	PrCore::Resources::ResourceSystem::GetInstance().RegisterDatabase<Assets::ModelResource>(std::move(modelDatabase));


}

EditorContext::~EditorContext()
{
	PRLOG_INFO("Deleting EditorContext");

	PrCore::Resources::ResourceSystem::GetInstance().UnregisterDatabase<Assets::ModelResource>();
}
