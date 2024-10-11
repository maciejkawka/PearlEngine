#pragma once
#include <functional>
#include <map>

#include "IResource.h"
#include "IResourceDatabase.h"

namespace PrCore::Resources {

	class ResourceDatabase : public IResourceDatabase {
	public:
		ResourceDatabase() :
			m_memoryUsage(0),
			m_memoryBudget(0)
		{}

		~ResourceDatabase() override;

		ResourceDescPtr Load(const std::string& p_path, std::shared_ptr<IResourceDataLoader> p_loader = nullptr) override;
		ResourceDescPtr Load(ResourceID p_id, std::shared_ptr<IResourceDataLoader> p_loader = nullptr) override;

		//IResourcePtr LoadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//IResourcePtr LoadAsync(ResourceID p_id);

		void Unload(const std::string& p_path) override;
		void Unload(ResourceID p_id) override;
		void UnloadAll() override;

		//void UnloadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//void UnloadAsync(ResourceID p_id);

		// Assures that resource data is loaded and available
		ResourceDescPtr Get(ResourceID p_id) override;
		ResourceDescPtr Get(const std::string& p_path) override;

		// Does not assures that resource data is loaded and available.
		ResourceDescPtr GetMetadata(ResourceID p_id) override;
		ResourceDescPtr GetMetadata(const std::string& p_path) override;

		ResourceDescPtr Register(IResourceDataPtr p_resourceData) override;
		ResourceDescPtr Register(const std::string& p_path) override;

		bool Remove(ResourceID p_id) override;
		void RemoveAll() override;

		void ForEachResource(ResourceVisitor p_visitor) override;

		bool SaveToFile(ResourceID p_sourceId, const std::string& p_path) override;
		ResourceDescPtr SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path) override;

		size_t GetMemoryUsage() const  override { return m_memoryUsage; }
		void   SetMemoryBudget(size_t p_budget) override { m_memoryBudget = p_budget; }
		size_t GetMemoryBudget() const override { return m_memoryBudget; }

		void                  RegisterLoader(const std::string& p_fileExtension, std::unique_ptr<IResourceDataLoader> p_loader) override;
		IResourceDataLoader*  GetLoader(const std::string& p_fileExtension) override;
		void                  UnregisterLoader(const std::string& p_fileExtension) override;
		void                  UnregisterAllLoaders() override;

	private:
		bool LoadResourcePrivate(const ResourceDescPtr& p_resourceDesc, std::shared_ptr<IResourceDataLoader> p_loader = nullptr);
		void UnloadResourcePrivate(const ResourceDescPtr& p_resourceDesc);

		ResourceDescPtr RegisterFileResourcePrivate(const std::string& p_path);
		ResourceDescPtr RegisterMemoryResourcePrivate(const IResourceDataPtr& p_resourceData);

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
		using LoaderMap = std::map<std::string, std::unique_ptr<IResourceDataLoader>>;
		using CustomLoaderMap = std::map<ResourceID, std::shared_ptr<IResourceDataLoader>>;

		IDMap m_resourcesID;
		PathMap m_resourcesPaths;
		LoaderMap m_loaders;
		CustomLoaderMap m_customLoaders;

		size_t m_memoryUsage;
		size_t m_memoryBudget;
	};
}
