#pragma once
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Camera.h"
#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::Core {

#define SHADOW_CASCADES_COUNT 4
#define MAX_LIGHT_COUNT 200

	struct RenderObject;

	class SortingHash {
	public:
		//Sorting Structs
		struct TransparenctySort
		{
			bool operator()(const SortingHash& a, const SortingHash& b) const
			{
				return a.GetDepth() > b.GetDepth();
			}
		};
		SortingHash() :
			m_hash(0)
		{}
		SortingHash(const RenderObject& p_renderObject);
		~SortingHash() = default;

		inline void SetDepth(size_t p_depth)
		{
			m_hash |= p_depth & 0xFFFFFF;
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

		inline bool operator<(const SortingHash& rhs) const
		{
			return this->m_hash < rhs.m_hash;
		}

		inline bool operator>(const SortingHash& rhs) const
		{
			return this->m_hash > rhs.m_hash;
		}

		inline bool operator==(const SortingHash& rhs) const
		{
			return this->m_hash == rhs.m_hash;
		}

	private:
		std::uint64_t m_hash;
	};

	const SortingHash INVALID_SORTINGHASH = SortingHash();

	///////////////////////////////////////

	//RenderObject
	///////////////////////////////////////

	enum class RenderObjectType {
		Mesh,
		InstancedMesh,
		CubeMap,
		TypeNum
	};


	struct RenderObject {
		RenderObjectType       type;
		size_t                 id;
		Resources::MaterialPtr material;
		Resources::MeshPtr     mesh;
		SortingHash            sortingHash;
		PrCore::Math::mat4     worldMat;

		//Instanced ot be moved
		size_t                 instanceSize;
		std::vector<PrCore::Math::mat4>    worldMatrices;

	public:

		//Sorting
		bool operator>(const RenderObject& rhs) const
		{
			return this->sortingHash > rhs.sortingHash;
		}

		bool operator<(const RenderObject& rhs) const
		{
			return this->sortingHash < rhs.sortingHash;
		}

		bool operator==(const RenderObject& rhs) const
		{
			return this->sortingHash == rhs.sortingHash;
		}
	};

	using RenderObjectPtr = std::shared_ptr<RenderObject>;

	struct NormalSort {
		bool operator()(const RenderObjectPtr& a, const RenderObjectPtr& b) const
		{
			return *a < *b;
		}
	};

	struct TransparentSort
	{
		bool operator()(const RenderObjectPtr& a, const RenderObjectPtr& b) const
		{
			return a->sortingHash.GetDepth() > b->sortingHash.GetDepth();
		}
	};

	///////////////////////////////////////


	//Light
	///////////////////////////////////////

	struct LightObject {
		PrCore::Math::mat4        lightMat;
		size_t                    shadowMapPos;
		size_t                    id;
	};

	using LightObjectPtr = std::shared_ptr<LightObject>;

	enum class RendererFlag {
		None = 0,
		RerenderAll = 1,
		RerenderLight = 2,
		RerenderCubeMap = 4,
		RecalculateProjectionMatrix = 8
	};
	DEFINE_ENUM_FLAG_OPERATORS(RendererFlag);

	struct RendererSettings {
		//Add Renderer settings



		//Shadows
		size_t pointLightShadowMapSize = 512;
		size_t lightShadowMapSize = 1024;
		size_t comboShadowMap = 8192;

		//CSM
		float  cascadeShadowBorders[SHADOW_CASCADES_COUNT];
		size_t cascadeShadowMapSize = 4096;
	};

	///////////////////////////////////////


	//Frame Types
	///////////////////////////////////////
	struct FrameInfo {
		size_t frameID;
		float  frameTimeStamp;

		size_t drawCallsCount;
		size_t instancedObjectsCount;
		size_t instanceCount;
		size_t culledObjectsCount;
	};

	using RenderObjectVector = std::list<RenderObjectPtr>;

	struct FrameData {
		//Renderables
		RenderObjectVector            shadowCasters;
		RenderObjectVector            opaqueObjects;
		RenderObjectVector            transpatrentObjects;
		RenderObjectPtr	              cubemapObject;
		RenderObjectVector            debugObject;
		Camera*                       camera;

		//Lighs and shadows
		std::vector<LightObject>      lights;
		LightObjectPtr                mainDirectLight;


		//Aux
		RendererFlag                  renderFlag;
		FrameInfo                     frameInfo;
		////Lock in future
	};

	using FrameDataPtr = std::shared_ptr<FrameData>;

}