#pragma once
#include"Core/ECS/BaseComponent.h"
#include"Core/Resources/ResourceLoader.h"
#include "Core/Resources/ResourceManager.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Color.h"
#include"Renderer/Core/Camera.h"

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

		std::shared_ptr<PrRenderer::Resources::Mesh> mesh;
		std::shared_ptr<PrRenderer::Resources::Material> material;

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["Mesh"] = mesh->GetName();
			p_serialized["Material"] = material->GetName();
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
				mesh = Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Mesh>(p_deserialized["Mesh"]);

			material = Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>(p_deserialized["Material"]);
		}
	};

	class LightComponent : public BaseComponent {
	public:
		LightComponent() :
			m_direction(PrCore::Math::vec3(1.0f)),
			m_color(PrRenderer::Core::Color::White),
			m_linearAttenuation(0.5f),
			m_quadraticAttenuation(0.5f),
			m_constantAttenuation(1.0f),
			m_range(500.0f),
			m_type(LightType::Directional),
			m_innerCone(0.0f),
			m_outterCone(0.0f)
		{}
		~LightComponent() override = default;

		inline void SetType(LightType p_type) { m_type = p_type; }
		inline void SetDirection(PrCore::Math::vec3 p_direction) { m_direction = p_direction; }
		inline void SetColor(PrRenderer::Core::Color p_color) { m_color = p_color; }

		inline void SetAttenuation(float p_quadratic, float p_linear, float p_constant = 1.0f) {
			m_linearAttenuation = p_linear; m_quadraticAttenuation = p_quadratic; m_constantAttenuation = p_constant;
		}
		inline void SetRange(float p_range) { m_range = p_range; }

		inline void SetInnerCone(float p_inner) { m_innerCone = p_inner; }
		inline void SetOutterCone(float p_outter) { m_outterCone = p_outter; }

		inline LightType GetType() const { return m_type; }
		inline PrCore::Math::vec3 GetDirection() const { return m_direction; }
		inline PrRenderer::Core::Color GetColor() const { return m_color; }

		inline float GetQuadraticAttenuation() const { return m_quadraticAttenuation; }
		inline float GetLinearAttenuation() const { return m_linearAttenuation; }
		inline float GetConstantAttenuation() const { return m_constantAttenuation; }
		inline float GetRange() const { return m_range; }

		inline float GetInnerCone() const { return m_innerCone; }
		inline float GetOutterCone() const { return m_outterCone; }

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override
		{
			p_serialized["Direction"] = Utils::JSONParser::ParseVec3(m_direction);
			p_serialized["Color"] = Utils::JSONParser::ParseColor(m_color);
			p_serialized["LinearAttenuation"] = m_linearAttenuation;
			p_serialized["QuadraticAttenuation"] = m_quadraticAttenuation;
			p_serialized["ConstantAttenuation"] = m_constantAttenuation;
			p_serialized["Range"] = m_range;

			p_serialized["Type"] = (int)m_type;
			p_serialized["InnerCone"] = m_innerCone;
			p_serialized["OutterCone"] = m_outterCone;
		}

		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
			m_direction = Utils::JSONParser::ToVec3(p_deserialized["Direction"]);
			m_color = Utils::JSONParser::ToColor(p_deserialized["Color"]);
			m_linearAttenuation = p_deserialized["LinearAttenuation"];
			m_quadraticAttenuation = p_deserialized["QuadraticAttenuation"];
			m_constantAttenuation = p_deserialized["ConstantAttenuation"];
			m_range = p_deserialized["Range"];

			m_type = p_deserialized["Type"];
			m_innerCone = p_deserialized["InnerCone"];
			m_outterCone = p_deserialized["OutterCone"];
		}

	private:
		Math::vec3 m_direction;
		PrRenderer::Core::Color m_color;

		float m_linearAttenuation;
		float m_quadraticAttenuation;
		float m_constantAttenuation;
		float m_range;

		LightType m_type;
		float m_innerCone;
		float m_outterCone;
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
