#include "Core/Common/pearl_pch.h"

#include"Core/ECS/SceneManager.h"
#include"Core/Filesystem/FileSystem.h"
#include"Core/ECS/Scene.h"

using namespace PrCore::ECS;

SceneManager::~SceneManager()
{
	for (const auto& scene : m_scenes)
		delete scene;

	m_scenes.clear();
}

Scene* SceneManager::CreateScene(const std::string& p_name)
{
	auto scene = new Scene(p_name);

	std::string dir = SCENE_DIR;
	dir += "/" + p_name;
	scene->SetScenePath(dir);

	m_scenes.push_back(scene);

	return scene; 
}

void SceneManager::DeleteScene(const Scene* p_scene)
{
	const auto sceneIterator = std::find_if(m_scenes.begin(), m_scenes.end(), [&](const Scene* p_lambdaScene)
		{
			return p_lambdaScene->GetSceneUUID() == p_scene->GetSceneUUID();
		});

	PR_ASSERT(sceneIterator != m_scenes.end(), "Scene " + std::to_string(p_scene->GetSceneUUID()) + "does not exist");

	m_scenes.erase(sceneIterator);
	delete *sceneIterator;
	p_scene = nullptr;
}

Scene* SceneManager::GetScenebyName(const std::string& p_name)
{
	const auto sceneIterator = std::find_if(m_scenes.begin(), m_scenes.end(), [&](const Scene* p_scene)
		{
			return p_scene->GetSceneName() == p_name;
		});

	PR_ASSERT(sceneIterator != m_scenes.end(), "Scene " + p_name + "does not exist");

	return *sceneIterator;
}

Scene* SceneManager::GetScenebyUUID(Utils::UUID p_UUID)
{
	const auto sceneIterator = std::find_if(m_scenes.begin(), m_scenes.end(), [=](const Scene* p_scene)
		{
			return p_scene->GetSceneUUID() == p_UUID;
		});

	PR_ASSERT(sceneIterator != m_scenes.end(), "Scene " + std::to_string(p_UUID) + "does not exist");

	return *sceneIterator;
}

size_t SceneManager::GetSceneCount() const
{
	return m_scenes.size();
}

std::vector<Scene*> SceneManager::GetAllScenes()
{
	return m_scenes;
}
