#pragma once
#include<string>
#include<any>

namespace PrRenderer::Resources {

	enum class UniformType {
		Int,
		Float,
		Float_Vec2,
		Float_Vec3,
		Float_Vec4,
		Float_Mat4,
		Float_Mat3,
		Texture2D,
		Texture3D,
		Cubemap,
		None
	};

	struct Uniform {
		std::string		name;
		UniformType		type;
		unsigned int	location;
	};
}