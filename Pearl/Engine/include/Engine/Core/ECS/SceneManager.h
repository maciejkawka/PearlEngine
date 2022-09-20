#pragma once
#include "Core/Utils/Singleton.h"
#include "Core/Utils/UUID.h"

namespace PrCore::ECS {

	class Scene;

	class SceneManager: public Utils::Singleton<SceneManager> {
	public:
		~SceneManager() override;

		Scene* CreateScene(const std::string& p_name = "Scene");
		void DeleteScene(const Scene* p_scene);

		Scene* LoadScene(const std::string& p_path);

		void SaveSceneByName(const std::string& p_name, const std::string& p_path = "");
		void SaveSceneByReference(Scene* p_scene, const std::string& p_path = "");

		Scene* GetScenebyName(const std::string& p_name);
		Scene* GetScenebyUUID(Utils::UUID p_UUID);

		size_t GetSceneCount() const;
		std::vector<Scene*> GetAllScenes();

	private:
		void SaveScene(Scene* p_scene, const std::string& p_path);

		std::vector<Scene*> m_scenes;
	};
}