#pragma once
#include"Core/ECS/Entity.h"
#include"Core/Utils/NonCopyable.h"

#include<array>
#include<unordered_map>

namespace PrCore::ECS {

	class BaseComponent;


	class IComponentPool : public Utils::NonCopyable {

		//Empty class that helps storing pools in array
	};

	template<class T>
	class ComponentPool: public IComponentPool {
	public:
		ComponentPool();

		T* AllocateData(ID p_ID);

		T* GetData(ID p_ID);

		void RemoveData(ID p_ID);

		bool DataExist(ID p_ID);

	private:
		//Vector holds all one type components created
		//Component vector is packed
		//Vector capacity would be set as quarter of max entities at the beginig
		std::array<T, MAX_ENTITIES> m_components;

		//Map holds Entites to index references
		std::unordered_map<ID, size_t> m_entityToIndexMap;

		//Map holds index to Entity references
		std::unordered_map<size_t, ID> m_indexToEntityMap;

		//Number of all components
		size_t m_componentsNumber;

		
	};

}

#include"Core/ECS/ComponentPool.inl"