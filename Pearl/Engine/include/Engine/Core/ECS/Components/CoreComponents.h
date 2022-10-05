#include "Core/ECS/BaseComponent.h"

#include "Core/Utils/UUID.h"

namespace PrCore::ECS {
	
	class UUIDComponent : public BaseComponent {
	public:
		Utils::UUID UUID;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["UUID"] = UUID;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			UUID = p_deserialized["UUID"];
		}
	};

	class NameComponent : public BaseComponent {
	public:
		std::string name;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["name"] = name;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			name = p_deserialized["name"];
		}
	};

	class TagComponent : public BaseComponent {
	public:
		std::string tag;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["tag"] = tag;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			tag = p_deserialized["tag"];
		}
	};

	class ToDestoryTag : public BaseComponent {
	public:

	};
}