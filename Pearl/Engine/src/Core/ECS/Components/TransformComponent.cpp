#include "Core/Common/pearl_pch.h"

#include"Core/ECS/Components/TransformComponent.h"


using namespace PrCore::ECS;

TransformComponent::TransformComponent()
{
	m_worldMat = Math::mat4();
	m_position = Math::vec3(0.0f);
	m_rotation = Math::quat(Math::vec4(0.0f));
	m_scale = Math::vec3(1.0f);
	
	m_localMat = Math::mat4();
	m_localPosition  = Math::vec3(0.0f);
	m_localRotation = Math::quat(Math::vec4(0.0f));
	m_localScale = Math::vec3(1.0f);

	GenerateLocalMatrix();
	GenerateWorldMatrix();

	m_isDirty = true;
}

void TransformComponent::SetPosition(const Math::vec3& p_position)
{
	if(m_position == m_localPosition)
	{
		m_position = p_position;
		m_localPosition = p_position;
	}
	else
	{
		auto positionDiff = p_position - m_position;
		m_localPosition += positionDiff;
		m_position = p_position;
	}

	GenerateLocalMatrix();
	GenerateWorldMatrix();

	m_isDirty = true;
}

void TransformComponent::SetRotation(const Math::quat& p_rotation)
{
	if(m_rotation == m_localRotation)
	{
		m_rotation = p_rotation;
		m_localRotation = p_rotation;
	}
	else
	{
		auto rotationDiff = Math::inverse(m_rotation) * p_rotation;
		m_localRotation *= rotationDiff;
		m_rotation = p_rotation;
	}

	GenerateLocalMatrix();
	GenerateWorldMatrix();

	m_isDirty = true;
}

void TransformComponent::SetLocalPosition(const Math::vec3& p_position)
{
	if (m_position == m_localPosition)
	{
		m_position = p_position;
		m_localPosition = p_position;
	}
	else
	{
		auto positionDiff = p_position - m_localPosition;
		m_position += positionDiff;
		m_localPosition = p_position;
	}

	GenerateLocalMatrix();
	GenerateWorldMatrix();

	m_isDirty = true;
}

void TransformComponent::SetLocalRotation(const Math::quat& p_rotation)
{
	if (m_rotation == m_localRotation)
	{
		m_rotation = p_rotation;
		m_localRotation = p_rotation;
	}
	else
	{
		auto rotationDiff = Math::inverse(m_localRotation) * p_rotation;
		m_rotation *= rotationDiff;
		m_localRotation = p_rotation;
	}

	GenerateLocalMatrix();
	GenerateWorldMatrix();

	m_isDirty = true;
}

void TransformComponent::SetLocalScale(const Math::vec3& p_scale)
{
	if(m_scale == m_localScale)
	{
		m_scale = p_scale;
		m_localScale = p_scale;
	}
	else
	{
		m_localScale = p_scale;
		m_scale *= p_scale;
	}

	GenerateLocalMatrix();
	GenerateWorldMatrix();

	m_isDirty = true;
}

void TransformComponent::GenerateWorldMatrix()
{
	m_worldMat = Math::translate(Math::mat4(1.0f), m_position)
		* Math::toMat4(m_rotation)
		* Math::scale(Math::mat4(1.0f), m_scale);
}

void TransformComponent::GenerateLocalMatrix()
{
	m_localMat = Math::translate(Math::mat4(1.0f), m_localPosition)
		* Math::toMat4(m_localRotation)
		* Math::scale(Math::mat4(1.0f), m_localScale);
}

PrCore::Math::vec3 TransformComponent::GetEulerRotation() const
{
	auto eulerAngles = Math::eulerAngles(m_rotation);

	return Math::degrees(eulerAngles);
}

PrCore::Math::vec3 TransformComponent::GetLocalEulerRotation() const
{
	auto eulerAngles = Math::eulerAngles(m_localRotation);

	return Math::degrees(eulerAngles);
}

PrCore::Math::vec3 TransformComponent::GetUpVector() const
{
	return m_rotation * Math::vec3(0.0f, 1.0f, 0.0f);
}

PrCore::Math::vec3 TransformComponent::GetRightVector() const
{
	return m_rotation * Math::vec3(1.0f, 0.0f, 0.0f);
}

PrCore::Math::vec3 TransformComponent::GetForwardVector() const
{
	return m_rotation * Math::vec3(0.0f, 0.0f, 1.0f);
}

void TransformComponent::DecomposeWorldMatrix()
{
	m_position.x = m_worldMat[3].x;
	m_position.y = m_worldMat[3].y;
	m_position.z = m_worldMat[3].z;

	m_scale.x = Math::length(Math::vec3(m_worldMat[0].x,
		m_worldMat[0].y,
		m_worldMat[0].z));
	m_scale.y = Math::length(Math::vec3(m_worldMat[1].x,
		m_worldMat[1].y,
		m_worldMat[1].z));
	m_scale.z = Math::length(Math::vec3(m_worldMat[2].x,
		m_worldMat[2].y,
		m_worldMat[2].z));

	Math::mat3 RS(m_worldMat);
	Math::mat3 scaleMat = Math::scale(Math::mat4(1), m_scale);
	Math::mat3 rotationMat = RS * Math::inverse(scaleMat);

	m_rotation = Math::quat(rotationMat);
}

void TransformComponent::DecomposeLocalMatrix()
{
	m_localPosition.x = m_localMat[3].x;
	m_localPosition.y = m_localMat[3].y;
	m_localPosition.z = m_localMat[3].z;

	m_localScale.x = Math::length(Math::vec3(m_localMat[0].x,
					m_localMat[0].y,
					m_localMat[0].z));
	m_localScale.y = Math::length(Math::vec3(m_localMat[1].x,
					m_localMat[1].y,
					m_localMat[1].z));
	m_localScale.z = Math::length(Math::vec3(m_localMat[2].x,
					m_localMat[2].y,
					m_localMat[2].z));

	Math::mat3 RS(m_localMat);
	Math::mat3 scaleMat = Math::scale(Math::mat4(1), m_localScale);
	Math::mat3 rotationMat = RS * Math::inverse(scaleMat);

	m_localRotation = Math::quat(rotationMat);
}
