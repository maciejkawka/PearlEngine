#pragma once
#include<string>
#include<memory>

namespace PrCore::Resources {

	class ResourceManager;

	typedef unsigned int ResourceID;
	typedef size_t ResourceSize;

	enum class ResourceStatus {
		Loading,
		Loaded,
		Unloading,
		Unloaded,
		Corrupted
	};

	class Resources {
	public:
		Resources(const std::string& p_name, ResourceManager* p_creator, ResourceID p_ID);
		virtual ~Resources() {}

		void Load();
		void Unload();

		inline ResourceSize GetSize() { return m_size; }
		inline std::string GetName() { return m_name; }
		inline ResourceID GetID() { return m_ID; }

		inline ResourceStatus GetStatus() { return m_state; }

		inline bool IsLoading() { return m_state == ResourceStatus::Loading; }
		inline bool IsLoaded() { return m_state == ResourceStatus::Loaded; }

		inline bool IsUnloading() { return m_state == ResourceStatus::Unloading; }
		inline bool IsUnloaded() { return m_state == ResourceStatus::Unloaded; }

		inline bool IsCorrupted() { return m_state == ResourceStatus::Corrupted; }

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
		ResourceID m_ID;
		ResourceSize m_size;
		ResourceManager* m_creator;
		ResourceStatus m_state;
	};

	typedef std::shared_ptr<Resources> ResourcePtr;
}