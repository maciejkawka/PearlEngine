#pragma once
#include<string>

#include"Core/ECS/EntityManager.h"
#include"Core/Utils/UUID.h"
#include<queue>

namespace PrCore::ECS {

	class SystemManager;
	class EntityManager;

	class Scene {
	public:
		Scene(const std::string& p_name);
		//Scene(const std::string& p_name, const std::string& p_path) {}

		~Scene();

		Entity CreateEntity(const std::string& p_name);
		void DestoryEntity(Entity p_entity);
		void DestoryEntityImmediate(Entity p_entity);

		template<class System>
		void RegisterSystem();

		template<class System>
		void SetActiveSystem(bool p_isActive);

		template<class System>
		bool IsActiveSystem();

		Entity GetEntityByName(const std::string& p_name);
		Entity GetEntityByID(Utils::UUID p_UUID);
		Entity GetEntityByTag(const std::string& p_tag);

		//Updates
		void Update(float p_dt) const;
		void FixUpdate(float p_dt) const;
		void LateUpdate(float p_dt) const;
		void CleanDestroyedEntities();

		//void RenderUpdate(float p_dt) const;
		//void PhysicsUpdate(float p_dt) const;

		//Getters and Setters
		inline std::string GetSceneName() const { return m_name; }
		inline std::string GetScenePath() const { return m_path; }
		inline Utils::UUID GetSceneUUID() const { return m_UUID; }

		inline void SetSceneName(const std::string& p_name) { m_name = p_name; }
		inline void SetScenePath(const std::string& p_path) { m_path = p_path; }

		size_t GetEntitiesCount();

	private:
		Utils::UUID m_UUID;
		std::string m_name;
		std::string m_path;

		SystemManager* m_systemManager;
		EntityManager* m_entityManager;

		std::queue<Entity> m_entitiesToDestory;
	};
}


#include"Core/ECS/Scene.inl"