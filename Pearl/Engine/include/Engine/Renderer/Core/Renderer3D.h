#pragma once
#include"Renderer/Core/Camera.h"
#include"Renderer/Core/Light.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"

#include"Core/Math/Math.h"
#include"Core/Events/Event.h"

#define MAX_LIGHTNUM 4

namespace PrRenderer::Core {

	class Renderer3D {
	public:
		Renderer3D();
		~Renderer3D() = default;

		void Begin();

		void SetCubemap(PrRenderer::Resources::MaterialPtr p_cubemap);
		Resources::MaterialPtr GetCubemap() { return m_cubemap; }
		Resources::CubemapPtr GetIRMap() { return m_IRMap; }
		Resources::CubemapPtr GetPrefiltered() { return m_prefilteredMap; }
		Resources::TexturePtr GetLUT() { return m_LUTMap; }
		
		void AddLight(const Light& p_light);
		void SetAmbientLight(Color p_ambientColor);

		//void DrawMesh(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, Resources::MaterialPtr p_material) {}
		void DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material);

		void Flush();

	private:
		void OnWindowResize(PrCore::Events::EventPtr p_event);

		void DrawCubemap();
		
		//PBR Maps
		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();

		Resources::MaterialPtr m_cubemap;

		//PBR Maps
		Resources::CubemapPtr m_IRMap;
		Resources::CubemapPtr m_prefilteredMap;
		Resources::TexturePtr m_LUTMap;

		std::vector<PrCore::Math::mat4> m_lightData;
		PrCore::Math::vec3 m_color;

		Resources::MeshPtr m_quad;
	};

}
