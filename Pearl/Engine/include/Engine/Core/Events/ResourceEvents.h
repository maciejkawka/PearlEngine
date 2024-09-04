#pragma once
#include"Event.h"
#include"Core/Resources/ResourceManager.h"
#include"Core/Resources/IResource.h"

namespace PrCore::Events{

	class ResourceUnloadedEventv2 : public Event {
	public:
		ResourceUnloadedEventv2(Resources::ResourceID p_id, const std::string& p_path) :
			m_id(p_id),
			m_path(p_path)
		{}

		Resources::ResourceID  m_id;
		std::string            m_path;

		DEFINE_EVENT_GUID(0x627b347d);
	};

	class ResourceLoadedEventv2 : public Event {
	public:
		ResourceLoadedEventv2(Resources::ResourceID p_id, const std::string& p_path) :
			m_id(p_id),
			m_path(p_path)
		{}

		Resources::ResourceID  m_id;
		std::string            m_path;

		DEFINE_EVENT_GUID(0x531d7fe);
	};

	class ResourceCorruptedEventv2 : public Event {
	public:
		ResourceCorruptedEventv2(Resources::ResourceID p_id, const std::string& p_path) :
			m_id(p_id),
			m_path(p_path)
		{}

		Resources::ResourceID  m_id;
		std::string            m_path;

		DEFINE_EVENT_GUID(0x98fb1b9e);
	};

	class CacheMissEventv2 : public Event {
	public:
		CacheMissEventv2(Resources::ResourceID p_id, const std::string& p_path) :
			m_id(p_id),
			m_path(p_path)
		{}

		Resources::ResourceID  m_id;
		std::string            m_path;

		DEFINE_EVENT_GUID(0xd28d9ae4);
	};

	class BudgetExceededv2 : public Event {
	public:
		BudgetExceededv2(Resources::ResourceID p_id, const std::string& p_path, size_t p_usage, size_t p_budget) :
			m_id(p_id),
			m_path(p_path),
			m_usage(p_usage),
			m_budget(p_budget)
		{}

		Resources::ResourceID  m_id;
		std::string            m_path;
		size_t                 m_usage;
		size_t                 m_budget;

		DEFINE_EVENT_GUID(0x5853dcbd);
	};

	class ResourceLoadedEvent : public Event {
	public:
		ResourceLoadedEvent(const std::string p_name, Resources::ResourceSize p_size,
			Resources::ResourceHandle p_ID):
			m_name(p_name),
			m_size(p_size),
			m_ID(p_ID)
		{}
	
		std::string m_name;
		Resources::ResourceSize m_size;
		Resources::ResourceHandle m_ID;

		virtual inline EventType GetType() const { return s_type; }
		inline const static EventType s_type = 0xa7288100;
	};

	class ResourceUnloadedEvent : public Event {
	public:
		ResourceUnloadedEvent(const std::string p_name, Resources::ResourceSize p_size,
			Resources::ResourceHandle p_ID) :
			m_name(p_name),
			m_size(p_size),
			m_ID(p_ID)
		{}

		std::string m_name;
		Resources::ResourceSize m_size;
		Resources::ResourceHandle m_ID;

		virtual inline EventType GetType() const { return s_type; }
		inline const static EventType s_type = 0x3149ba25;
	};

	class ResourceCorruptedEvent : public Event {
	public:
		ResourceCorruptedEvent(const std::string p_name, Resources::ResourceSize p_size,
			Resources::ResourceHandle p_ID) :
			m_name(p_name),
			m_size(p_size),
			m_ID(p_ID)
		{}

		std::string m_name;
		Resources::ResourceSize m_size;
		Resources::ResourceHandle m_ID;

		virtual inline EventType GetType() const { return s_type; }
		inline const static EventType s_type = 0x63c8e53e;
	};

	class CacheMissEvent : public Event {
	public:
		CacheMissEvent(Resources::CacheMiss p_cacheMissType) :
			m_cacheMissType(p_cacheMissType)
		{}

		Resources::CacheMiss m_cacheMissType;

		virtual inline EventType GetType() const { return s_type; }
		inline const static EventType s_type = 0xb5a6a882;
	};
}


