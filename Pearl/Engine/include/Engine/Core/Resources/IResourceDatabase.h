#pragma once

#include "IResource.h"
#include <functional>

namespace PrCore::Resources {

	class IResourceDataLoader;

	using ResourceVisitor = std::function<void(ResourceDescConstPtr)>;

	// IResourceDatabase interface that is responsible for storing and managing resources. 
	// Every resource type should have separate database instance
	// Insert resource into the Resource System
	class IResourceDatabase {
	public:
		virtual ~IResourceDatabase() = default;

		virtual ResourceDescPtr Load(const std::string& p_path, std::shared_ptr<IResourceDataLoader> p_loader = nullptr) = 0;
		virtual ResourceDescPtr Load(ResourceID p_id, std::shared_ptr<IResourceDataLoader> p_loader = nullptr) = 0;

		//IResourcePtr LoadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//IResourcePtr LoadAsync(ResourceID p_id);

		virtual void Unload(const std::string& p_path) = 0;
		virtual void Unload(ResourceID p_id) = 0;
		virtual void UnloadAll() = 0;

		//void UnloadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//void UnloadAsync(ResourceID p_id);

		// Assures that resource data is loaded and available
		virtual ResourceDescPtr Get(ResourceID p_id) = 0;
		virtual ResourceDescPtr Get(const std::string& p_path) = 0;

		// Does not assures that resource data is loaded and available.
		virtual ResourceDescPtr GetMetadata(ResourceID p_id) = 0;
		virtual ResourceDescPtr GetMetadata(const std::string& p_path) = 0;

		virtual ResourceDescPtr Register(IResourceDataPtr p_resourceData) = 0;
		virtual ResourceDescPtr Register(const std::string& p_path) = 0;

		virtual bool Remove(ResourceID p_id) = 0;
		virtual void RemoveAll() = 0;

		virtual void ForEachResource(ResourceVisitor p_visitor) = 0;

		virtual bool SaveToFile(ResourceID p_sourceId, const std::string& p_path) = 0;
		virtual ResourceDescPtr SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path) = 0;

		virtual size_t GetMemoryUsage() const = 0;
		virtual void   SetMemoryBudget(size_t p_budget) = 0;
		virtual size_t GetMemoryBudget() const = 0;

		virtual void                                  RegisterLoader(const std::string& p_fileExtension, std::unique_ptr<IResourceDataLoader> p_loader) = 0;
		virtual std::unique_ptr<IResourceDataLoader>& GetLoader(const std::string& p_fileExtension) = 0;
		virtual void                                  UnregisterLoader(const std::string& p_fileExtension) = 0;
		virtual void                                  UnregisterAllLoaders() = 0;
	};
}
