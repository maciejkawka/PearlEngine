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

		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Black;
		static Color White;
		static Color Magenta;

		//Add more functions in future
	};
}