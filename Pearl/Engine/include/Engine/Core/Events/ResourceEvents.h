#pragma once
#include"Event.h"
#include"Core/Resources/ResourceManager.h"

namespace PrCore::Events{

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

		virtual inline EventType GetType() { return s_type; }
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

		virtual inline EventType GetType() { return s_type; }
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

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0x63c8e53e;
	};

	class CacheMissEvent : public Event {
	public:
		CacheMissEvent(Resources::CacheMiss p_cacheMissType) :
			m_cacheMissType(p_cacheMissType)
		{}

		Resources::CacheMiss m_cacheMissType;

		virtual inline EventType GetType() { return s_type; }
		inline const static EventType s_type = 0xb5a6a882;
	};
}


