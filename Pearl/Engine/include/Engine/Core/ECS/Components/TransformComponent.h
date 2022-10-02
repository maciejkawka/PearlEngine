#pragma once
#include "Core/ECS/BaseComponent.h"
#include "Core/Math/Math.h"
#include "Core/ECS/EntityManager.h"


namespace PrCore::ECS {

	class TransformComponent : public BaseComponent {
	public:
		TransformComponent();

		void SetPosition(const Math::vec3& p_position);
		void SetRotation(const Math::quat& p_rotation);

		void SetLocalPosition(const Math::vec3& p_position);
		void SetLocalRotation(const Math::quat& p_rotation);
		void SetLocalScale(const Math::vec3& p_scale);

		void GenerateWorldMatrix();
		void GenerateLocalMatrix();

		inline Math::vec3 GetPosition() const { return m_position; }
		inline Math::quat GetRotation() const { return m_rotation; }
		inline Math::vec3 GetScale() const { return m_scale; }
		Math::vec3 GetEulerRotation() const;

		inline Math::vec3 GetLocalPosition() const { return m_localPosition; }
		inline Math::quat GetLocalRotation() const { return m_localRotation; }
		inline Math::vec3 GetLocalScale() const { return m_localScale; }
		Math::vec3 GetLocalEulerRotation() const;

		Math::vec3 GetUpVector() const;
		Math::vec3 GetRightVector() const;
		Math::vec3 GetForwardVector() const;

		Math::mat4 GetWorldMatrix() const { return m_worldMat; }
		Math::mat4 GetLocalMatrix() const { return m_localMat; }

		bool IsDirty() const { return m_isDirty; }
		void SetDiry(bool p_isDirty) { m_isDirty = p_isDirty; }

		Math::mat4 m_worldMat;
		Math::vec3 m_position;
		Math::quat m_rotation;
		Math::vec3 m_scale;

		Math::mat4 m_localMat;
		Math::vec3 m_localPosition;
		Math::quat m_localRotation;
		Math::vec3 m_localScale;

		bool m_isDirty;;
	};

	class ParentComponent : public BaseComponent {
	public:
		Entity parent;
		bool isDirty = true;
	};
}
