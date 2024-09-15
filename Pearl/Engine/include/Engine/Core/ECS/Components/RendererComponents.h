#pragma once

#include"Core/ECS/BaseComponent.h"
#include"Core/Resources/ResourceSystem.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Color.h"
#include"Renderer/Core/Camera.h"
#include"Renderer/Resources/Light.h"

namespace PrCore::ECS {

	enum class LightType {
		Directional = 0,
		Point = 1,
		Spot = 2
	};

	class MeshRendererComponent: public BaseComponent {
	public:
		MeshRendererComponent() = default;
		~MeshRendererComponent() override = default;

		bool shadowCaster = true;
		PrRenderer::Resources::MeshHandle mesh;
		PrRenderer::Resources::MeshHandle shadowMesh;
		PrRenderer::Resources::MaterialHandle material;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			if(shadowMesh != nullptr)
			{
				p_serialized["ShadowMesh"] = shadowMesh.GetPath();
			}
			p_serialized["Mesh"] = mesh.GetPath();
			p_serialized["Material"] = material.GetPath();
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			std::string meshName = p_deserialized["Mesh"];
			if(meshName.find("Primitive_Cube") != std::string::npos)
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
				mesh = Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>(static_cast<std::string>(p_deserialized["Mesh"]));

			auto shadowMeshIt = p_deserialized.find("ShadowMesh");
			if(shadowMeshIt != p_deserialized.end())
			{
				std::string meshName = p_deserialized["ShadowMesh"];
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
					shadowMesh = Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Mesh>(static_cast<std::string>(p_deserialized["ShadowMesh"]));
			}

			material = Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>(static_cast<std::string>(p_deserialized["Material"]));
		}
	};

	class LightComponent : public BaseComponent {
	public:

		PrRenderer::Resources::LightPtr m_light = std::make_shared<PrRenderer::Resources::Light>();
		bool m_shadowCast = true;
		bool mainDirectLight = false;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["Color"] = Utils::JSONParser::ParseColor(m_light->GetColor());
			p_serialized["LinearAttenuation"] = m_light->GetLinearAttenuation();
			p_serialized["QuadraticAttenuation"] = m_light->GetQuadraticAttenuation();
			p_serialized["ConstantAttenuation"] = m_light->GetConstantAttenuation();
			p_serialized["Range"] = m_light->GetRange();

			p_serialized["Type"] = m_light->GetType();
			p_serialized["InnerCone"] = m_light->GetInnerCone();
			p_serialized["OutterCone"] = m_light->GetOutterCone();

			p_serialized["ShadowCaster"] = m_shadowCast;
			p_serialized["MainLight"] = mainDirectLight;
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			m_light->SetColor(Utils::JSONParser::ToColor(p_deserialized["Color"]));
			m_light->SetAttenuation(p_deserialized["QuadraticAttenuation"], p_deserialized["LinearAttenuation"], p_deserialized["ConstantAttenuation"]);
			m_light->SetRange(p_deserialized["Range"]);

			m_light->SetType(p_deserialized["Type"]);
			m_light->SetInnerCone(p_deserialized["InnerCone"]);
			m_light->SetOutterCone(p_deserialized["OutterCone"]);

			m_shadowCast = p_deserialized["ShadowCaster"];
			mainDirectLight = p_deserialized["MainLight"];
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
			p_serialized["Type"] = (int)m_camera.GetType();
			p_serialized["ClearColor"] = Utils::JSONParser::ParseColor(m_camera.GetClearColor());
			p_serialized["Far"] = m_camera.GetFar();
			p_serialized["Near"] = m_camera.GetNear();
			p_serialized["FOV"] = m_camera.GetFOV();
			p_serialized["Ratio"] = m_camera.GetRatio();
			p_serialized["Size"] = m_camera.GetSize();
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			m_camera.SetType(p_deserialized["Type"]);
			m_camera.SetClearColor(Utils::JSONParser::ToColor(p_deserialized["ClearColor"]));
			m_camera.SetFar(p_deserialized["Far"]);
			m_camera.SetNear(p_deserialized["Near"]);
			m_camera.SetFOV(p_deserialized["FOV"]);
			m_camera.SetRatio(p_deserialized["Ratio"]);
			m_camera.SetSize(p_deserialized["Size"]);
		}

	private:
		PrRenderer::Core::Camera m_camera;
	};
}
