#pragma once
#include<cstdint>

namespace PrCore::ECS {

	class EntityManager;

	constexpr unsigned int MAX_ENTITIES = 10000;
	constexpr unsigned int MAX_COMPONENTS = 64;


	//ID wrapps version and Index
	//       ________________________
	//		|		ID STRUCTURE	 |
	//      |---------64 bit---------|
	//      |  32 bit   |   32 bit   |
	//		|  INDEX    |   VERSION  |
	//      |________________________|
	struct ID
	{
		ID();
		explicit ID(uint64_t p_ID);
		explicit ID(uint32_t p_index, uint32_t p_version);

		ID(const ID& p_ID) = default;
		ID& operator = (const ID& p_ID) = default;

		inline uint32_t GetIndex() const { return m_ID >> 32; }
		inline uint32_t GetVersion() const { return uint32_t(m_ID); }
		inline uint32_t GetID() const { return m_ID; }

		inline bool IsValid() const { return m_ID >> 32 != 0; }

		inline bool operator ==(const ID& p_ID) const { return p_ID.m_ID == m_ID; }
		inline bool operator !=(const ID& p_ID) const { return p_ID.m_ID != m_ID; }

	private:
		uint64_t m_ID;

#ifdef _DEBUG
		uint32_t DEBUG_INDEX;
		uint32_t DEBUG_VERSION;
#endif
	};
	
	const ID INVALID_ID;

	class Entity {


	public:
		Entity() = delete;
		Entity(ID p_ID, EntityManager* p_entityManager): m_ID(p_ID), m_entityManager(p_entityManager) {} 
		Entity(const Entity& other) = default;
		Entity& operator = (const Entity & other) = default;

		inline ID GetID() const { return m_ID; }

		void Destroy();
		bool IsValid();

		//Components
		//template<class T>
		//T* AddComponent();

		//template<class T>
		//T* GetComponent();

		//template<class T>
		//void RemoveComponent();

		//template<class T>
		//bool HasComponent();

	private:
		void Invalidate();

		ID m_ID;
		EntityManager* m_entityManager;
	}; 
}


namespace std {

	template<>
	struct hash<PrCore::ECS::ID>
	{
		inline size_t operator()(const PrCore::ECS::ID& p_ID) const {
			return p_ID.GetID();
		}
	};
}

//#include "Core/ECS/EntityManager.inl"