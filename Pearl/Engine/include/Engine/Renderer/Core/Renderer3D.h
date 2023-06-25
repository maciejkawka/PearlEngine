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

#include"Core/Resources/ResourceLoader.h"

#include<queue>
#include<algorithm>

#define MAX_LIGHTNUM 4
#define MAX_OPAQUE_RENDERABLES 2500
#define MAX_TRANSPARENT_RENDERABLES 2500
#define MAX_INSTANCE_COUNT 200
#define MIN_INSTANCE_COUNT 1

namespace PrRenderer::Core {

	class Renderer3D: public PrCore::Utils::Singleton<Renderer3D> {
	public:
		
		//using MeshObjectBuffer = std::vector<MeshRenderObject>;
		//using SortPair = std::pair<RenderSortingHash, MeshObjectBuffer::iterator>;
		//using MeshObjectPriority = std::vector<SortPair>;


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

		//Instancing
		inline void CreateInstancesOpaqueMesh();

		//Other
		size_t CalculateDepthValue(const PrCore::Math::vec3& p_position);
		Resources::ShaderPtr m_instancingShader;
		/////////////////////////////////
		
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
		RenderBuffer<MeshRenderObject, RenderSortingHash> m_opaqueObjects;
		RenderBuffer<MeshRenderObject, RenderSortingHash> m_transparentObjects;
		//MeshObjectBuffer m_opaqueMeshObjects;
		//MeshObjectBuffer m_transparentMeshObjects;
		//MeshObjectPriority m_opaqueMeshPriority;
		//MeshObjectPriority m_transparentMeshPriority;

		//More objects in future

		////

		//RenderCommand
		using RenderCommandQueue = std::queue<RenderCommand*>;
		RenderCommandQueue m_RCQueue;

		friend Singleton<Renderer3D>;
	};
}

void PrRenderer::Core::Renderer3D::CreateInstancesOpaqueMesh()
{
	//Inital variables
	//auto instancedShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("PBR/PBRwithIR_Instanced.shader");
	//PR_ASSERT(instancedShader != nullptr, "Instance shader was not found");

	std::vector<MeshRenderObject*> instancedObjCandidates;
	instancedObjCandidates.reserve(m_opaqueObjects.size());

	RenderData renderData{
	m_lightData,
	m_IRMap,
	m_prefilteredMap,
	m_LUTMap,
	m_color,
	false
	};

	if (m_prefilteredMap && m_IRMap && m_LUTMap != nullptr)
		renderData.hasCubeMap = true;

	for (auto bufferIt = m_opaqueObjects.begin(); bufferIt != m_opaqueObjects.end();)
	{
		//First element Hash data
		auto& prorityHash = bufferIt->first;
		auto materialHash = prorityHash.GetMaterialHash();
		auto renderOrder = prorityHash.GetRenderOrder();

		//Find candidates to instance
		size_t instanceCount = 0;
		const auto innerItBegin = bufferIt;
		while (bufferIt != m_opaqueObjects.end() && bufferIt->first.GetMaterialHash() == materialHash)
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
			//Grab all data
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

				m_RCQueue.push(new InstancedMeshesRC(std::move(mehsObject), renderData));
				matrices.clear();
			}

			//Erase instanced objects from the buffer
			bufferIt = m_opaqueObjects.erase(innerItBegin, bufferIt);
		}
		else
		{
			//Render non instanced object in correct order
		}

		instancedObjCandidates.clear();
	}
}