#pragma once
#include"Core/Resources/Resource.h"
#include"Core/Events/Event.h"
#include<string>
#include<map>
#include<list>
#include<unordered_map>


namespace PrCore::Resources {
	
	enum class CacheMiss {
		Compulsory,
		Capacity,
		Conflict
	};

	class ResourceManager {
	private:
		class LRU {
		public:
			LRU() {}
			~LRU() {}

			void UpdateLRU(ResourceHandle p_ID);
			void RemoveResource(ResourceHandle p_ID);
			ResourceHandle PopLeastUsed();

			void Clear();
			bool IsEmpty();

		private:
			std::list<ResourceHandle> m_LRU;
			std::unordered_map<ResourceHandle, std::list<ResourceHandle>::iterator> m_lruMap;
		};

		typedef std::map<size_t, ResourcePtr> ResourceIDMap;
		typedef std::map<std::string, ResourcePtr> ResourceNameMap;
		typedef std::list<ResourcePtr> ResourceList;

	public:
		ResourceManager();
		virtual ~ResourceManager();

		ResourcePtr Load(const std::string& p_name);
		ResourcePtr Reload(const std::string& p_name);
		void Unload(const std::string& p_name);
		void UnloadAll();

		void Delete(const std::string& p_name);
		void DeleteAll();

		inline size_t GetMaxMemory() { return m_maxMemory; }
		void  SetMaxMemory(size_t p_maxMemory);
		inline size_t GetMemoryUsage() { return m_memoryUsage; }

		ResourcePtr GetResource(const std::string& p_name);

		void OnResourceLoaded(PrCore::Events::EventPtr p_event);
		void OnResourceUnloaded(PrCore::Events::EventPtr p_event);

	protected:
		ResourceHandle ResNameToID(const std::string& p_name);
		std::string ResIDToName(ResourceHandle p_ID);

		ResourcePtr ResourceByName(const std::string& p_name);
		ResourcePtr ResourceByID(ResourceHandle p_ID);

		void MemoryCheck();
		void UpdateMemoryUsage();

		ResourcePtr CreateResource(const std::string& p_name);
		virtual Resource* CreateImpl(const std::string& p_name) = 0;
		virtual void DeleteImpl(ResourcePtr& p_resource);

		void FireCacheMiss(CacheMiss p_cacheMiss);

		inline static ResourceHandle NextResourceHandle() { return s_nextResourceID++; }

		ResourceIDMap m_resourceID;
		ResourceNameMap m_resourceName;
		ResourceList m_resources;
		LRU m_LRU;

		size_t m_maxMemory;
		size_t m_memoryUsage;

		static unsigned int s_nextResourceID;
	};
}