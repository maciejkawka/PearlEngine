#pragma once
#include "Renderer/Core/IRenderBackend.h"
#include "Renderer/Core/RenderCommand.h"
#include "Renderer/Core/CascadeShadowMapper.h"
#include "Renderer/Buffers/Framebuffer.h"

#include <list>

namespace PrRenderer {

	class DeferRenderBackend : public IRenderBackend {
	public:
		explicit DeferRenderBackend(RendererSettingsPtr& p_settings);

		~DeferRenderBackend() override = default;

		void PreparePipeline() override;
		void Render() override;
		void PostRender() override;

	private:
		void OnWindowResize(PrCore::EventPtr p_event);

		struct gBuffer
		{
			FramebuffferPtr buffer;
			TexturePtr positionTex; //Position (RGB) + Depth (A)
			TexturePtr albedoTex; //Albedo (RGB) + Roughness (A)
			TexturePtr normalsTex; // Normals (RGB) + Metalness (A)
			TexturePtr aoTex; // Emissive (RGB) + AO (A)
		};

		struct RenderContext {

			gBuffer                             gBuffer;

			// PBR Lighting
			FramebuffferPtr                     otuputBuff;
			TexturePtr                          outputTex;
			CubemapPtr                          IRMap;
			CubemapPtr                          prefilterMap;
			TexturePtr                          brdfLUT;

			// SSAO
			FramebuffferPtr                     SSAOBuff;
			TexturePtr                          SSAOTex;
			TexturePtr                          SSAONoiseTex;
			std::vector<PrCore::Math::vec3>     ssaoKernel;

			// Postprocess 
			FramebuffferPtr                     postprocessBuff;
			TexturePtr                          postprocessTex;
			FramebuffferPtr                     bloomDownscaleBuff[BLOOM_SIZE];
			TexturePtr                          bloomDownscaleTex[BLOOM_SIZE];
			FramebuffferPtr                     bloomBuff;
			TexturePtr                          bloomTex;

			// Shadow mapping
			// One point light uses 6 subparts of the texture, so number of lights = TextureSize / (ShadowMapTexture * 6)
			FramebuffferPtr                     shadowMapPointBuff;
			TexturePtr                          shadowMapPointTex;

			FramebuffferPtr                     shadowMapSpotBuff;
			TexturePtr                          shadowMapSpotTex;

			FramebuffferPtr                     shadowMapDirBuff;
			TexturePtr                          shadowMapDirTex;

			FramebuffferPtr                     shadowMapMainDirBuff;
			TexturePtr                          shadowMapMainDirTex;

			//Aux
			Camera*                             camera;
			MeshPtr                             quadMesh;
			RendererSettingsPtr                 settings;
			FrameInfo*                          frameInfo;
		};

		// Differed pipeline commands
		// Consider wrapping that with an objects in the future
		// Renders Opaque objects, uses a shader stored in object's material
		static void RenderOpaque(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderOpaque, RenderObjectPtr, RenderContext*);

		// Renders object into the directional and spotlight map
		static void RenderToShadowMap(ShaderPtr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToShadowMap, ShaderPtr, PrCore::Math::mat4, LightObjectPtr, std::list<RenderObjectPtr>*, RenderContext*);

		// Renders object into the point light map
		static void RenderToPointShadowMap(ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightView, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToPointShadowMap, ShaderPtr, PrCore::Math::mat4, LightObjectPtr, std::list<RenderObjectPtr>*, RenderContext*);

		// After objects are rendered into GBuffer this pass calculates PBR Ligthing, with global iluminance
		static void RenderLight(ShaderPtr p_lightShdr, DirLightObjectPtr p_mianDirectLight, std::vector<LightObjectPtr>* p_lights, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderLight, ShaderPtr, DirLightObjectPtr, std::vector<LightObjectPtr>*, const RenderContext*);

		// Renders cubemap in background
		static void RenderCubeMap(MaterialPtr p_material, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderCubeMap, MaterialPtr, const RenderContext*);

		// Renders a tone mapping and exposure
		static void RenderToneMapping(ShaderPtr p_toneMapShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderToneMapping, ShaderPtr, const RenderContext*);

		// Renders a transparent objects using froward rendering. Uses global ilumination, does not affected by light or shadows
		static void RenderTransparent(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderTransparent, RenderObjectPtr, RenderContext*);

		// Renders SSAO
		static void RenderSSAO(ShaderPtr p_SSAOShader, ShaderPtr p_BlurSSAOShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderSSAO, ShaderPtr, ShaderPtr, RenderContext*);

		// Renders FXAA
		static void RenderFXAA(ShaderPtr p_FXAAShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFXAA, ShaderPtr, RenderContext*);

		// Renders a logarithmic fog, does not put a fog on transparent objects that are rendered on top of the skymap
		static void RenderFog(ShaderPtr p_fogShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFog, ShaderPtr, RenderContext*);

		// Renders bloom
		static void RenderBloom(ShaderPtr p_downsampleShader, ShaderPtr p_upsampleShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderBloom, ShaderPtr, ShaderPtr, RenderContext*);

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
		ShaderHandle m_ToneMappingShdr;
		ShaderHandle m_pbrLightShdr;
		ShaderHandle m_shadowMappingShdr;
		ShaderHandle m_pointshadowMappingShdr;
		ShaderHandle m_SSAOShdr;
		ShaderHandle m_SSAOBlurShdr;
		ShaderHandle m_FXAAShdr;
		ShaderHandle m_fogShdr;
		ShaderHandle m_downsample;
		ShaderHandle m_upsample;

		CascadeShadowUtility m_CSMUtility;
	};
}