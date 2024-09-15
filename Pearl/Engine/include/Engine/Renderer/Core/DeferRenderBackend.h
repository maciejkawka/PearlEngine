#pragma once
#include "Renderer/Core/IRenderBackend.h"
#include "Renderer/Core/RenderCommand.h"
#include "Renderer/Core/CascadeShadowMapper.h"
#include "Renderer/Buffers/Framebuffer.h"

#include <list>

namespace PrRenderer::Core {

	class DeferRenderBackend : public IRenderBackend {
	public:
		explicit DeferRenderBackend(RendererSettingsPtr& p_settings);

		~DeferRenderBackend() override = default;

		void PreparePipeline() override;
		void Render() override;
		void PostRender() override;

	private:
		void OnWindowResize(PrCore::Events::EventPtr p_event);

		struct gBuffer
		{
			Buffers::FramebuffferPtr buffer;
			Resources::Texturev2Ptr positionTex; //Position (RGB) + Depth (A)
			Resources::Texturev2Ptr albedoTex; //Albedo (RGB) + Roughness (A)
			Resources::Texturev2Ptr normalsTex; // Normals (RGB) + Metalness (A)
			Resources::Texturev2Ptr aoTex; // Emissive (RGB) + AO (A)
		};

		struct RenderContext {

			gBuffer                             gBuffer;

			// PBR Lighting
			Buffers::FramebuffferPtr            otuputBuff;
			Resources::Texturev2Ptr               outputTex;
			Resources::Cubemapv2Ptr               IRMap;
			Resources::Cubemapv2Ptr               prefilterMap;
			Resources::Texturev2Ptr               brdfLUT;

			// SSAO
			Buffers::FramebuffferPtr            SSAOBuff;
			Resources::Texturev2Ptr               SSAOTex;
			Resources::Texturev2Ptr               SSAONoiseTex;
			std::vector<PrCore::Math::vec3>     ssaoKernel;

			// Postprocess 
			Buffers::FramebuffferPtr            postprocessBuff;
			Resources::Texturev2Ptr               postprocessTex;
			Buffers::FramebuffferPtr            bloomDownscaleBuff[BLOOM_SIZE];
			Resources::Texturev2Ptr               bloomDownscaleTex[BLOOM_SIZE];
			Buffers::FramebuffferPtr            bloomBuff;
			Resources::Texturev2Ptr               bloomTex;

			// Shadow mapping
			// One point light uses 6 subparts of the texture, so number of lights = TextureSize / (ShadowMapTexture * 6)
			Buffers::FramebuffferPtr            shadowMapPointBuff;
			Resources::Texturev2Ptr               shadowMapPointTex;

			Buffers::FramebuffferPtr            shadowMapSpotBuff;
			Resources::Texturev2Ptr               shadowMapSpotTex;

			Buffers::FramebuffferPtr            shadowMapDirBuff;
			Resources::Texturev2Ptr               shadowMapDirTex;

			Buffers::FramebuffferPtr            shadowMapMainDirBuff;
			Resources::Texturev2Ptr               shadowMapMainDirTex;

			//Aux
			Camera*                             camera;
			Resources::Meshv2Ptr                  quadMesh;
			RendererSettingsPtr                 settings;
			FrameInfo*                          frameInfo;
		};

		// Differed pipeline commands
		// Consider wrapping that with an objects in the future
		// Renders Opaque objects, uses a shader stored in object's material
		static void RenderOpaque(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderOpaque, RenderObjectPtr, RenderContext*);

		// Renders object into the directional and spotlight map
		static void RenderToShadowMap(Resources::Shaderv2Ptr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToShadowMap, Resources::Shaderv2Ptr, PrCore::Math::mat4, LightObjectPtr, std::list<RenderObjectPtr>*, RenderContext*);

		// Renders object into the point light map
		static void RenderToPointShadowMap(Resources::Shaderv2Ptr p_pointShadowMapShader, PrCore::Math::mat4& p_lightView, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToPointShadowMap, Resources::Shaderv2Ptr, PrCore::Math::mat4, LightObjectPtr, std::list<RenderObjectPtr>*, RenderContext*);

		// After objects are rendered into GBuffer this pass calculates PBR Ligthing, with global iluminance
		static void RenderLight(Resources::Shaderv2Ptr p_lightShdr, DirLightObjectPtr p_mianDirectLight, std::vector<LightObjectPtr>* p_lights, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderLight, Resources::Shaderv2Ptr, DirLightObjectPtr, std::vector<LightObjectPtr>*, const RenderContext*);

		// Renders cubemap in background
		static void RenderCubeMap(Resources::Materialv2Ptr p_material, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderCubeMap, Resources::Materialv2Ptr, const RenderContext*);

		// Renders a tone mapping and exposure
		static void RenderToneMapping(Resources::Shaderv2Ptr p_toneMapShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderToneMapping, Resources::Shaderv2Ptr, const RenderContext*);

		// Renders a transparent objects using froward rendering. Uses global ilumination, does not affected by light or shadows
		static void RenderTransparent(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderTransparent, RenderObjectPtr, RenderContext*);

		// Renders SSAO
		static void RenderSSAO(Resources::Shaderv2Ptr p_SSAOShader, Resources::Shaderv2Ptr p_BlurSSAOShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderSSAO, Resources::Shaderv2Ptr, Resources::Shaderv2Ptr, RenderContext*);

		// Renders FXAA
		static void RenderFXAA(Resources::Shaderv2Ptr p_FXAAShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFXAA, Resources::Shaderv2Ptr, RenderContext*);

		// Renders a logarithmic fog, does not put a fog on transparent objects that are rendered on top of the skymap
		static void RenderFog(Resources::Shaderv2Ptr p_fogShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFog, Resources::Shaderv2Ptr, RenderContext*);

		// Renders bloom
		static void RenderBloom(Resources::Shaderv2Ptr p_downsampleShader, Resources::Shaderv2Ptr p_upsampleShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderBloom, Resources::Shaderv2Ptr, Resources::Shaderv2Ptr, RenderContext*);

		// Draw Debug
		static void RenderDebug(RenderObjectVector* p_debugObjects, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderDebug, RenderObjectVector*, RenderContext*);

		void GenerateSSAOContext();
		void GeneratePostprocessContext();
		void GenerategGBuffersContext();

		void GenerateShadowMaps();
		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();

		// Shadow Mapping
		PrCore::Math::vec4 CalculateLightTexture(size_t p_lightID, size_t p_lightMapSize, size_t p_comboMapSize);

		// Main Data
		RenderContext m_renderContext;

		// Shaders
		Resources::ShaderHandle m_ToneMappingShdr;
		Resources::ShaderHandle m_pbrLightShdr;
		Resources::ShaderHandle m_shadowMappingShdr;
		Resources::ShaderHandle m_pointshadowMappingShdr;
		Resources::ShaderHandle m_SSAOShdr;
		Resources::ShaderHandle m_SSAOBlurShdr;
		Resources::ShaderHandle m_FXAAShdr;
		Resources::ShaderHandle m_fogShdr;
		Resources::ShaderHandle m_downsample;
		Resources::ShaderHandle m_upsample;

		CascadeShadowUtility m_CSMUtility;
	};
}