#pragma once

#include"Core/Math/Math.h"
#include"Renderer/Core/Color.h"

namespace PrRenderer::Core {

	enum class LightType{
		Directional = 0,
		Point = 1,
		Spot = 2
	};

	class Light {
	public:
		Light();
		Light(LightType p_type);
		Light(LightType p_type, PrCore::Math::vec3 p_position);
		Light(LightType p_type, PrCore::Math::vec3 p_position, PrCore::Math::vec3 p_direction);

		~Light() = default;

		inline void SetType(LightType p_type) { m_type = p_type; }
		inline void SetPosition(PrCore::Math::vec3 p_position) { m_position = p_position; }
		inline void SetDirection(PrCore::Math::vec3 p_direction) { m_direction = p_direction; }
		inline void SetColor(PrRenderer::Core::Color p_color) { m_color = p_color; }

		inline void SetAttenuation(float p_quadratic, float p_linear, float p_constant = 1.0f) {
			m_linearAttenuation = p_linear; m_quadraticAttenuation = p_quadratic; m_constantAttenuation = p_constant;
		}
		inline void SetRange(float p_range) { m_range = p_range; }

		inline void SetInnerCone(float p_inner) { m_innerCone = p_inner; }
		inline void SetoutterCone(float p_outter) { m_outterCone = p_outter; }

		inline LightType GetType() { return m_type; }
		inline PrCore::Math::vec3 GetPosition() { return m_position; }
		inline PrCore::Math::vec3 GetDirection() { return m_direction; }
		inline PrRenderer::Core::Color GetColor() { return m_color; }

		inline float GetQuadraticAttenuation() { return m_quadraticAttenuation; }
		inline float GetLinearAttenuation() { return m_linearAttenuation; }
		inline float GetConstantAttenuation() { return m_constantAttenuation; }
		inline float GetRange() { return m_range; }

		inline float GetInnerCone() { return m_innerCone; }
		inline float GetOutterCone() { return m_outterCone; }

		PrCore::Math::mat4 GetPackedMatrix() const;

	private:
		PrCore::Math::vec3 m_position;
		PrCore::Math::vec3 m_direction;
		PrRenderer::Core::Color m_color;

		float m_linearAttenuation;
		float m_quadraticAttenuation;
		float m_constantAttenuation;
		float m_range;

		LightType m_type;
		float m_innerCone;
		float m_outterCone;
	};

	typedef std::shared_ptr<Light> LightPtr;
}