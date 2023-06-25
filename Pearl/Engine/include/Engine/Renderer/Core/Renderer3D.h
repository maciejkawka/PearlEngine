#pragma once
#include"Renderer/Core/Camera.h"
#include"Renderer/Core/Light.h"
#include"Renderer/Core/RenderCommand.h"
#include"Renderer/Core/RenderObjectBuffer.h"

#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Material.h"

#include"Core/Math/Math.h"
#include"Core/Events/Event.h"
#include"Core/Utils/Singleton.h"

#include<queue>
#include<algorithm>

#define MAX_LIGHTNUM 4
#define MAX_OPAQUE_RENDERABLES 2500
#define MAX_TRANSPARENT_RENDERABLES 2500
#define MAX_INSTANCE_COUNT 200
#define MIN_INSTANCE_COUNT 5

namespace PrRenderer::Core {

	struct RenderData {
		std::vector<PrCore::Math::mat4>		lightData;
		Resources::CubemapPtr				IRMap;
		Resources::CubemapPtr				prefilterMap;
		Resources::TexturePtr				brdfLUT;
		PrCore::Math::vec3					ambientColor;
		Camera*								mainCamera;

		bool								hasCubeMap;
	};

	class Renderer3D: public PrCore::Utils::Singleton<Renderer3D> {
	public:		
		void SetCubemap(Resources::MaterialPtr p_cubemap);
		Resources::MaterialPtr GetCubemap() { return m_cubemap; }

		void SetMainCamera(Camera* p_camera);
		Camera* GetMainCamera() { return m_renderData.mainCamera; }

		inline void EnableInstancing(bool p_enable) { m_useInstancing = p_enable; }
		inline bool IsInstancingEnable() const { return m_useInstancing; }

		void AddLight(const PrCore::Math::mat4& p_lightmMat);
		void SetAmbientLight(Color p_ambientColor);

		void AddMeshRenderObject(MeshRenderObject&& p_meshRenderObject);
		void DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material);

		void Begin();
		void Render();
		void Flush();
		
	private:
		//using MeshRenderBuffer = RenderBuffer<MeshRenderObject, RenderSortingHash>;

		Renderer3D();

		//Events
		void OnWindowResize(PrCore::Events::EventPtr p_event);
		
		//PBR Maps
		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();

		//Instancing
		template<class MeshRenderBuffer, bool renderUninstanced = true>
		void InstantateMeshObjects(MeshRenderBuffer& p_meshRenderBuffer);

		//Other
		size_t CalculateDepthValue(const PrCore::Math::vec3& p_position) const;

		/////////////////////////////////

		//RenderData
		RenderData m_renderData;

		Resources::MaterialPtr m_cubemap;
		Resources::ShaderPtr m_instancingShader;
		bool m_useInstancing = true;

		/////////////////////////////////
		//Stored rendered objects
		//Mesh Objects
		RenderBuffer<MeshRenderObject, RenderSortingHash> m_opaqueObjects;
		RenderBuffer<MeshRenderObject, RenderSortingHash, RenderSortingHash::TransparenctySort> m_transparentObjects;

		//More objects in future

		/////////////////////////////////

		//RenderCommand
		using RenderCommandQueue = std::queue<RenderCommand*>;
		RenderCommandQueue m_RCQueue;

		friend Singleton<Renderer3D>;
	};
}

template<class MeshRenderBuffer, bool renderUninstanced>
void PrRenderer::Core::Renderer3D::InstantateMeshObjects(MeshRenderBuffer& p_meshRenderBuffer)
{
	std::vector<MeshRenderObject*> instancedObjCandidates;
	instancedObjCandidates.reserve(p_meshRenderBuffer.size());

	for (auto bufferIt = p_meshRenderBuffer.begin(); bufferIt != p_meshRenderBuffer.end();)
	{
		//First element Hash data
		auto& prorityHash = bufferIt->first;
		auto materialHash = prorityHash.GetMaterialHash();
		auto renderOrder = prorityHash.GetRenderOrder();

		//Find candidates to instantiate
		size_t instanceCount = 0;
		const auto innerItBegin = bufferIt;
		while (bufferIt != p_meshRenderBuffer.end() && bufferIt->first.GetMaterialHash() == materialHash)
		{ 
			//Is it a good candidate to instantiate
			auto candidateHash = bufferIt->first;
			if (candidateHash.GetMaterialHash() == materialHash && candidateHash.GetRenderOrder() == renderOrder)
			{
				instancedObjCandidates.push_back(bufferIt->second);
				instanceCount++;
			}
			++bufferIt;
		}

		//Check if worth instancing
		if (instanceCount > MIN_INSTANCE_COUNT)
		{
			//Grab common data for all instanced chunk
			Resources::MeshPtr mesh = instancedObjCandidates[0]->mesh;
			Resources::MaterialPtr material = instancedObjCandidates[0]->material;
			Resources::MaterialPtr instancedMaterial = std::make_shared<Resources::Material>(m_instancingShader);
			instancedMaterial->CopyPropertiesFrom(*material);

			std::vector<PrCore::Math::mat4> matrices;
			matrices.reserve(MAX_INSTANCE_COUNT);
			for (int i = 0; i < instanceCount; )
			{
				while (i < instancedObjCandidates.size() && matrices.size() < MAX_INSTANCE_COUNT)
				{
					matrices.push_back(instancedObjCandidates[i]->worldMat);
					i++;
				}

				InstancedMeshObject mehsObject{
					mesh,
					instancedMaterial,
					matrices.size(),
					std::move(matrices)
				};

				m_RCQueue.push(new InstancedMeshesRC(std::move(mehsObject), &m_renderData));
				matrices.clear();
			}

			//Erase instanced objects from the buffer
			bufferIt = p_meshRenderBuffer.erase(innerItBegin, bufferIt);
		}
		else
		{
			if constexpr (renderUninstanced == true)
			{
				//Render non instanced object in correct order
				for (auto object : instancedObjCandidates)
					m_RCQueue.push(new MeshRenderRC(std::move(*object), &m_renderData));
				bufferIt = p_meshRenderBuffer.erase(innerItBegin, bufferIt);
			}
		}

		instancedObjCandidates.clear();
	}
}