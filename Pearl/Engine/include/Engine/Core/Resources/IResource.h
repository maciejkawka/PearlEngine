#pragma once
#include<string>
#include<memory>

#include "Core/Utils/UUID.h"

namespace PrCore::Resources {

	using ResourceID = Utils::UUID;
	constexpr ResourceID InvalidID = 0;

	enum class ResourceState : uint8_t
	{
		// Standalone resource not tracked by ResourceSystem
		// This is default Resource state
		Unmanaged = 0,

		// Registered by a ResourceSystem and tracked in a shared pointer
		// Change made on this resource will be visible on all living references
		Registered = 1,

		// Loaded from file into memory and registered in ResourceSystem
		// Only resources created from file can have this state
		Loaded = 2,

		// Unloaded from the file but still tracked by a ResourceSystem, can be loaded later 
		// Only Resources created from file can have this state
		Unloaded = 3,

		// There was an attempt to load the resource but it failed
		// Only Resource created from file can have this state
		Corrupted = 4
	};

	/*
	=========== FILE RESOURCE LIFE CICLE ==================
	 ______________________________________________________
	| |---------|                                          |
	| |		    |                           -->            |
	| |Unmanaged| --> Registered --> Loaded <-- Unloaded   |
	| |		    |        |          Corrupted       |      |
	| |---------|  <---------------------------------      |
	|______________________________________________________|
	========================================================
	
	= MEMORY RESOURCE LIFE CICLE =
	 ____________________________
	| |---------|                |
	| |		    | <--            |
	| |Unmanaged| --> Registered |
	| |		    |				 |
	| |---------|                |
	 ____________________________|
	============================
	*/

	enum class ResourceOrigin : uint8_t
	{
		// Created and loaded from file can be unloaded or corrupted
		File = 0,

		// Created manually in a code lives in the memory only
		Memory = 1
	};


	// Base class for all Resource in the engine like Textures, Materials, Shaders
	class IResourceData {
	public:
		virtual ~IResourceData() = default;

		// Return size of loaded resource this should update during asset manipulations and calculate current size.
		virtual size_t GetByteSize() const = 0;

		// These functions allow adding resource name. It is used for only logging and debug purpose.
		// Name is automatically set by the IResourceDatabase but you can set your custom name too.
		const std::string& GetName() const { return m_name; }
		void               SetName(const std::string& p_name) { m_name = p_name; }
		
	protected:
		std::string m_name;
	};
	using IResourceDataPtr = std::shared_ptr<IResourceData>;


	// The Resource descriptor storing resource data and metadata
	// To access resource data call GetData() and cast to correct type
	struct ResourceDesc final {
	public:
		ResourceDesc() :
			data(nullptr),
			id(InvalidID),
			filePath(""),
			state(ResourceState::Unmanaged),
			origin(ResourceOrigin::Memory),
			size(0)
		{}

		ResourceDesc(IResourceDataPtr p_data) :
			data(p_data),
			id(InvalidID),
			filePath(""),
			state(ResourceState::Unmanaged),
			origin(ResourceOrigin::Memory),
			size(0)
		{}

		// Resource data 
		IResourceDataPtr data;

		// Size in bytes snapshot created during loading, it is not guaranteed to refresh if the resource data changes
		size_t          size;

		// Unique ID assigned by resource database
		ResourceID      id{};

		// Current resource state
		ResourceState   state;

		// Resource origin can be file or memory
		ResourceOrigin  origin;

		// If resource origin is file this stores the file path, empty otherwise
		std::string     filePath;

	};
	using ResourceDescPtr = std::shared_ptr<ResourceDesc>;
	using ResourceDescConstPtr = std::shared_ptr<const ResourceDesc>;

	// Resource descriptor proxy that automatically deduces the GetData type
	// It is main resource access object 
	// Manually created resources should also be assigned to this object
	template<class T>
	class ResourceHandle final {
	public:
		ResourceHandle() :
			m_resourceDesc(nullptr)
		{}

		ResourceHandle(const ResourceHandle<T>& p_ref) :
			m_resourceDesc(p_ref.m_resourceDesc)
		{}

		ResourceHandle(ResourceHandle<T>&& p_ref) :
			m_resourceDesc(p_ref.m_resourceDesc)
		{}

		ResourceHandle(const ResourceDescPtr& p_resourceDesc) :
			m_resourceDesc(p_resourceDesc)
		{}

		// Create ResourceHandle from the stack allocated ResourceDesc
		ResourceHandle(const ResourceDesc& p_resourceDesc) :
			m_resourceDesc(std::make_shared<ResourceDesc>(p_resourceDesc))
		{}

		// Create ResourceHandle from the stack allocated ResourceDesc 
		ResourceHandle(ResourceDesc&& p_resourceDesc) :
			m_resourceDesc(std::make_shared<ResourceDesc>(std::move(p_resourceDesc)))
		{}

		// Create ResourceHandle from resource data 
		// Useful when creating resource data manually and want to wrap with the handler
		// IMPORTANT! This constructor is implicit
		ResourceHandle(std::shared_ptr<T> p_dataPtr)
		{
			m_resourceDesc = std::make_shared<ResourceDesc>(p_dataPtr);
		}

		// Assign operators
		ResourceHandle<T>& operator=(const ResourceHandle<T>& p_ref)
		{
			if (this != &p_ref)
			{
				m_resourceDesc = p_ref.m_resourceDesc;
			}

			return *this;
		}

		ResourceHandle<T>& operator=(ResourceHandle<T>&& p_ref) noexcept
		{
			if (this != &p_ref)
			{
				m_resourceDesc = std::move(p_ref.m_resourceDesc);
			}

			return *this;
		}

		ResourceID          GetID() const { return m_resourceDesc->id; }
		const std::string&  GetPath() const { return m_resourceDesc->filePath; }
		ResourceState       GetState() const { return m_resourceDesc->state; }
		ResourceOrigin      GetOrigin() const { return m_resourceDesc->origin; }
		size_t              GetSize() const { return m_resourceDesc->size; }

		// Gets IResourceData from the descriptor. If descriptor is nullptr the function returns nullptr
		std::shared_ptr<T>  GetData() 
		{
			if (m_resourceDesc == nullptr)
				return nullptr;

			return std::static_pointer_cast<T>(m_resourceDesc->data); 
		}

		// Checks if the handle holds valid resource descriptor
		bool               IsValid() { return m_resourceDesc != nullptr; }

		// Proxy functions to access IResourceData directly
		// Use these functions instead of GetData()
		std::shared_ptr<T> operator->() { return GetData(); }
		bool               operator==(const IResourceDataPtr& p_ptr) const { return m_resourceDesc != nullptr && std::static_pointer_cast<const T>(m_resourceDesc->data) == p_ptr; }
		bool               operator!=(const IResourceDataPtr& p_ptr) const { return m_resourceDesc != nullptr && std::static_pointer_cast<const T>(m_resourceDesc->data) != p_ptr; }

	protected:
		ResourceDescPtr m_resourceDesc;
	};

#define REGISTRER_RESOURCE_HANDLE(ResourceName) \
	using ResourceName ## Handle = PrCore::Resources::ResourceHandle<ResourceName>
}
