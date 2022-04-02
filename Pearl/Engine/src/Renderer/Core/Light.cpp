#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Light.h"

using namespace PrRenderer::Core;


Light::Light():
	m_position(0.0f),
	m_direction(0.0f),
	m_color(PrRenderer::Core::Color::White),
	m_linearAttenuation(0.5f),
	m_quadraticAttenuation(0.5f),
	m_constantAttenuation(1.0f),
	m_range(500.0f),
	m_type(LightType::Directional),
	m_innerCone(0.0f),
	m_outterCone(0.0f)
{
}

Light::Light(LightType p_type):
	m_position(0.0f),
	m_direction(0.0f),
	m_color(PrRenderer::Core::Color::White),
	m_linearAttenuation(0.5f),
	m_quadraticAttenuation(0.5f),
	m_constantAttenuation(1.0f),
	m_range(10.0f),
	m_type(p_type),
	m_innerCone(0.0f),
	m_outterCone(0.0f)
{
}

Light::Light(LightType p_type, PrCore::Math::vec3 p_position):
	m_position(p_position),
	m_direction(0.0f),
	m_color(PrRenderer::Core::Color::White),
	m_linearAttenuation(1.0f),
	m_quadraticAttenuation(0.0f),
	m_constantAttenuation(1.0f),
	m_range(1.0f),
	m_type(p_type),
	m_innerCone(0.0f),
	m_outterCone(0.0f)
{
}

Light::Light(LightType p_type, PrCore::Math::vec3 p_position, PrCore::Math::vec3 p_direction):
	m_position(p_position),
	m_direction(p_direction),
	m_color(PrRenderer::Core::Color::White),
	m_linearAttenuation(1.0f),
	m_quadraticAttenuation(0.0f),
	m_constantAttenuation(1.0f),
	m_range(1.0f),
	m_type(p_type),
	m_innerCone(0.0f),
	m_outterCone(0.0f)
{
}

PrCore::Math::mat4 Light::GetPackedMatrix() const
{
	PrCore::Math::mat4 mat;

	mat[0][0] = m_position.x;
	mat[0][1] = m_position.y;
	mat[0][2] = m_position.z;
	mat[0][3] = (float)m_type;

	mat[1][0] = m_direction.x;
	mat[1][1] = m_direction.y;
	mat[1][2] = m_direction.z;
	mat[1][3] = PrCore::Math::cos(PrCore::Math::radians(m_innerCone));

	mat[2][0] = m_color.x;
	mat[2][1] = m_color.y;
	mat[2][2] = m_color.z;
	mat[2][3] = PrCore::Math::cos(PrCore::Math::radians(m_outterCone));

	mat[3][0] = m_quadraticAttenuation;
	mat[3][1] = m_linearAttenuation;
	mat[3][2] = m_constantAttenuation;
	mat[3][3] = m_range;

	return mat;
}
