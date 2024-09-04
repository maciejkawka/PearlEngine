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

	enum class ResourceSource : uint8_t
	{
		// Created and loaded from file can be unloaded or corrupted
		FromFile = 0,

		// Created manually in a code lives in the memory only
		FromMemory = 1
	};

	// The place holder to store resource data only 
	// Overrides along with IResourceDesc to implement new Resource type
	class IResourceData {
	public:
		virtual ~IResourceData() = default;
	};
	using IResourceDataPtr = std::shared_ptr<IResourceData>;

	// The Resource descriptor storing resource data and metadata
	// To access resource data call GetData()
	class ResourceDesc {
	public:
		ResourceDesc() :
			m_data(nullptr),
			m_ID(InvalidID),
			m_path(""),
			m_state(ResourceState::Unmanaged),
			m_source(ResourceSource::FromMemory),
			m_size(0)
		{}

		ResourceID          GetID() const { return m_ID; }
		const std::string&  GetPath() const { return m_path; }
		ResourceState       GetState() const { return m_state; }
		ResourceSource      GetSource() const { return m_source; }
		size_t              GetSize() const { return m_size; }
		IResourceDataPtr    GetData() { return m_data; }

		IResourceDataPtr m_data;
		size_t          m_size; // Size in bytes
		ResourceID      m_ID{};
		ResourceState   m_state;
		ResourceSource  m_source;
		std::string     m_path;

	};
	using ResourceDescPtr = std::shared_ptr<ResourceDesc>;
	using ResourceDescConstPtr = std::shared_ptr<const ResourceDesc>;

	// Resource descriptor wrapper that automatically deduces the GetData type
	// Override when creating a new resource type
	template<class T>
	class IResource {
	public:
		IResource(const ResourceDescPtr p_resourceDesc) :
			m_resourceDesc(p_resourceDesc)
		{}

		ResourceID          GetID() const { return m_resourceDesc->m_ID; }
		const std::string&  GetPath() const { return m_resourceDesc->m_path; }
		ResourceState       GetState() const { return m_resourceDesc->m_state; }
		ResourceSource      GetSource() const { return m_resourceDesc->m_source; }
		size_t              GetSize() const { return m_resourceDesc->m_size; }
		std::shared_ptr<T>  GetData() { return std::static_pointer_cast<T>(m_resourceDesc->m_data); }

	protected:
		ResourceDescPtr m_resourceDesc;
	};

	// Example usage
	//
	//class TextureData : public IResourceData
	//class TextureResource: public IResource<TextureData>

	// TextureResource textureResource = ResourceSystem::Get<T>("TestPath/Elo.png");
	// TextureData texture = textureResource->GetData();
}
