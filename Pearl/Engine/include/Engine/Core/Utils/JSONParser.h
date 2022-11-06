#pragma once

#include"Renderer/Core/Color.h"

#include"json/json.hpp"

namespace PrCore {
	namespace Utils {
		namespace JSON = nlohmann;
	}
}

namespace PrCore::Utils {

	class JSONParser {
	public:
		JSONParser() = delete;
		JSONParser(JSONParser& p_copy) = delete;

		static PrRenderer::Core::Color ToColor(const JSON::json& p_json);

		static Math::vec2 ToVec2(const JSON::json& p_json);
		static Math::vec3 ToVec3(const JSON::json& p_json);
		static Math::vec4 ToVec4(const JSON::json& p_json);
		static Math::quat ToQuat(const JSON::json& p_json);

		static Math::mat4 ToMat4(const JSON::json& p_json);
		static Math::mat3 ToMat3(const JSON::json& p_json);

		static JSON::json ParseColor(const PrRenderer::Core::Color& p_color);

		static JSON::json ParseVec2(const Math::vec2& p_vec2);
		static JSON::json ParseVec3(const Math::vec3& p_vec3);
		static JSON::json ParseVec4(const Math::vec4& p_vec4);
		static JSON::json ParseQuat(const Math::quat& p_quat);

		static JSON::json ParseMat4(const Math::mat4& p_mat4);
		static JSON::json ParseMat3(const Math::mat3& p_mat3);
	};
}