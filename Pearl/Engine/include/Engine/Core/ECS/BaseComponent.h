#pragma once
#include"Core/Utils/ISerializable.h"

namespace PrCore::ECS {

	class BaseComponent: public Utils::ISerializable {
	public:
		BaseComponent() = default;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};
}
