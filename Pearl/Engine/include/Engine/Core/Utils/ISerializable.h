#pragma once
#include <string>
#include "Core/Utils/JSONParser.h"

namespace PrCore::Utils {

	class ISerializable {
	public:
		virtual ~ISerializable() = default;

		virtual void OnSerialize(JSON::json& p_serialized) = 0;

		virtual void OnDeserialize(const JSON::json& p_deserialized) = 0;
	};

}