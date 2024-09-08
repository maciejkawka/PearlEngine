#pragma once

#include "IResource.h"
#include "IResourceDataLoader.h"
#include "IResourceDatabase.h"

namespace PrCore::Resources {

	class ResourceSystem {
	public:
		ResourceSystem() = default;

		//-----------------------------------------------------------------------------
		// Loads Resource into memory, only file originated resource can be loaded
		// Resources created manually in code should be Registered with IResourceDataPtr only
		// Pass p_loader when want to load resource differently. This loader is bound to the resource Id until it is Removed from the database.
		template<class T>
		Resourcev2<T> Load(const std::string& p_path, std::shared_ptr<IResourceDataLoader> p_loader = nullptr);

		template<class T>
		Resourcev2<T> Load(ResourceID p_id, std::shared_ptr<IResourceDataLoader> p_loader = nullptr);

		////Implement Later
		////IResource LoadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		////IResource LoadAsync(ResourceID p_id);


		//-----------------------------------------------------------------------------
		// Unloads Resource from memory, only file originated resource can be unloaded. Resource can be loaded later
		// Unloads with default extension loader or uses custom loader pass during loading
		// Resources created manually in code cannot be unloaded and should be removed
		template<class T>
		void Unload(const std::string& p_path);

		template<class T>
		void Unload(ResourceID p_id);

		template<class T>
		void UnloadAll();

		//Implement Later
		//void UnloadAsync(const std::string p_path, const IResourceLoader* p_loader = nullptr);
		//void UnloadAsync(ResourceID p_id);


		//-----------------------------------------------------------------------------
		// Gets the registered resource, loads if it is unloaded and fires CacheMiss event
		// If the resource is fetched by this function and then removed from database with Remove() client is owner of the IResourceData
		// IMPORTANT!! Avoid dangling shared pointers to the IResourceData, this prevents memory deallocations and might cause the memory usage increase
		template<class T>
		Resourcev2<T> Get(ResourceID p_id);

		template<class T>
		Resourcev2<T> Get(const std::string& p_path);


		//-----------------------------------------------------------------------------
		// Gets the ResourceMetadata
		// Use to check resource metadata without fetching the ptr.
		template<class T>
		Resourcev2<T> GetMetadata(ResourceID p_id);

		template<class T>
		Resourcev2<T> GetMetadata(const std::string& p_path);


		//-----------------------------------------------------------------------------
		// Registers a new Resource in the database, the resource is Managed by the system after
		// For memory originated resource the resource data ownership goes to the resource database and will be released after removing 
		// For file originated resource the resource will be registered but not loaded. Use Load to load file into the memory
		template<class T>
		Resourcev2<T> Register(IResourceDataPtr p_resourceData);

		template<class T>
		Resourcev2<T> Register(const std::string& p_path);


		//-----------------------------------------------------------------------------
		// Removes the resource from the database. The resource ownership goes to the client that stores data pointer
		// The IResource ptr stored by client will lose access to the data ptr, copy the data pointer if the untracked memory is important.
		// IMPORTANT!! Avoid dangling shared pointers to the IResourceData, this prevents memory deallocations and might cause the memory usage increase
		template<class T>
		bool Remove(ResourceID p_id);

		template<class T>
		void RemoveAll();


		//-----------------------------------------------------------------------------
		// Visitor pattern to allow iterating and gathering data from all resources in the database
		template<class T>
		void ForEachResource(ResourceVisitor p_visitor);


		//-----------------------------------------------------------------------------
		// Saves resource to the file if the p_path loader is registered or loaded with custom one, will assert otherwise
		// File and Memory resources can be saved into the memory if the correct loader is registered for the file extension
		template<class T>
		bool SaveToFile(ResourceID p_sourceId, const std::string& p_path);

		template<class T>
		Resourcev2<T> SaveToFileAndLoad(ResourceID p_sourceId, const std::string& p_path);


		//-----------------------------------------------------------------------------
		// Copies the IResourceData and returns the pointer it does not register the data into the database
		// The data has to be registered manually by callig Register()
		template<class T>
		std::shared_ptr<T> Copy(ResourceID p_sourceId);

		template<class T>
		std::shared_ptr<T> Copy(const std::string& p_path);


		//-----------------------------------------------------------------------------
		// Gets resource database memory usage and sets budgets
		template<class T>
		size_t GetMemoryUsage() const;

		template<class T>
		void   SetMemoryBudget(size_t p_budget);

		template<class T>
		size_t GetMemoryBudget() const;


		//-----------------------------------------------------------------------------
		//Registers the loaders
		template<class T>
		void RegisterLoader(const std::string& p_fileExtension, IResourceDataLoader* p_loader);

		template<class T>
		void UnregisterLoader(const std::string& p_fileExtension);

		template<class T>
		void UnregisterAllLoaders();


		//-----------------------------------------------------------------------------
		//Registers the resource database associated with the IResourceData type.
		template<class T>
		void RegisterDatabase(std::unique_ptr<IResourceDatabase> p_database);

		template<class T>
		void UnregisterDatabase();

		void UnregisterDatabasesAll();


	private:
		template<class T>
		std::unique_ptr<IResourceDatabase>& GetResourceDatabase();

		using ResourceDatabaseTypeMap = std::map<size_t, std::unique_ptr<IResourceDatabase>>;

		ResourceDatabaseTypeMap m_resourceDatabaseTypes;
	};
}

#include "ResourceSystem.inl"