#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Light.h"

using namespace PrRenderer::Resources;


Light::Light():
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

Light::Light(const Light& p_other) :
	m_color(p_other.m_color),
	m_linearAttenuation(p_other.m_linearAttenuation),
	m_quadraticAttenuation(p_other.m_quadraticAttenuation),
	m_constantAttenuation(p_other.m_constantAttenuation),
	m_range(p_other.m_range),
	m_type(p_other.m_type),
	m_innerCone(p_other.m_innerCone),
	m_outterCone(p_other.m_outterCone)
{
}

PrCore::Math::mat4 Light::CreatePackedMatrix(const PrCore::Math::vec3& p_position, const PrCore::Math::vec3& p_direction) const
{
	PrCore::Math::mat4 mat;

	mat[0][0] = p_position.x;
	mat[0][1] = p_position.y;
	mat[0][2] = p_position.z;
	mat[0][3] = (float)m_type;

	mat[1][0] = p_direction.x;
	mat[1][1] = p_direction.y;
	mat[1][2] = p_direction.z;
	mat[1][3] = PrCore::Math::cos(PrCore::Math::radians(m_innerCone));

	mat[2][0] = PrCore::Math::max(m_color.x, 0.0f);
	mat[2][1] = PrCore::Math::max(m_color.y, 0.0f);
	mat[2][2] = PrCore::Math::max(m_color.z, 0.0f);
	mat[2][3] = PrCore::Math::cos(PrCore::Math::radians(m_outterCone));

	mat[3][0] = m_quadraticAttenuation; // Unused
	mat[3][1] = m_linearAttenuation; // Unused
	mat[3][2] = m_constantAttenuation; // Unused
	mat[3][3] = m_range;

	return mat;
}
