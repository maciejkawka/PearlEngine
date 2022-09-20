#pragma once
#include"Core/Utils/UUID.h"
#include"Core/ECS/BaseComponent.h"
#include"Core/Math/Math.h"
#include <string>

//Renderer
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Light.h"

namespace PrCore::ECS {

	//Mandatory Components
	class UUIDComponent : public BaseComponent {
	public:
		Utils::UUID UUID;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["UUID"] = UUID;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			UUID = p_deserialized["UUID"];
		}
	};

	class NameComponent : public BaseComponent {
	public:
		std::string name;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["name"] = name;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			name = p_deserialized["name"];
		}
	};

	class TagComponent : public BaseComponent {
	public:
		std::string tag;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["tag"] = tag;
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			tag = p_deserialized["tag"];
		}
	};
	//

	//Transformations
	class TransformComponent : public BaseComponent {
	public:
		TransformComponent()
		{
			position = { 1.0f, 1.0f, 1.0f };
			rotation = { 0.0f, 0.0f, 0.0f };
			scale = { 1.0f, 1.0f, 1.0f };

			localPosition = { 1.0f, 1.0f, 1.0f };
			localRotation = { 0.0f, 0.0f, 0.0f };
			localScale = { 1.0f, 1.0f, 1.0f };
		}
		
		Math::vec3 position;
		Math::vec3 rotation;
		Math::vec3 scale;

		Math::vec3 localPosition;
		Math::vec3 localRotation;
		Math::vec3 localScale;

		glm::mat4 GetLocalMatrix() const
		{
			Math::mat4 rotationMat = Math::toMat4(Math::quat(localRotation));

			return Math::translate(Math::mat4(1.0f), localPosition)
				* rotationMat
				* Math::scale(Math::mat4(1.0f), localScale);
		}

		glm::mat4 GetWorldMatrix() const
		{
			Math::mat4 rotationMat = Math::toMat4(Math::quat(rotation));

			return Math::translate(Math::mat4(1.0f), position)
				* rotationMat
				* Math::scale(Math::mat4(1.0f), scale);
		}
	};

	class ForParentComponent: public BaseComponent {
	public:
		size_t childCount = 0;
		std::vector<Entity> children;
	};

	class ForChildComponent : public BaseComponent {
	public:
		Entity parent;
	};
	//

	//Rendering

	class MeshRendererComponent: public BaseComponent {
	public:
		std::shared_ptr<PrRenderer::Resources::Mesh> mesh;
		std::shared_ptr < PrRenderer::Resources::Material> material;
	};

	class LightComponent: public BaseComponent {
	public:
		PrRenderer::Core::Light light;
	};

	//

	//Test
	struct TimeComponent : public BaseComponent
	{
		float time = 0;
	};
}
