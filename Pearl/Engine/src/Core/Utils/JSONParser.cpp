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

PrCore::Math::quat JSONParser::ToQuat(const JSON::json& p_json)
{
	return PrCore::Math::quat(p_json["w"], p_json["x"], p_json["y"], p_json["z"]);
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

JSON::json JSONParser::ParseColor(const PrRenderer::Core::Color& p_color)
{
	JSON::json color;
	color["r"] = p_color.r;
	color["g"] = p_color.g;
	color["b"] = p_color.b;
	color["a"] = p_color.a;
	return color;
}

JSON::json JSONParser::ParseVec2(const PrCore::Math::vec2& p_vec2)
{
	JSON::json vec2;
	vec2["x"] = p_vec2.x;
	vec2["y"] = p_vec2.y;

	return vec2;
}

JSON::json JSONParser::ParseVec3(const PrCore::Math::vec3& p_vec3)
{
	JSON::json vec3;
	vec3["x"] = p_vec3.x;
	vec3["y"] = p_vec3.y;
	vec3["z"] = p_vec3.z;

	return vec3;
}

JSON::json JSONParser::ParseVec4(const PrCore::Math::vec4& p_vec4)
{
	JSON::json vec4;
	vec4["x"] = p_vec4.x;
	vec4["y"] = p_vec4.y;
	vec4["z"] = p_vec4.z;
	vec4["w"] = p_vec4.w;

	return vec4;
}

JSON::json JSONParser::ParseQuat(const PrCore::Math::quat& p_quat)
{
	JSON::json quat;
	quat["x"] = p_quat.x;
	quat["y"] = p_quat.y;
	quat["z"] = p_quat.z;
	quat["w"] = p_quat.w;

	return quat;
}

JSON::json JSONParser::ParseMat4(const PrCore::Math::mat4& p_mat4)
{
	JSON::json mat4;

	mat4["m0"] = p_mat4[0].x;
	mat4["m4"] = p_mat4[0].y;
	mat4["m8"] = p_mat4[0].z;
	mat4["m12"] = p_mat4[0].w;

	mat4["m1"] = p_mat4[1].x;
	mat4["m5"] = p_mat4[1].y;
	mat4["m9"] = p_mat4[1].z;
	mat4["m13"] = p_mat4[1].w;

	mat4["m2"] = p_mat4[2].x;
	mat4["m6"] = p_mat4[2].y;
	mat4["m10"] = p_mat4[2].z;
	mat4["m14"] = p_mat4[2].w;

	mat4["m3"] = p_mat4[3].x;
	mat4["m4"] = p_mat4[3].y;
	mat4["m11"] = p_mat4[3].z;
	mat4["m15"] = p_mat4[3].w;

	return mat4;
}

JSON::json JSONParser::ParseMat3(const PrCore::Math::mat3& p_mat3)
{
	JSON::json mat3;

	mat3["m0"] = p_mat3[0].x;
	mat3["m3"] = p_mat3[0].y;
	mat3["m6"] = p_mat3[0].z;

	mat3["m1"] = p_mat3[1].x;
	mat3["m4"] = p_mat3[1].y;
	mat3["m7"] = p_mat3[1].z;

	mat3["m2"] = p_mat3[2].x;
	mat3["m5"] = p_mat3[2].y;
	mat3["m8"] = p_mat3[2].z;
						
	return mat3;
}
