#pragma once
#include "Renderer/Core/IRendererBackend.h"
#include "Renderer/Core/RenderCommand.h"
#include "Renderer/Core/CascadeShadowMapper.h"

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
			Resources::TexturePtr    m_shadowMapPointTex;
			//One light uses one subpart of the texture
			Buffers::FramebuffferPtr m_shadowMapOther;

			//Main light shadow mapping objects
			std::unique_ptr<CascadeShadowMapper>  CSM;
			Buffers::FramebuffferPtr m_cascadeShadow;
			Resources::TexturePtr     m_CSMShadowMap;

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

		static void RenderToPointShadowMap(Resources::ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightMatrix, PrCore::Math::vec3& p_lightPos, std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderToPointShadowMap, RenderToPointShadowMap, Resources::ShaderPtr, PrCore::Math::mat4, PrCore::Math::vec3, std::list<RenderObjectPtr>*, RenderData*);

		static void RenderLight(Resources::ShaderPtr p_lightShdr, LightObjectPtr mianDirectLight, std::vector<LightObject>* p_lightMats, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderLight, RenderLight, Resources::ShaderPtr, LightObjectPtr, std::vector<LightObject>*, const RenderData*);

		static void RenderCubeMap(Resources::MaterialPtr p_material, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderCubeMap, RenderCubeMap, Resources::MaterialPtr, const RenderData*);

		static void RenderPostProcess(Resources::ShaderPtr p_postProcessShader, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderPostProcess, RenderPostProcess, Resources::ShaderPtr, const RenderData*);

		static void RenderTransparent(RenderObjectPtr p_object, const RenderData* p_renderData);
		REGISTER_RENDER_COMMAND(RenderTransparent, RenderTransparent, RenderObjectPtr, RenderData*);

		void GenerategBuffers();
		void GenerateShadowMaps();

		void GenerateIRMap();
		void GeneratePrefilterMap();
		void GenerateLUTMap();

		//Shadow Mapping
		PrCore::Math::vec4 CalculatePointLightTexture(size_t p_lightID);
		//std::vector<glm::vec4> lightFrustrumCorners(const PrCore::Math::mat4& p_projectionMat);

		//Main Data
		//---------
		RenderData m_renderData;

		//Shaders
		Resources::ShaderPtr m_postProcesShdr;
		Resources::ShaderPtr m_pbrLightShader;
		Resources::ShaderPtr m_shadowMappingShader;
		Resources::ShaderPtr m_pointshadowMappingShader;
		
	};

}