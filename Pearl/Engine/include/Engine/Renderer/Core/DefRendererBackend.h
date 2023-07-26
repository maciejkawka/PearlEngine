#pragma once
#include "Renderer/Core/IRendererBackend.h"
#include "Renderer/Core/RenderCommand.h"

#include "Renderer/Buffers/Framebuffer.h"
#include <list>

namespace PrRenderer::Core {

	class DefRendererBackend : public IRendererBackend {
	public:
		explicit DefRendererBackend(const RendererSettings& p_settings);

		~DefRendererBackend()  override = default;
		void PreRender() override;
		void Render() override;
		void PostRender() override;

	private:
		void OnWindowResize(PrCore::Events::EventPtr p_event);

		struct gBuffer
		{
			Buffers::FramebuffferPtr buffer;
			Resources::TexturePtr positionTex; //Position (RGB) + Depth (A)
			Resources::TexturePtr albedoTex; //Albedo (RGB) + Roughness (A)
			Resources::TexturePtr normalsTex; // Normals (RGB) + Metalness (A)
			Resources::TexturePtr aoTex; // To decide (RGB) + AO (A)
		};

		struct RenderData {

			gBuffer gBuffer;

			//PBR Lighting
			Buffers::FramebuffferPtr            postProccesBuff;
			Resources::TexturePtr               postProccesTex;
			Resources::CubemapPtr               IRMap;
			Resources::CubemapPtr               prefilterMap;
			Resources::TexturePtr               brdfLUT;

			Camera* camera;

			//Shadow mapping
			//One point light uses 6 subparts of the texture, so number of lights = TextureSize / (ShadowMapTexture * 6)
			Buffers::FramebuffferPtr m_shadowMapPoint;
			//One light uses one subpart of the texture
			Buffers::FramebuffferPtr m_shadowMapOther;
			//This texture is for main directional sun light
			Buffers::FramebuffferPtr m_cascadeShadow;

			//Aux
			Resources::MeshPtr m_quadMesh;
		};

		//Render Commands
		static void RenderToGBuffer(RenderObjectPtr p_object, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToGBuffer, RenderToGBuffer, RenderObjectPtr, RenderData*);

		static void RenderToShadowMap(Resources::ShaderPtr p_shadowsShdr, const LightObject& p_lightObject, const std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToShadowMap, RenderToShadowMap, Resources::ShaderPtr, LightObject, std::list<RenderObjectPtr>*, RenderData*);

		static void RenderToCascadeShadowMap(Resources::ShaderPtr p_cascadesShadowsShdr, PrCore::Math::mat4& p_lightMatrix, std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToCascadeShadowMap, RenderToCascadeShadowMap, Resources::ShaderPtr, PrCore::Math::mat4, std::list<RenderObjectPtr>*, RenderData*);

		static void RenderLight(Resources::ShaderPtr p_lightShdr, std::vector<LightObject>* p_lightMats, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderLight, RenderLight, Resources::ShaderPtr, std::vector<LightObject>*, const RenderData*);

		static void RenderCubeMap(Resources::MaterialPtr p_material, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderCubeMap, RenderCubeMap, Resources::MaterialPtr, const RenderData*);

		static void RenderPostProcess(Resources::ShaderPtr p_postProcessShader, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderPostProcess, RenderPostProcess, Resources::ShaderPtr, const RenderData*);

		static void RenderTransparent(RenderObjectPtr p_object, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderTransparent, RenderTransparent, RenderObjectPtr, RenderData*);
		//--------------

		//Shadow Mapping
		PrCore::Math::mat4 CalculateCSMFrusturmCorners(const PrCore::Math::vec3& p_lightDir, const PrCore::Math::mat4& p_cameraProjMat);
		PrCore::Math::vec4 CalculateShadowMapCoords(size_t p_index, size_t p_mapSize, size_t p_combinedMapSize);


		void GenerategBuffers();
		void GenerateShadowMaps();

		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();

		//Cascade shadow mapping
		//std::vector<glm::vec4> lightFrustrumCorners(const PrCore::Math::mat4& p_projectionMat);

		//Main Data
		//---------
		RenderData m_renderData;

		//Shaders
		Resources::ShaderPtr m_postProcesShdr;
		Resources::ShaderPtr m_pbrLightShader;
		Resources::ShaderPtr m_shadowMappingShader;
		
	};

}