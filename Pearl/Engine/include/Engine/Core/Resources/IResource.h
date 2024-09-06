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

		// Resource data 
		IResourceDataPtr data;

		// Size in bytes snapshot created during loading, it is not refreshed if the resource data changes
		size_t          size;

		// Unique ID assigned by resource database
		ResourceID      id{};

		// Current resource state
		ResourceState   state;

		// Resource origin can be file or memory
		ResourceOrigin  origin;

		// If resource origin is file this stores the file path
		std::string     filePath;

	};
	using ResourceDescPtr = std::shared_ptr<ResourceDesc>;
	using ResourceDescConstPtr = std::shared_ptr<const ResourceDesc>;

	// Resource descriptor wrapper that automatically deduces the GetData type
	// It is main resource access object
	template<class T>
	class Resourcev2 final {
	public:
		Resourcev2(const ResourceDescPtr& p_resourceDesc) :
			m_resourceDesc(p_resourceDesc)
		{}

		ResourceID          GetID() const { return m_resourceDesc->id; }
		const std::string&  GetPath() const { return m_resourceDesc->filePath; }
		ResourceState       GetState() const { return m_resourceDesc->state; }
		ResourceOrigin      GetSource() const { return m_resourceDesc->origin; }
		size_t              GetSize() const { return m_resourceDesc->size; }
		std::shared_ptr<T>  GetData() { return std::static_pointer_cast<T>(m_resourceDesc->data); }

	protected:
		const ResourceDescPtr m_resourceDesc;
	};

#define REGISTRER_RESOURCE_TYPE(ResourceName) \
	using ResourceName ## Handle = Resourcev2<ResourceName>

	// Example usage
	//
	//class Texture : public IResourceData
	// using TextureResource = Resourcev2<Texture>;
	// 
	// TextureResource textureResource = ResourceSystem::Get<T>("TestPath/Elo.png");
	// Texture texture = textureResource->GetData();
}
