#pragma once
#include"Core/Math/Math.h"

namespace PrRenderer::Core {
	
	class Color: public PrCore::Math::vec4 {
	public:
		Color():
			PrCore::Math::vec4(0.0f, 0.0f, 0.0f, 0.0f)
		{}

		Color(float p_r, float p_g, float p_b, float p_a):
			PrCore::Math::vec4(p_r, p_g, p_b, p_a)
		{}

		Color(float p_r, float p_g, float p_b):
			PrCore::Math::vec4(p_r, p_g, p_b, 1.0f)
		{}

		Color(const PrCore::Math::vec4& p_vec4):
			PrCore::Math::vec4(p_vec4.r, p_vec4.g, p_vec4.b, p_vec4.a)
		{}

		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Black;
		static const Color White;
		static const Color Magenta;

		//Add more functions in future
	};
}