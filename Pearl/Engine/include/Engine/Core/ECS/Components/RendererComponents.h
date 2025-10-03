#pragma once

#include"Core/ECS/BaseComponent.h"
#include"Core/Resources/ResourceSystem.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Color.h"
#include"Renderer/Core/Camera.h"
#include"Renderer/Resources/Light.h"

namespace PrCore::ECS {

// Access first material in the component
#define mainMaterial materials[0]

	class MeshRendererComponent: public BaseComponent {
	public:
		MeshRendererComponent()
		{
			materials.resize(1);
		}
		~MeshRendererComponent() override = default;

		bool                                               shadowCaster = true;
		PrRenderer::Resources::MeshHandle                  mesh;
		PrRenderer::Resources::MeshHandle                  shadowMesh;
		std::vector<PrRenderer::Resources::MaterialHandle> materials;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			if (shadowMesh != nullptr)
			{
				if (shadowMesh.GetOrigin() == PrCore::Resources::ResourceOrigin::Memory)
					p_serialized["shadowMesh"] = shadowMesh->GetName();
				else
					p_serialized["shadowMesh"] = shadowMesh.GetPath();
			}

			if(mesh.GetOrigin() == PrCore::Resources::ResourceOrigin::Memory)
			{
				p_serialized["mesh"] = mesh->GetName();
			}
			else
			{
				p_serialized["mesh"] = mesh.GetPath();
			}

			Utils::JSON::json jsonMaterials;
			for (auto& mat : materials)
			{
				if (mat.GetOrigin() == PrCore::Resources::ResourceOrigin::File)
				{
					Utils::JSON::json jsonMatElement;
					jsonMatElement["path"] = mat.GetPath();
					jsonMaterials.push_back(jsonMatElement);
				}
				else
				{
					PRLOG_WARN("Material {} is not a file origin, cannot serialize", mat->GetName());
				}
			}

			p_serialized["materialList"] = jsonMaterials;
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			std::string meshName = p_deserialized["mesh"];
			if (meshName.find("Primitive_Cube") != std::string::npos)
				mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);
			else if (meshName.find("Primitive_Sphere") != std::string::npos)
				mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
			else if (meshName.find("Primitive_Capsule") != std::string::npos)
				mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Capsule);
			else if (meshName.find("Primitive_Cylinder") != std::string::npos)
				mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cylinder);
			else if (meshName.find("Primitive_Plane") != std::string::npos)
				mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Plane);
			else if (meshName.find("Primitive_Quad") != std::string::npos)
				mesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Quad);
			else
				mesh = Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>(static_cast<std::string>(p_deserialized["mesh"]));

			auto shadowMeshIt = p_deserialized.find("shadowMesh");
			if (shadowMeshIt != p_deserialized.end())
			{
				std::string meshName = p_deserialized["shadowMesh"];
				if (meshName.find("Primitive_Cube") != std::string::npos)
					shadowMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cube);
				else if (meshName.find("Primitive_Sphere") != std::string::npos)
					shadowMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Sphere);
				else if (meshName.find("Primitive_Capsule") != std::string::npos)
					shadowMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Capsule);
				else if (meshName.find("Primitive_Cylinder") != std::string::npos)
					shadowMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Cylinder);
				else if (meshName.find("Primitive_Plane") != std::string::npos)
					shadowMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Plane);
				else if (meshName.find("Primitive_Quad") != std::string::npos)
					shadowMesh = PrRenderer::Resources::Mesh::CreatePrimitive(PrRenderer::Resources::PrimitiveType::Quad);
				else
					shadowMesh = Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>(static_cast<std::string>(p_deserialized["shadowMesh"]));
			}

			auto materialJson = p_deserialized["materialList"];
			materials.resize(materialJson.size());
			for (int i = 0; i < materialJson.size(); ++i)
			{
				auto mat = Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>(static_cast<std::string>(materialJson.at(i)["path"]));
				materials[i] = mat;
			}
		}
	};

	class LightComponent : public BaseComponent {
	public:

		PrRenderer::Resources::LightPtr m_light = std::make_shared<PrRenderer::Resources::Light>();
		bool m_shadowCast = true;
		bool mainDirectLight = false;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["color"] = Utils::JSONParser::ParseColor(m_light->GetColor());
			p_serialized["linearAttenuation"] = m_light->GetLinearAttenuation();
			p_serialized["quadraticAttenuation"] = m_light->GetQuadraticAttenuation();
			p_serialized["constantAttenuation"] = m_light->GetConstantAttenuation();
			p_serialized["range"] = m_light->GetRange();

			p_serialized["type"] = m_light->GetType();
			p_serialized["innerCone"] = m_light->GetInnerCone();
			p_serialized["outterCone"] = m_light->GetOutterCone();

			p_serialized["shadowCaster"] = m_shadowCast;
			p_serialized["mainLight"] = mainDirectLight;
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			m_light->SetColor(Utils::JSONParser::ToColor(p_deserialized["color"]));
			m_light->SetAttenuation(p_deserialized["quadraticAttenuation"], p_deserialized["linearAttenuation"], p_deserialized["constantAttenuation"]);
			m_light->SetRange(p_deserialized["range"]);

			m_light->SetType(p_deserialized["type"]);
			m_light->SetInnerCone(p_deserialized["innerCone"]);
			m_light->SetOutterCone(p_deserialized["outterCone"]);

			m_shadowCast = p_deserialized["shadowCaster"];
			mainDirectLight = p_deserialized["mainLight"];
		}
	};

	class CameraComponent: public BaseComponent {
	public:
		CameraComponent() = default;

		inline void SetType(PrRenderer::Core::CameraType p_type) { m_camera.SetType(p_type); }
		inline void SetClearColor(const PrRenderer::Core::Color& p_clearColor) { m_camera.SetClearColor(p_clearColor); }
		inline void SetFar(float p_far) {m_camera.SetFar(p_far); }
		inline void SetNear(float p_near) { m_camera.SetNear(p_near); }
		inline void SetFOV(float p_FOV) { m_camera.SetFOV(p_FOV); }
		inline void SetRatio(float p_ratio) { m_camera.SetRatio(p_ratio); }
		inline void SetSize(float p_size) { m_camera.SetSize(p_size); }

		inline PrRenderer::Core::CameraType GetType() { return m_camera.GetType(); }
		inline PrRenderer::Core::Color GetClearColor() { return m_camera.GetClearColor(); }
		inline float GetFar() { return m_camera.GetFar(); }
		inline float GetNear() { return m_camera.GetNear(); }
		inline float GetFOV() { return m_camera.GetFOV(); }
		inline float GetRatio() { return m_camera.GetRatio(); }
		inline float GetSize() { return m_camera.GetSize(); }

		inline PrRenderer::Core::Camera* GetCamera() { return &m_camera; }


		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["type"] = (int)m_camera.GetType();
			p_serialized["clearColor"] = Utils::JSONParser::ParseColor(m_camera.GetClearColor());
			p_serialized["far"] = m_camera.GetFar();
			p_serialized["near"] = m_camera.GetNear();
			p_serialized["fov"] = m_camera.GetFOV();
			p_serialized["ratio"] = m_camera.GetRatio();
			p_serialized["size"] = m_camera.GetSize();
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			m_camera.SetType(p_deserialized["type"]);
			m_camera.SetClearColor(Utils::JSONParser::ToColor(p_deserialized["clearColor"]));
			m_camera.SetFar(p_deserialized["far"]);
			m_camera.SetNear(p_deserialized["near"]);
			m_camera.SetFOV(p_deserialized["fov"]);
			m_camera.SetRatio(p_deserialized["ratio"]);
			m_camera.SetSize(p_deserialized["size"]);
		}

	private:
		PrRenderer::Core::Camera m_camera;
	};
}
