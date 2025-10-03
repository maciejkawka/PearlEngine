#pragma once

#include "Core/ECS/BaseComponent.h"
#include "Core/ECS/EntityManager.h"
#include "Core/ECS/SceneManager.h"
#include "Core/ECS/Scene.h"
#include "Core/ECS/Components/CoreComponents.h"

#include "Core/Math/Math.h"

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

		//Do not use does not update position, rotation, scale
		inline void SetWorldMatrix(const Math::mat4& p_worldMat) { m_worldMat = p_worldMat; }
		inline void SetLocalMatrix(const Math::mat4& p_localMat) { m_localMat = p_localMat; }

		bool IsDirty() const { return m_isDirty; }
		void SetDiry(bool p_isDirty) { m_isDirty = p_isDirty; }

		void DecomposeWorldMatrix();
		void DecomposeLocalMatrix();

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["position"] = Utils::JSONParser::ParseVec3(m_position);
			p_serialized["rotation"] = Utils::JSONParser::ParseQuat(m_rotation);
			p_serialized["scale"] = Utils::JSONParser::ParseVec3(m_scale);

			p_serialized["localPosition"] = Utils::JSONParser::ParseVec3(m_localPosition);
			p_serialized["localRotation"] = Utils::JSONParser::ParseQuat(m_localRotation);
			p_serialized["localScale"] = Utils::JSONParser::ParseVec3(m_localScale);
		}
		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			m_position = Utils::JSONParser::ToVec3(p_deserialized["position"]);
			m_rotation = Utils::JSONParser::ToQuat(p_deserialized["rotation"]);
			m_scale = Utils::JSONParser::ToVec3(p_deserialized["scale"]);

			m_localPosition = Utils::JSONParser::ToVec3(p_deserialized["localPosition"]);
			m_localRotation = Utils::JSONParser::ToQuat(p_deserialized["localRotation"]);
			m_localScale = Utils::JSONParser::ToVec3(p_deserialized["localScale"]);

			GenerateWorldMatrix();
			GenerateLocalMatrix();
		}

	private:
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

		Entity GetParent()
		{
			return parent;
		}

		void SetParent(Entity p_parent)
		{
			parent = p_parent;
			isDirty = true;
		}

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override 
		{
			p_serialized["parentUuid"] = parent.GetComponent<UUIDComponent>()->UUID;
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			if (p_deserialized.contains("parentUuid"))
			{
				parent = SceneManager::GetInstance().GetActiveScene()->GetEntityByID(p_deserialized["parentUuid"]);
				isDirty = true;
			}
		}
	};
}
