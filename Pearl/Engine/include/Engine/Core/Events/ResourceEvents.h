#pragma once

#include"Event.h"
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
}


