#pragma once
#include"Core/Utils/UUID.h"
#include"Core/ECS/BaseComponent.h"
#include <string>

//Renderer
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Light.h"

//Include Components
#include"Core/ECS/Components/TransformComponent.h"

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
