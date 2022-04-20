#pragma once
#include<string>
#include<memory>

namespace PrCore::Resources {

	class ResourceManager;

	typedef unsigned int ResourceHandle;
	typedef size_t ResourceSize;

	enum class ResourceStatus {
		Loading,
		Loaded,
		Unloading,
		Unloaded,
		Corrupted,
		Unmanaged
	};

	class Resource {
	public:
		Resource() = delete;

		//Constructor for unmanaged Resource
		Resource(const std::string& p_name);

		//Constructor for managed Resource
		Resource(const std::string& p_name, ResourceHandle p_ID);

		virtual ~Resource() = default;

		void Load();
		void Unload();

		inline ResourceSize GetSize() const { return m_size; }
		inline std::string GetName() const { return m_name; }
		inline ResourceHandle GetHandle() const { return m_handle; }

		inline ResourceStatus GetStatus() const { return m_state; }

		inline bool IsLoading() const { return m_state == ResourceStatus::Loading; }
		inline bool IsLoaded() const { return m_state == ResourceStatus::Loaded; }

		inline bool IsUnloading() const { return m_state == ResourceStatus::Unloading; }
		inline bool IsUnloaded() const { return m_state == ResourceStatus::Unloaded; }

		inline bool IsCorrupted() const { return m_state == ResourceStatus::Corrupted; }
		inline bool IsManaged() const { return m_state != ResourceStatus::Unmanaged; }

	protected:
		void FireUnloadedEvent();
		void FireLoadedEvent();
		void FireCorruptedEvent();

		virtual void PreLoadImpl() = 0;
		virtual bool LoadImpl() = 0;
		virtual void PostLoadImpl() = 0;

		virtual void PreUnloadImpl() = 0;
		virtual bool UnloadImpl() = 0;
		virtual void PostUnloadImpl() = 0;

		virtual void CalculateSize() = 0;

		std::string m_name;
		ResourceHandle m_handle;
		ResourceSize m_size;
		ResourceStatus m_state;
	};

	typedef std::shared_ptr<Resource> ResourcePtr;
}