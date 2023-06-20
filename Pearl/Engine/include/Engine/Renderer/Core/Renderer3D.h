#pragma once
#include"Renderer/Core/Camera.h"
#include"Renderer/Core/Light.h"
#include"Renderer/Core/RenderCommand.h"

#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"

#include"Core/Math/Math.h"
#include"Core/Events/Event.h"
#include"Core/Utils/Singleton.h"

#include<queue>

#define MAX_LIGHTNUM 4
#define MAX_OPAQUE_RENDERABLES 2500
#define MAX_TRANSPARENT_RENDERABLES 2500

namespace PrRenderer::Core {

	class Renderer3D: public PrCore::Utils::Singleton<Renderer3D> {
	public:
		void SetCubemap(Resources::MaterialPtr p_cubemap);
		Resources::MaterialPtr GetCubemap() { return m_cubemap; }

		void SetMainCamera(Camera* p_camera);
		Camera* GetMainCamera() { return m_mainCamera; }

		void AddLight(const PrCore::Math::mat4& p_lightmMat);
		void SetAmbientLight(Color p_ambientColor);

		void AddMeshRenderObject(MeshRenderObject&& p_meshRenderObject);
		void DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material);

		void Begin();
		void Render();
		void Flush();
		
	private:
		Renderer3D();

		//Events
		void OnWindowResize(PrCore::Events::EventPtr p_event);
		
		//PBR Maps
		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();


		//PBR Maps
		Resources::MaterialPtr m_cubemap;
		Resources::CubemapPtr m_IRMap;
		Resources::CubemapPtr m_prefilteredMap;
		Resources::TexturePtr m_LUTMap;

		std::vector<PrCore::Math::mat4> m_lightData;
		PrCore::Math::vec3 m_color;

		Camera* m_mainCamera;

		//Stored rendered objects
		//Mesh Objects
		using MeshObjectBuffer = std::vector<MeshRenderObject>;
		using MeshObjectPriority = std::vector<std::pair<uint64_t, MeshObjectBuffer::iterator>>;
		MeshObjectBuffer m_opaqueMeshObjects;
		MeshObjectBuffer m_transparentMeshObjects;
		MeshObjectPriority m_opaqueMeshPriority;
		MeshObjectPriority m_transparentMeshPriority;

		//More objects in future

		////

		//RenderCommand
		using RenderCommandQueue = std::queue<RenderCommand*>;
		RenderCommandQueue m_RCQueue;

		friend Singleton<Renderer3D>;
	};

}
