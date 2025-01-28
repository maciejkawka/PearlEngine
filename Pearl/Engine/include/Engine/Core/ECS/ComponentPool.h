#pragma once
#include"Core/ECS/Defines.h"
#include"Core/ECS/BaseComponent.h"
#include"Core/Utils/NonCopyable.h"

#include<array>
#include<unordered_map>

namespace PrCore::ECS {

	class IComponentPool : public Utils::NonCopyable {
	public:
		virtual void EntityDestroyed(ID p_ID) = 0;

		virtual BaseComponent* GetRawData(ID p_ID) = 0;
	};

	template<class T>
	class ComponentPool: public IComponentPool {
	public:
		ComponentPool();
		~ComponentPool();

		T* AllocateData(ID p_ID);

		T* GetData(ID p_ID);

		void RemoveData(ID p_ID);

		bool DataExist(ID p_ID);

		void EntityDestroyed(ID p_ID) override;

		BaseComponent* GetRawData(ID p_ID) override;

	private:
		//Vector holds all one type components created
		std::array<T*, MAX_ENTITIES> m_components;
	};
}

#include"Core/ECS/ComponentPool.inl"
