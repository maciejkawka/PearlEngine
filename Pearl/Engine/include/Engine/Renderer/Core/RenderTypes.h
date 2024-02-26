#pragma once
#include"Renderer/Resources/Material.h"
#include"Renderer/Core/Camera.h"
#include"Renderer/Resources/Mesh.h"
#include"Renderer/Resources/Light.h"

namespace PrRenderer::Core {

#define SHADOW_CASCADES_COUNT 4
#define MAX_LIGHT_COUNT 200
#define BLOOM_SIZE 4

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
		bool                   wiredframe = false;

		//Instanced to be moved
		size_t                 instanceSize;
		std::vector<PrCore::Math::mat4>    worldMatrices;


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
	//-------------------------------------
	struct LightObject {
		// Light Matrix packed for a shader
		PrCore::Math::mat4     packedMat; 

		// Light ID taken from EntityID
		size_t                 id; 

		// Shadow map contain all lights, this is position in the texture
		// Set to -1 by default to indicate that light does not cast a shadow
		int                    shadowMapPos = -1; 

		// Light can skip shadow casting
		bool                   castShadow = false; 

		const PrCore::Math::vec3 GetPosition()
		{
			return { packedMat[0][0], packedMat[0][1], packedMat[0][2] };
		}

		Resources::LightType GetType()
		{
			return static_cast<Resources::LightType>(packedMat[0][3]);
		}

		const PrCore::Math::vec3 GetDirection()
		{
			return { packedMat[1][0], packedMat[1][1], packedMat[1][2] };
		}

		float GetInnerCone()
		{
			return packedMat[1][3];
		}

		float GetOuterCone()
		{
			return packedMat[2][3];
		}

		const PrCore::Math::vec3 GetColor()
		{
			return { packedMat[2][0], packedMat[2][1], packedMat[2][2] };
		}

		float GetRange()
		{
			return packedMat[3][3];
		}
	};
	using LightObjectPtr = std::shared_ptr<LightObject>;

	struct DirLightObject : public LightObject {
		std::vector<PrCore::Math::mat4>        viewMatrices; // Directional light uses CSM, it has the CSM cascades number
	};
	using DirLightObjectPtr = std::shared_ptr<DirLightObject>;

	struct SpotLightObject : public LightObject {
		PrCore::Math::mat4       viewMatrix; // Directional light uses CSM, it has the CSM cascades number
	};
	using SpotLightObjectPtr = std::shared_ptr<SpotLightObject>;

	//-------------------------------------

	// Flag can be set in a frame to trigger a behaviour
	enum class RendererFlag: std::uint64_t {
		None = 0,
		RerenderCubeMap = 1,
		ForceStripframe = 2,
		CameraPerspectiveRecalculate = 4,
	};
	DEFINE_ENUM_FLAG_OPERATORS(RendererFlag);

	//Backend renderer is a settings owner, clinets can recieve reference only
	struct RendererSettings {

		//Shadows
		size_t             dirLightMaxShadows = 8;
		size_t             dirLightShadowsMapSize = 2048;
		size_t             dirLightCombineMapSize = 12288; // A size to contain all light shadow maps size * mapSize
		float              dirLightShadowBias = 0.005f;
		float              dirLightCascadeExtend = 6.0f;
		float              dirLightSize = 3.0f;

		size_t             pointLightMaxShadows = 16;
		size_t             pointLightShadowMapSize = 1024;
		size_t             pointLightCombineShadowMapSize = 10240; // A size to contain all light shadow maps size * mapSize
		float              pointLightShadowBias = 0.4f;
		float              pointLightSize = 6.0f;

		size_t             spotLightMaxShadows = 16;
		size_t             spotLightShadowMapSize = 1024;
		size_t             spotLightCombineShadowMapSize = 4096; // A size to contain all light shadow maps size * mapSize
		float              spotLightShadowBias = 0.005f;
		float              spotLightSize = 1.0f;

		size_t             mainLightShadowMapSize = 4096;
		size_t             mainLightShadowCombineMapSize = 8196;
		float              mainLightShadowBias = 0.005f;
		float              mainLightSize = 3.0f;
		float              cascadeShadowBorders[SHADOW_CASCADES_COUNT];
		float              cascadeShadowBordersCamSpace[SHADOW_CASCADES_COUNT];
		float              cascadeShadowRadiusRatio[SHADOW_CASCADES_COUNT];
		float              mainLightBlendDist = 5.0f;
		float              mainLightCascadeExtend = 6.0f;

		// Ambient
		PrCore::Math::vec3 ambientColor = { 0.0f, 0.0f, 0.0f };
		float              ambientIntensity = 1.0f;
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
		bool               enableBloom = true;
		float              bloomThreshold = 1.4f;
		float              bloomKnee = 0.1f;
		
		// General
		bool               enableInstancing = true;

		//Tone Mapping
		float              toneMappingExposure = 1.0f;

	};
	using RendererSettingsPtr = std::shared_ptr<RendererSettings>;

	///////////////////////////////////////


	//Frame Types
	///////////////////////////////////////
	struct FrameInfo {
		// Frame ID increased every frame
		std::uint64_t                               frameID;

		// Time when frame is built
		float                                       frameTimeStamp;

		// Custom timings with name, time in ms
		// Ex. SSAO 5ms dutation
		std::list<std::pair<std::string, float>>    timeEvents;

		// Number of drawcalls in a frame, only counts objects drawcalls
		size_t                                      objectDrawCalls;

		// Number of all drawcall in a frame, counts shadowmapping and postprocessing
		size_t                                      drawCalls;

		// Number of drawn objects in a frame
		// Does not count shadow mapping drawcalls
		size_t                                      drawObjects;

		// Number of drawn triangles in a frame
		size_t                                      drawTriangles;

		// Number of instanced objects
		// Ex. 100 Cubes reduced to 2 instanced calls == 100 instanced objects
		size_t                                      instancedObjects;

		// Number of discarded objects due to frustrum cullings
		size_t                                      culledObjects;

		void Reset()
		{
			frameID = 0;
			frameTimeStamp = 0;
			timeEvents.clear();
			drawCalls = 0;
			objectDrawCalls = 0;
			drawObjects = 0;
			drawTriangles = 0;
			instancedObjects = 0;
			culledObjects = 0;
		}
	};

	using RenderObjectVector = std::list<RenderObjectPtr>;

	struct FrameData {
		//Renderables
		RenderObjectVector              shadowCasters;
		RenderObjectVector              opaqueObjects;
		RenderObjectVector              transpatrentObjects;
		RenderObjectPtr	                cubemapObject;
		RenderObjectVector              debugObjects;
		Camera*                         camera;

		//Lighs and shadows
		std::vector<LightObjectPtr>     lights;
		std::shared_ptr<DirLightObject> mainDirectLight;


		//Aux
		RendererFlag                    renderFlag;
		FrameInfo                       frameInfo;
		////Lock in future
	};

	using FrameDataPtr = std::shared_ptr<FrameData>;
}