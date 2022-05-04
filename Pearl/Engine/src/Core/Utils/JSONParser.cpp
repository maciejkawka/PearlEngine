#include"Core/Common/pearl_pch.h"

#include"Core/Utils/JSONParser.h"

using namespace PrCore::Utils;

PrRenderer::Core::Color JSONParser::ToColor(const JSON::json& p_json)
{
	return 	PrRenderer::Core::Color(p_json["r"],
		p_json["g"],
		p_json["b"],
		p_json["a"]);
}

PrCore::Math::vec2 JSONParser::ToVec2(const JSON::json& p_json)
{
	return PrCore::Math::vec2(p_json["x"], p_json["y"]);
}

PrCore::Math::vec3 JSONParser::ToVec3(const JSON::json& p_json)
{
	return PrCore::Math::vec3(p_json["x"], p_json["y"], p_json["z"]);
}

PrCore::Math::vec4 JSONParser::ToVec4(const JSON::json& p_json)
{
	return PrCore::Math::vec4(p_json["x"], p_json["y"], p_json["z"], p_json["w"]);
}

PrCore::Math::mat4 JSONParser::ToMat4(const JSON::json& p_json)
{
	return PrCore::Math::mat4(
		(float)p_json["m0"], (float)p_json["m1"], (float)p_json["m2"], (float)p_json["m3"],
		(float)p_json["m4"], (float)p_json["m5"], (float)p_json["m6"], (float)p_json["m7"],
		(float)p_json["m8"], (float)p_json["m9"], (float)p_json["m10"],(float)p_json["m11"],
		(float)p_json["m12"], (float)p_json["m13"], (float)p_json["m14"], (float)p_json["m15"]
	);
}

PrCore::Math::mat3 JSONParser::ToMat3(const JSON::json& p_json)
{
	return PrCore::Math::mat3(
		(float)p_json["m0"], (float)p_json["m1"], (float)p_json["m2"],
		(float)p_json["m3"], (float)p_json["m4"], (float)p_json["m5"],
		(float)p_json["m6"], (float)p_json["m7"], (float)p_json["m8"]
	);
}
