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

		~DefRendererBackend() override = default;

		void PreparePipeline() override;
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
			Resources::MeshPtr                  quadMesh;
			RendererSettingsPtr                 settings;
			FrameInfo*                          frameInfo;
		};

		// Differed pipeline commands
		// Consider wrapping that with an objects in the future
		// Renders Opaque objects, uses a shader stored in object's material
		static void RenderOpaque(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderOpaque, RenderOpaque, RenderObjectPtr, RenderContext*);

		// Renders object into the directional and spotlight map
		static void RenderToShadowMap(Resources::ShaderPtr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToShadowMap, RenderToShadowMap, Resources::ShaderPtr, PrCore::Math::mat4, LightObjectPtr, std::list<RenderObjectPtr>*, RenderContext*);

		// Renders object into the point light map
		static void RenderToPointShadowMap(Resources::ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightView, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToPointShadowMap, RenderToPointShadowMap, Resources::ShaderPtr, PrCore::Math::mat4, LightObjectPtr, std::list<RenderObjectPtr>*, RenderContext*);

		// After objects are rendered into GBuffer this pass calculates PBR Ligthing, with global iluminance
		static void RenderLight(Resources::ShaderPtr p_lightShdr, DirLightObjectPtr p_mianDirectLight, std::vector<LightObjectPtr>* p_lights, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderLight, RenderLight, Resources::ShaderPtr, DirLightObjectPtr, std::vector<LightObjectPtr>*, const RenderContext*);

		// Renders cubemap in background
		static void RenderCubeMap(Resources::MaterialPtr p_material, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderCubeMap, RenderCubeMap, Resources::MaterialPtr, const RenderContext*);

		// Renders a tone mapping and exposure
		static void RenderToneMapping(Resources::ShaderPtr p_toneMapShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderToneMapping, RenderToneMapping, Resources::ShaderPtr, const RenderContext*);

		// Renders a transparent objects using froward rendering. Uses global ilumination, does not affected by light or shadows
		static void RenderTransparent(RenderObjectPtr p_object, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderTransparent, RenderTransparent, RenderObjectPtr, RenderContext*);

		// Renders SSAO
		static void RenderSSAO(Resources::ShaderPtr p_SSAOShader, Resources::ShaderPtr p_BlurSSAOShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderSSAO, RenderSSAO, Resources::ShaderPtr, Resources::ShaderPtr, RenderContext*);

		// Renders FXAA
		static void RenderFXAA(Resources::ShaderPtr p_FXAAShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFXAA, RenderFXAA, Resources::ShaderPtr, RenderContext*);

		// Renders a logarithmic fog, does not put a fog on transparent objects that are rendered on top of the skymap
		static void RenderFog(Resources::ShaderPtr p_fogShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderFog, RenderFog, Resources::ShaderPtr, RenderContext*);

		// Renders bloom
		static void RenderBloom(Resources::ShaderPtr p_downsampleShader, Resources::ShaderPtr p_upsampleShader, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderBloom, RenderBloom, Resources::ShaderPtr, Resources::ShaderPtr, RenderContext*);

		// Draw Debug
		static void RenderDebug(RenderObjectVector* p_debugObjects, const RenderContext* p_renderContext);
		REGISTER_RENDER_COMMAND(RenderDebug, RenderDebug, RenderObjectVector*, RenderContext*);

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
		Resources::ShaderPtr m_ToneMappingShdr;
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