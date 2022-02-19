#pragma once

#include"Renderer/Core/Color.h"

#include"json/json.hpp"

namespace PrCore {
	namespace Utils
	{
		namespace JSON = nlohmann;
	}
}

namespace PrCore::Utils {

	class JSONParser {
	public:
		JSONParser() = delete;
		JSONParser(JSONParser& p_copy) = delete;

		static const PrRenderer::Core::Color& ToColor(const JSON::json& p_json);

		static const PrCore::Math::vec2& ToVec2(const JSON::json& p_json);
		static const PrCore::Math::vec3& ToVec3(const JSON::json& p_json);
		static const PrCore::Math::vec4& ToVec4(const JSON::json& p_json);

		static const PrCore::Math::mat4& ToMat4(const JSON::json& p_json);
		static const PrCore::Math::mat3& ToMat3(const JSON::json& p_json);
	};
}