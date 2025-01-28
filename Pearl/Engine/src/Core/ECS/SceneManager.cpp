#include "Core/Common/pearl_pch.h"

#include"Core/ECS/SceneManager.h"
#include"Core/File/FileSystem.h"
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
	delete p_scene;
	p_scene = nullptr;
}

Scene* SceneManager::LoadScene(const std::string& p_path)
{
	auto file = PrCore::File::FileSystem::GetInstance().OpenFileWrapper(p_path);
	
	PR_ASSERT(file != nullptr, "File cannot be opened! Path " + p_path);
	if (file == nullptr)
		return nullptr;

	size_t fileSize = file->GetSize();
	char* data = new char[fileSize];
	file->Read(data, fileSize);

	std::vector<uint8_t> dataVector;

	for (auto i = 0; i < fileSize; i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	auto sceneJSON = Utils::JSON::json::parse(dataVector);

	auto scene = CreateScene("");
	scene->OnDeserialize(sceneJSON);

	m_activeScene = scene;

	return scene;
}

void SceneManager::SaveSceneByName(const std::string& p_name, const std::string& p_path)
{
	auto scene = GetScenebyName(p_name);
	auto path = p_path.empty() ? scene->GetScenePath() : p_path;

	SaveScene(scene, path);
}

void SceneManager::SaveSceneByReference(Scene* p_scene, const std::string& p_path)
{
	PR_ASSERT(p_scene != nullptr, "Scene ptr == nullptr");

	auto path = p_path.empty() ? p_scene->GetScenePath() : p_path;

	SaveScene(p_scene, path);
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

Scene* SceneManager::GetActiveScene()
{
	return m_activeScene;
}

std::vector<Scene*> SceneManager::GetAllScenes()
{
	return m_scenes;
}

void SceneManager::SaveScene(Scene* p_scene, const std::string& p_path)
{
	PR_ASSERT(!p_path.empty(), "Scene path invalid " + p_path);

	Utils::JSON::json sceneJSON;
	p_scene->OnSerialize(sceneJSON);

	auto sceneStr = sceneJSON.dump(4);

	auto file = File::FileSystem::GetInstance().FileOpen(p_path, File::OpenMode::Write);
	File::FileSystem::GetInstance().FileWrite(file, sceneStr.c_str(), sceneStr.length());
	File::FileSystem::GetInstance().FileClose(file);
}
