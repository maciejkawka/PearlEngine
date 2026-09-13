#pragma once

#include "Core/Utils/ISerializable.h"

namespace PrCore {

	class BaseComponent: public Utils::ISerializable {
	public:
		BaseComponent() = default;
		virtual ~BaseComponent() = default;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override 
		{
		}

		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override 
		{
		}
	};
}
