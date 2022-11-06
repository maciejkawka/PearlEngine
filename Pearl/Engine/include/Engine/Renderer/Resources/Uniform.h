#pragma once
#include<any>

namespace PrRenderer::Resources {

	enum class UniformType {
		Int,
		Int_Array,
		Float,
		Float_Array,
		Float_Vec2,
		Float_Vec2_Array,
		Float_Vec3,
		Float_Vec3_Array,
		Float_Vec4,
		Float_Vec4_Array,
		Float_Mat4,
		Float_Mat4_Array,
		Float_Mat3,
		Float_Mat3_Array,
		Bool,
		Texture2D,
		Texture3D,
		Cubemap,
		None
	};

	struct Uniform {
		UniformType		type = UniformType::None;
		unsigned int	size = 1;
		std::any		value;
	};
}