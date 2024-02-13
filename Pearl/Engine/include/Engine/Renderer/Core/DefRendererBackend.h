#pragma once
#include "Renderer/Core/IRendererBackend.h"
#include "Renderer/Core/RenderCommand.h"
#include "Renderer/Core/CascadeShadowMapper.h"

#include "Renderer/Buffers/Framebuffer.h"
#include <list>

namespace PrRenderer::Core {

	class DefRendererBackend : public IRendererBackend {
	public:
		explicit DefRendererBackend(RendererSettingsPtr& p_settings);

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
			Resources::TexturePtr aoTex; // Emissive (RGB) + AO (A)
		};

		struct RenderContext {

			gBuffer                             gBuffer;

			// PBR Lighting
			Buffers::FramebuffferPtr            otuputBuff;
			Resources::TexturePtr               outputTex;
			Resources::CubemapPtr               IRMap;
			Resources::CubemapPtr               prefilterMap;
			Resources::TexturePtr               brdfLUT;

			// SSAO
			Buffers::FramebuffferPtr            SSAOBuff;
			Resources::TexturePtr               SSAOTex;
			Resources::TexturePtr               SSAONoiseTex;
			std::vector<PrCore::Math::vec3>     ssaoKernel;

			// Postprocess 
			Buffers::FramebuffferPtr            postprocessBuff;
			Resources::TexturePtr               postprocessTex;
			Buffers::FramebuffferPtr            bloomDownscaleBuff[BLOOM_SIZE];
			Resources::TexturePtr               bloomDownscaleTex[BLOOM_SIZE];
			Buffers::FramebuffferPtr            bloomBuff;
			Resources::TexturePtr               bloomTex;

			// Shadow mapping
			// One point light uses 6 subparts of the texture, so number of lights = TextureSize / (ShadowMapTexture * 6)
			Buffers::FramebuffferPtr            shadowMapPointBuff;
			Resources::TexturePtr               shadowMapPointTex;

			Buffers::FramebuffferPtr            shadowMapSpotBuff;
			Resources::TexturePtr               shadowMapSpotTex;

			Buffers::FramebuffferPtr            shadowMapDirBuff;
			Resources::TexturePtr               shadowMapDirTex;

			Buffers::FramebuffferPtr            shadowMapMainDirBuff;
			Resources::TexturePtr               shadowMapMainDirTex;

			//Aux
			Camera*                             camera;
			Resources::MeshPtr                  m_quadMesh;
			RendererSettingsPtr                 m_settings;
			FrameInfo*                          frameInfo;
		};

		//Render Commands
		static void RenderOpaque(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderOpaque, RenderOpaque, RenderObjectPtr, RenderContext*);

		static void RenderToShadowMap(Resources::ShaderPtr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToShadowMap, RenderToShadowMap, Resources::ShaderPtr, PrCore::Math::mat4, std::list<RenderObjectPtr>*, RenderContext*);

		static void RenderToPointShadowMap(Resources::ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightMatrix, PrCore::Math::vec3& p_lightPos, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToPointShadowMap, RenderToPointShadowMap, Resources::ShaderPtr, PrCore::Math::mat4, PrCore::Math::vec3, std::list<RenderObjectPtr>*, RenderContext*);

		static void RenderLight(Resources::ShaderPtr p_lightShdr, DirLightObjectPtr p_mianDirectLight, std::vector<LightObjectPtr>* p_lights, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderLight, RenderLight, Resources::ShaderPtr, DirLightObjectPtr, std::vector<LightObjectPtr>*, const RenderContext*);

		static void RenderCubeMap(Resources::MaterialPtr p_material, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderCubeMap, RenderCubeMap, Resources::MaterialPtr, const RenderContext*);

		static void RenderBackBuffer(Resources::ShaderPtr p_postProcessShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderBackBuffer, RenderBackBuffer, Resources::ShaderPtr, const RenderContext*);

		static void RenderTransparent(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderTransparent, RenderTransparent, RenderObjectPtr, RenderContext*);

		static void RenderSSAO(Resources::ShaderPtr p_SSAOShader, Resources::ShaderPtr p_BlurSSAOShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderSSAO, RenderSSAO, Resources::ShaderPtr, Resources::ShaderPtr, RenderContext*);

		static void RenderFXAA(Resources::ShaderPtr p_FXAAShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFXAA, RenderFXAA, Resources::ShaderPtr, RenderContext*);

		static void RenderFog(Resources::ShaderPtr p_fogShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFog, RenderFog, Resources::ShaderPtr, RenderContext*);

		static void RenderBloom(Resources::ShaderPtr p_downsample, Resources::ShaderPtr p_upsample, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderBloom, RenderBloom, Resources::ShaderPtr, Resources::ShaderPtr, RenderContext*);

		void GenerateSSAOContext();
		void GeneratePostprocessContext();
		void GenerategGBuffersContext();

		void GenerateShadowMaps();
		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();

		//Shadow Mapping
		PrCore::Math::vec4 CalculateLightTexture(size_t p_lightID, size_t p_lightMapSize, size_t p_comboMapSize);

		//Main Data
		//---------
		RenderContext m_renderContext;

		//Shaders
		Resources::ShaderPtr m_backBuffShdr;
		Resources::ShaderPtr m_pbrLightShdr;
		Resources::ShaderPtr m_shadowMappingShdr;
		Resources::ShaderPtr m_pointshadowMappingShdr;
		Resources::ShaderPtr m_SSAOShdr;
		Resources::ShaderPtr m_SSAOBlurShdr;
		Resources::ShaderPtr m_FXAAShdr;
		Resources::ShaderPtr m_fogShdr;
		Resources::ShaderPtr m_downsample;
		Resources::ShaderPtr m_upsample;

		CascadeShadowUtility m_CSMUtility;
	};

}