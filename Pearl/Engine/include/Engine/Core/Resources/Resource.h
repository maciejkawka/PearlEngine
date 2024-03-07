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
		explicit Resource(const std::string& p_name);

		//Constructor for managed Resource
		explicit Resource(const std::string& p_name, ResourceHandle p_ID);

		virtual ~Resource() = default;

		void Load();
		void Unload();

		inline ResourceSize GetSize() const { return m_size; }
		inline std::string GetName() const { return m_name; }
		inline size_t GetNameHash() const { return m_nameHash; }
		inline ResourceHandle GetHandle() const { return m_handle; }

		//Only for not a file resources
		inline void SetName(const std::string& p_name)
		{
			m_name = p_name;
			m_nameHash = std::hash<std::string>{}(m_name);
		}

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

		virtual void LoadCorruptedResource() = 0;

		virtual void CalculateSize() = 0;

		std::string m_name;
		size_t m_nameHash;
		ResourceHandle m_handle;
		ResourceSize m_size;
		ResourceStatus m_state;
	};

	typedef std::shared_ptr<Resource> ResourcePtr;
}