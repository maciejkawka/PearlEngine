#pragma once
#include"Core/ECS/BaseSystem.h"
#include"Core/ECS/Components.h"

namespace PrCore::ECS {

	class MeshRendererSystem: public BaseSystem {
	public:
		MeshRendererSystem() = default;
		~MeshRendererSystem() override;

		void OnCreate() override;
		void OnEnable() override;
		void OnDisable() override;
		void OnUpdate(float p_dt) override;

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}

	private:
		Math::mat4 GetPackedMatrix(const LightComponent* p_lightComponent,const TransformComponent* p_transform) const;

		PrRenderer::Core::Camera* m_camera;
	};
}