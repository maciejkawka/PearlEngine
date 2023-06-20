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
		class RenderSortingHash {
		public:
			RenderSortingHash(const MeshRenderObject& p_renderObject)
			{

				std::uint32_t materialHash = std::hash<Resources::MaterialPtr>{}(p_renderObject.material);
				std::uint8_t renderOrder = p_renderObject.material->GetRenderOrder();

				m_hash = (uint64_t)materialHash << 32 | (uint64_t)50 << 24;
			}

			void SetDepth(uint32_t p_depth)
			{
				m_hash |= p_depth >> 8;
			}

			inline size_t GetRenderOrder() const
			{
				return (m_hash & 0xFF000000) >> 24;
			}
			inline size_t GetMaterialHash() const
			{
				return m_hash >> 32;
			}
			inline size_t GetDepth() const
			{
				return m_hash & 0xFFFFFF;
			}

			bool operator<(const RenderSortingHash& rhs) const
			{
				return this->m_hash < rhs.m_hash;
			}


			bool operator==(const RenderSortingHash& rhs) const
			{
				return this->m_hash == rhs.m_hash;
			}

		private:
			std::uint64_t m_hash;
		};

		using MeshObjectBuffer = std::vector<MeshRenderObject>;
		using SortPair = std::pair<RenderSortingHash, MeshObjectBuffer::iterator>;
		using MeshObjectPriority = std::vector<SortPair>;


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

		size_t CalculateDepthValue(const PrCore::Math::vec3& p_position)
		{
			auto distance = PrCore::Math::distance(p_position, m_mainCamera->GetPosition());
			return (distance - m_mainCamera->GetNear()) / (m_mainCamera->GetFar() - m_mainCamera->GetNear()) * 0xFFFFFFFF;
		}

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

	struct TransparenctySort
	{
		bool operator()(const Renderer3D::SortPair& a, const Renderer3D::SortPair& b) const
		{
			return a.first.GetDepth() > b.first.GetDepth();
		}
	};
}
