#pragma once
#include <functional>
#include <map>

#include "IResource.h"

namespace PrCore::Resources {

	class IResourceDataLoader;

	using ResourceVisitor = std::function<void(ResourceDescConstPtr)>;

	class ResourceDatabase final {
	public:
		ResourceDatabase() :
			m_memoryUsage(0),
			m_memoryBudget(0)
		{}

		ResourceDescPtr Load(const std::string& p_path, IResourceDataLoader* p_loader = nullptr);
		ResourceDescPtr Load(ResourceID p_id, IResourceDataLoader* p_loader = nullptr);

		//IResourcePtr LoadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//IResourcePtr LoadAsync(ResourceID p_id);

		void Unload(const std::string& p_path, IResourceDataLoader* p_loader = nullptr);
		void Unload(ResourceID p_id, IResourceDataLoader* p_loader = nullptr);
		void UnloadAll();

		//void UnloadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//void UnloadAsync(ResourceID p_id);

		// Assures that resource data is loaded and available
		ResourceDescPtr Get(ResourceID p_id);
		ResourceDescPtr Get(const std::string& p_path);

		// Does not assures that resource data is loaded and available.
		// Fetches only current descriptor and returns copy without data ptr 
		ResourceDesc GetMetadata(ResourceID p_id);
		ResourceDesc GetMetadata(const std::string& p_path);

		ResourceDescPtr Register(IResourceDataPtr p_resourceData, size_t p_size = 0);
		ResourceDescPtr Register(const std::string& p_path);

		bool Remove(ResourceID p_id);
		void RemoveAll();

		void ForEachResource(ResourceVisitor p_visitor);

		bool SaveToFile(ResourceID p_sourceId, const std::string& p_path);
		ResourceDescPtr SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path);

		size_t GetMemoryUsage() const { return m_memoryUsage; }
		void   SetMemoryBudget(size_t p_budget) { m_memoryBudget = p_budget; }
		size_t GetMemoryBudget() const { return m_memoryBudget; }

		void                 RegisterLoader(const std::string& p_fileExtension, IResourceDataLoader* p_loader);
		IResourceDataLoader* GetLoader(const std::string& p_fileExtension);
		void                 UnregisterLoader(const std::string& p_fileExtension);
		void                 UnregisterAllLoaders();

	private:
		bool LoadResourcePrivate(ResourceDescPtr p_resourceDesc, IResourceDataLoader* p_loader = nullptr);
		void UnloadResourcePrivate(ResourceDescPtr p_resourceDesc, IResourceDataLoader* p_loader = nullptr);

		//ResourceID RegisterResourcePrivate(ResourceDescPtr p_resource);
		ResourceDescPtr RegisterFileResourcePrivate(const std::string& p_path);
		ResourceDescPtr RegisterMemoryResourcePrivate(IResourceDataPtr p_resourceData, size_t p_size = 0);

		ResourceDescPtr ResourceByID(ResourceID p_id);
		ResourceDescPtr ResourceByPath(const std::string& p_path);

		void CheckMemoryBudget(ResourceDescPtr p_resource);

		void FireUnloadedEvent(ResourceID p_id, const std::string& p_path);
		void FireLoadedEvent(ResourceID p_id, const std::string& p_path);
		void FireCorruptedEvent(ResourceID p_id, const std::string& p_path);

		void FireCacheMiss(ResourceID p_id, const std::string& p_path);
		void FireBudgetExceeded(ResourceID p_id, const std::string& p_path, size_t p_usage, size_t p_budget);

		using IDMap = std::map<ResourceID, ResourceDescPtr>;
		using PathMap = std::map<std::string, ResourceDescPtr>;
		using LoaderMap = std::map<std::string, IResourceDataLoader*>;
		using CustomLoaderMap = std::map<ResourceID, IResourceDataLoader*>;

		IDMap m_resourcesID;
		PathMap m_resourcesPaths;
		LoaderMap m_loaders;
		CustomLoaderMap m_customLoaders;

		size_t m_memoryUsage;
		size_t m_memoryBudget;
	};
}