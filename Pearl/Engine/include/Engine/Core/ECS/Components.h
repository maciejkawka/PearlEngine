#pragma once
#include"Core/ECS/BaseComponent.h"


//Renderer
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Light.h"

//Include Components
#include"Core/ECS/Components/CoreComponents.h"
#include"Core/ECS/Components/TransformComponent.h"

namespace PrCore::ECS {

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
	class TimeComponent : public BaseComponent
	{
	public:
		float time = 0;
	};
}
