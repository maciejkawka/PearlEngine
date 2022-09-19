#pragma once
#include "Core/Utils/Singleton.h"
#include "Core/Utils/UUID.h"

namespace PrCore::ECS {

	class Scene;

	class SceneManager: public Utils::Singleton<SceneManager> {
	public:
		~SceneManager() override;

		Scene* CreateScene(const std::string& p_name);
		void DeleteScene(const Scene* p_scene);
		//Scene& LoadScene(const std::string& p_path);
		//void UnloadScene(const std::string& p_path);
		//void SaveScene(const std::string& p_path = "");

		Scene* GetScenebyName(const std::string& p_name);
		Scene* GetScenebyUUID(Utils::UUID p_UUID);

		size_t GetSceneCount() const;
		std::vector<Scene*> GetAllScenes();

	private:
		std::vector<Scene*> m_scenes;
	};
}