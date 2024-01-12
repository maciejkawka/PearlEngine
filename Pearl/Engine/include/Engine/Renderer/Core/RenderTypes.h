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
		PrCore::Math::mat4                     lightMat;
		std::vector<PrCore::Math::mat4>        lightViewMats;
		size_t                                 shadowMapPos;
		size_t                                 id;
	};

	using LightObjectPtr = std::shared_ptr<LightObject>;

	enum class RendererFlag {
		None = 0,
		RerenderCubeMap = 1
	};
	DEFINE_ENUM_FLAG_OPERATORS(RendererFlag);

	struct RendererSettings {

		//Shadows
		size_t             dirLightMaxShadows = 8;
		size_t             dirLightShadowsMapSize = 2048;
		size_t             dirLightCombineMapSize = 12288;
		float              dirLightShadowBias = 0.005f;
		float              dirLightCascadeExtend = 6.0f;

		size_t             pointLightMaxShadows = 16;
		size_t             pointLightShadowMapSize = 1024;
		size_t             pointLightCombineShadowMapSize = 10240;
		float              pointLightShadowBias = 0.4f;

		size_t             spotLightMaxShadows = 16;
		size_t             spotLightShadowMapSize = 1024;
		size_t             spotLightCombineShadowMapSize = 4096;
		float              spotLightShadowBias = 0.005f;

		size_t             mainLightShadowMapSize = 4096;
		size_t             mainLightShadowCombineMapSize = 8196;
		float              mainLightShadowBias = 0.005f;
		float              cascadeShadowBorders[SHADOW_CASCADES_COUNT];
		float              cascadeShadowBordersCamSpace[SHADOW_CASCADES_COUNT];
		float              cascadeShadowRadiusRatio[SHADOW_CASCADES_COUNT];
		float              mainLightBlendDist = 5.0f;
		float              mainLightCascadeExtend = 6.0f;

		// Ambient
		PrCore::Math::vec3 ambientColor;
		PrCore::Math::vec3 skyColor = { 0.5f, 0.5f, 0.5f };

		// Fog
		bool               enableFog = false;
		PrCore::Math::vec3 fogColor = {0.5f, 0.5f, 0.5f};
		float              fogDencity = 1.0f;
		float              fogMaxDistance = 50.0f;

		// FXAA
		bool               enableFXAAA = false;
		float              FXAAThreasholdMax = 0.0312f;
		float              FXAAThreasholdMin = 0.125f;
		float              FXAAEdgeIterations = 16;
		float              FXAASubpixelQuiality = 0.75f;

		// SSAO
		bool               enableSSAO = false;
		float              SSAOKenrelSize = 16;
		float              SSAORadius = 0.5f;
		float              SSAObias   = 0.1f;
		float              SSAOMagnitude = 1.1f;
		int                SSAOBlureSize = 4;

		// Bloom
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