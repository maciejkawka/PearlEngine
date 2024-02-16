#include "Core/Common/pearl_pch.h"
#include "Renderer/Core/DefRendererBackend.h"

#include "Core/Events/WindowEvents.h"
#include "Renderer/Core/LowRenderer.h"
#include "Core/Resources/ResourceLoader.h"
#include "Core/Windowing/Window.h"
#include <Core/Events/EventManager.h>
#include <random>

#include "Core/Input/InputManager.h"
#include "Core/Utils/Clock.h"

namespace PrRenderer::Core
{
	DefRendererBackend::DefRendererBackend(RendererSettingsPtr& p_settings) :
		IRendererBackend(p_settings)
	{
		//Prepare shaders
		m_shadowMappingShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Shadows/ShadowMapping.shader");
		m_backBuffShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/RenderFront.shader");
		m_pbrLightShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/lightPass.shader");
		m_pointshadowMappingShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Shadows/PointShadowMapping.shader");
		m_SSAOShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/SSAO.shader");
		m_SSAOBlurShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/SSAO_Blur.shader");
		m_FXAAShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/FXAA.shader");
		m_fogShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/Fog.shader");
		m_downsample = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/downsample.shader");
		m_upsample = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/upsample.shader");

		m_renderContext.m_quadMesh = Resources::Mesh::CreatePrimitive(Resources::Quad);
		m_renderContext.m_settings = m_settings;

		GenerategGBuffersContext();
		GenerateShadowMaps();
		GenerateSSAOContext();
		GeneratePostprocessContext();

		//Set events
		PrCore::Events::EventListener windowResizedListener;
		windowResizedListener.connect<&DefRendererBackend::OnWindowResize>(this);
		PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	}

	void DefRendererBackend::PreRender()
	{
		//Temp variables
		const auto camera = m_frame->camera;
		m_renderContext.frameInfo = &m_frame->frameInfo;
		//Prepare camera
		m_renderContext.camera = camera;
		m_renderContext.camera->RecalculateMatrices();
		auto clock = PrCore::Utils::Clock::GetInstancePtr();

		//Pipeline
		//Clear Back Buffer
		PushCommand(CreateRC<LowRenderer::EnableDepthRC>(true));
		PushCommand(CreateRC<LowRenderer::EnableBlendingRC>(false));
		PushCommand(CreateRC<LowRenderer::EnableCullFaceRC>(true));
		PushCommand(CreateRC<LowRenderer::ClearRC>(ColorBuffer | DepthBuffer));

		//Calculate PBR Reflection if cubemap changed
		if((m_frame->renderFlag & RendererFlag::RerenderCubeMap) == RendererFlag::RerenderCubeMap && m_frame->cubemapObject)
		{
			SCOPE_HIGH_TIMER_CALLBACK(
				{ m_frame->frameInfo.timeEvents.push_back({ "CubeMapGeneration", time }); });

			GenerateIRMap();
			GeneratePrefilterMap();
			GenerateLUTMap();
		}

		// Reset global reflections if cubemap removed
		if (m_frame->cubemapObject == nullptr &&
			m_renderContext.IRMap != nullptr &&
			m_renderContext.prefilterMap != nullptr &&
			m_renderContext.brdfLUT != nullptr
			)
		{
			m_renderContext.IRMap->Unbind(4);
			m_renderContext.prefilterMap->Unbind(5);
			m_renderContext.brdfLUT->Unbind(6);

			m_renderContext.IRMap.reset();
			m_renderContext.prefilterMap.reset();
			m_renderContext.brdfLUT.reset();
		}

		// Shadow Mapping
		//---------------------------------
		TIME_RC_START(ShadowMapping);

		static bool calculateProjs = true;
		if(calculateProjs)
		{
			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
				m_settings->cascadeShadowBordersCamSpace[i] = m_settings->cascadeShadowBorders[i] * camera->GetFar();

			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), camera->GetFar() * m_settings->cascadeShadowBorders[0]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings->cascadeShadowBorders[0], camera->GetFar() * m_settings->cascadeShadowBorders[1]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings->cascadeShadowBorders[1], camera->GetFar() * m_settings->cascadeShadowBorders[2]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings->cascadeShadowBorders[2], camera->GetFar() * m_settings->cascadeShadowBorders[3]));

			calculateProjs = false;
		}

		// Main Directional Light
		if(m_frame->mainDirectLight && m_frame->mainDirectLight->castShadow)
		{
			const auto& mainLight = m_frame->mainDirectLight;
			const auto& lightDir = mainLight->GetDirection();

			PushCommand(CreateRC<LambdaFunctionRC>([&]
				{
					m_renderContext.shadowMapMainDirBuff->Bind();
					LowRenderer::Clear(ColorBuffer | DepthBuffer);
				}));

			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
			{
				auto lightMat = m_CSMUtility.ClaculateFrustrums(m_settings->cascadeShadowRadiusRatio[i] ,i, lightDir, camera->GetViewMatrix(), m_settings->mainLightShadowMapSize, m_settings->mainLightCascadeExtend);
				auto viewport = CalculateLightTexture(i, m_settings->mainLightShadowMapSize, m_settings->mainLightShadowCombineMapSize);
				mainLight->viewMatrices.push_back(lightMat);

				PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
				PushCommand(CreateRC<RenderToShadowMapRC>(m_shadowMappingShdr, lightMat, &m_frame->shadowCasters, &m_renderContext));
			}
		}

		// Directional Light
		PushCommand(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderContext.shadowMapDirBuff->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));

		for (auto& lightPtr : m_frame->lights)
		{
			if (lightPtr->GetType() != Resources::LightType::Directional)
				continue;

			if (!lightPtr->castShadow)
				continue;

			auto light = std::static_pointer_cast<DirLightObject>(lightPtr);
			auto lightDir = light->GetDirection();

			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
			{
				auto lightMat = m_CSMUtility.ClaculateFrustrums(m_settings->cascadeShadowRadiusRatio[i], i, lightDir, camera->GetViewMatrix(), m_settings->dirLightCombineMapSize, m_settings->dirLightCascadeExtend);
				auto viewport = CalculateLightTexture(light->shadowMapPos * SHADOW_CASCADES_COUNT + i, m_settings->dirLightShadowsMapSize, m_settings->dirLightCombineMapSize);
				light->viewMatrices.push_back(lightMat);

				PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
				PushCommand(CreateRC<RenderToShadowMapRC>(m_shadowMappingShdr, lightMat, &m_frame->shadowCasters, &m_renderContext));
			}

			m_settings->cascadeShadowRadiusRatio[1] = m_settings->cascadeShadowRadiusRatio[0] / m_settings->cascadeShadowRadiusRatio[1];
			m_settings->cascadeShadowRadiusRatio[2] = m_settings->cascadeShadowRadiusRatio[0] / m_settings->cascadeShadowRadiusRatio[2];
			m_settings->cascadeShadowRadiusRatio[3] = m_settings->cascadeShadowRadiusRatio[0] / m_settings->cascadeShadowRadiusRatio[3];
			m_settings->cascadeShadowRadiusRatio[0] = 1.0f;
		}


		// Point Lights
		PushCommand(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderContext.shadowMapPointBuff->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));

		for (auto light : m_frame->lights)
		{
			if (light->GetType() != Resources::LightType::Point)
				continue;

			if (!light->castShadow)
				continue;

			auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, 0.001f, light->GetRange() * 1.2f);
			auto lightPos = light->GetPosition();
			auto shadowMapPos = light->shadowMapPos;

			auto lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			auto viewport = CalculateLightTexture(shadowMapPos + 0, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(-1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(shadowMapPos + 1, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 1.0f, 0.0), PrCore::Math::vec3(0.0f, 0.0f, 1.0f));
			viewport = CalculateLightTexture(shadowMapPos + 2, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, -1.0f, 0.0f), PrCore::Math::vec3(0.0f, 0.0f, -1.0f));
			viewport = CalculateLightTexture(shadowMapPos + 3, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, 1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(shadowMapPos + 4, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, -1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(shadowMapPos + 5, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderContext));
		}
		

		// Spot Lights
		PushCommand(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderContext.shadowMapSpotBuff->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));

		for (auto& lightPtr : m_frame->lights)
		{
			if (lightPtr->GetType() != Resources::LightType::Spot)
				continue;

			if (!lightPtr->castShadow)
				continue;

			auto light = std::static_pointer_cast<SpotLightObject>(lightPtr);
			auto lightPos = light->GetPosition();
			auto lightDir = light->GetDirection();

			auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(120.0f), 1.0f, camera->GetNear(), camera->GetFar());
			auto lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + lightDir, PrCore::Math::vec3(0.0f, 0.0f, 1.0f));
			light->viewMatrix = lightProjMatrix * lightViewMatrix;

			auto viewport = CalculateLightTexture(light->shadowMapPos, m_settings->pointLightShadowMapSize, m_settings->spotLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToShadowMapRC>(m_shadowMappingShdr, lightProjMatrix * lightViewMatrix, &m_frame->shadowCasters, &m_renderContext));
		}

		PushCommand(CreateRC<LowRenderer::SetViewportRC>(PrCore::Windowing::Window::GetMainWindow().GetWidth(),
			PrCore::Windowing::Window::GetMainWindow().GetHeight(), 0, 0));

		TIME_RC_STOP(ShadowMapping);

		//---------------------------------

		// Opaque Objects
		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.gBuffer.buffer->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
				LowRenderer::EnableCullFace(true);
				LowRenderer::EnableDepth(true);
			}));

		TIME_RC_START(OpaquePass);
		for (auto object : m_frame->opaqueObjects)
			PushCommand(CreateRC<RenderOpaqueRC>(object, &m_renderContext));
		TIME_RC_STOP(OpaquePass);

		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.gBuffer.buffer->Unbind();
			}));

		//Clear output buffer
		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.otuputBuff->Bind();
				LowRenderer::EnableDepth(false);
				LowRenderer::EnableCullFace(false);
			}));

		// Clear Sky Color
		PushCommand(CreateRC<LowRenderer::ClearColorFloatRC>(m_settings->skyColor.x, m_settings->skyColor.y, m_settings->skyColor.z, 1.0f));
		PushCommand(CreateRC<LowRenderer::ClearRC>(ColorBuffer));
		PushCommand(CreateRC<LowRenderer::ClearColorFloatRC>(0.0f, 0.0f, 0.0f, 0.0f));

		// Draw Cubemap
		if(m_frame->cubemapObject)
			PushCommand(CreateRC<RenderCubeMapRC>(m_frame->cubemapObject->material, &m_renderContext));

		// Copy depth buffer to postprocess buffer
		PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.gBuffer.buffer, m_renderContext.otuputBuff, Buffers::FramebufferMask::DepthBufferBit));

		// SSAO
		if(m_settings->enableSSAO)
			PushCommand(CreateRC<RenderSSAORC>(m_SSAOShdr, m_SSAOBlurShdr, &m_renderContext));

		// PBR Light Pass
		TIME_RC_START(LightPass);
		PushCommand(CreateRC<RenderLightRC>(m_pbrLightShdr, m_frame->mainDirectLight, &m_frame->lights, &m_renderContext));
		TIME_RC_STOP(LightPass);

		//Transparent Forward Pass
		TIME_RC_START(TransparentPass);
		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.otuputBuff->Bind();
				LowRenderer::EnableDepth(true);
				LowRenderer::EnableCullFace(true);
				LowRenderer::EnableBlending(true);
				LowRenderer::SetBlendingAlgorythm(BlendingAlgorithm::SourceAlpha, BlendingAlgorithm::OneMinusSourceAlpha);
			}));
		for (auto object : m_frame->transpatrentObjects)
			PushCommand(CreateRC<RenderTransparentRC>(object, &m_renderContext));
		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.otuputBuff->Unbind();
				LowRenderer::EnableDepth(false);
				LowRenderer::EnableCullFace(false);
				LowRenderer::EnableBlending(false);
			}));
		TIME_RC_STOP(TransparentPass);
		//---------------------------------

		// Post Processing
		TIME_RC_START(PostProcessPass);

		// Fog
		if (m_settings->enableFog)
		{
			PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
			PushCommand(CreateRC<RenderFogRC>(m_fogShdr, &m_renderContext));
		}

		//Bloom
		if(m_settings->enableBloom)
		{
			PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
			PushCommand(CreateRC<RenderBloomRC>(m_downsample, m_upsample, &m_renderContext));
		}

		// Tone Mapping
		PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
		PushCommand(CreateRC<RenderBackBufferRC>(m_backBuffShdr, &m_renderContext));

		// FXAA Anti-Aliasing
		PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
		PushCommand(CreateRC<RenderFXAARC>(m_FXAAShdr, &m_renderContext));

		TIME_RC_STOP(PostProcessPass);
	}

	void DefRendererBackend::Render()
	{
		while (!m_commandQueue.empty())
		{
			m_commandQueue.front()->Invoke();
			m_commandQueue.pop_front();
		}
	}

	void DefRendererBackend::PostRender()
	{
	}

	void DefRendererBackend::OnWindowResize(PrCore::Events::EventPtr p_event)
	{
		auto windowResizeEvent = std::static_pointer_cast<PrCore::Events::WindowResizeEvent>(p_event);
		m_screenWidth = windowResizeEvent->m_width;
		m_screenHeight = windowResizeEvent->m_height;

		//Regenerate Buffers;
		GenerategGBuffersContext();
		GenerateSSAOContext();
		GeneratePostprocessContext();

		PushCommand(CreateRC<LowRenderer::SetViewportRC>(m_screenWidth, m_screenHeight, 0, 0));
	}

	void DefRendererBackend::RenderOpaque(RenderObjectPtr p_object, const RenderContext* p_renderContext)
	{
		auto mesh = p_object->mesh;
		auto material = p_object->material;
		auto shader = p_object->material->GetShader();

		p_renderContext->gBuffer.buffer->Bind();

		material->Bind();
		shader->SetUniformMat4("PIPELINE_VP_MAT", p_renderContext->camera->GetCameraMatrix());
		shader->SetUniformFloat("PIPELINE_NEAR", p_renderContext->camera->GetNear());
		shader->SetUniformFloat("PIPELINE_FAR", p_renderContext->camera->GetFar());

		if (p_object->type == RenderObjectType::Mesh)
		{
			shader->SetUniformMat4("PIPELINE_MODEL_MAT", p_object->worldMat);
			shader->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);

			mesh->Bind();
			LowRenderer::Draw(mesh->GetVertexArray());

			p_renderContext->frameInfo->drawTriangles += mesh->GetIndicesCount() / 3;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}
		else if(p_object->type == RenderObjectType::InstancedMesh)
		{
			shader->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", p_object->worldMatrices.data(), p_object->worldMatrices.size());
			shader->SetUniformInt("PIPELINE_INTANCE_COUNT", p_object->instanceSize);

			mesh->Bind();
			LowRenderer::DrawInstanced(mesh->GetVertexArray(), p_object->instanceSize);

			p_renderContext->frameInfo->drawTriangles += mesh->GetIndicesCount() / 3 * p_object->instanceSize;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}

		mesh->Unbind();
	}

	void DefRendererBackend::RenderToShadowMap(Resources::ShaderPtr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData)
	{
		p_shaderPtr->Bind();
		p_shaderPtr->SetUniformMat4("PIPELINE_LIGHT_MAT", p_lightMatrix);

		for (auto object : *p_objects)
		{
			if (object->type == RenderObjectType::Mesh)
			{
				p_shaderPtr->SetUniformMat4("PIPELINE_MODEL_MAT", object->worldMat);
				p_shaderPtr->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);
				object->mesh->Bind();
				LowRenderer::Draw(object->mesh->GetVertexArray());
				p_renderData->frameInfo->drawCalls++;
				object->mesh->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_shaderPtr->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_shaderPtr->SetUniformInt("PIPELINE_INTANCE_COUNT", object->instanceSize);
				object->mesh->Bind();
				LowRenderer::DrawInstanced(object->mesh->GetVertexArray(), object->instanceSize);
				p_renderData->frameInfo->drawCalls++;
				object->mesh->Unbind();
			}
		}

		p_shaderPtr->Unbind();
	}

	void DefRendererBackend::RenderToPointShadowMap(Resources::ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightMatrix, PrCore::Math::vec3& p_lightPos, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData)
	{
		p_pointShadowMapShader->Bind();
		p_pointShadowMapShader->SetUniformMat4("PIPELINE_LIGHT_MAT", p_lightMatrix);
		p_pointShadowMapShader->SetUniformVec3("PIPELINE_LIGHT_POS", p_lightPos);

		for (auto object : *p_objects)
		{
			if (object->type == RenderObjectType::Mesh)
			{
				p_pointShadowMapShader->SetUniformMat4("PIPELINE_MODEL_MAT", object->worldMat);
				p_pointShadowMapShader->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);
				object->mesh->Bind();
				LowRenderer::Draw(object->mesh->GetVertexArray());
				object->mesh->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_pointShadowMapShader->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_pointShadowMapShader->SetUniformInt("PIPELINE_INTANCE_COUNT", object->instanceSize);
				object->mesh->Bind();
				LowRenderer::DrawInstanced(object->mesh->GetVertexArray(), object->instanceSize);
				object->mesh->Unbind();
			}
		}

		p_pointShadowMapShader->Unbind();
	}

	void DefRendererBackend::GenerateSSAOContext()
	{
		m_renderContext.SSAOBuff.reset();
		m_renderContext.SSAOTex.reset();

		//Generate Kernel
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;
		std::vector<PrCore::Math::vec3> ssaoKernel;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = PrCore::Math::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0f;

			// scale samples s.t. they're more aligned to center of kernel
			scale = 0.1f + scale * scale * (1.0f - 0.1f);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
		m_renderContext.ssaoKernel = ssaoKernel;

		//Generate Noise
		std::vector<float> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise.x);
			ssaoNoise.push_back(noise.y);
			ssaoNoise.push_back(noise.z);
		}

		auto noiseTex = Resources::Texture2D::Create();
		noiseTex->SetHeight(4);
		noiseTex->SetWidth(4);
		noiseTex->SetFormat(Resources::TextureFormat::RGB16F);
		noiseTex->SetWrapModeU(Resources::TextureWrapMode::Repeat);
		noiseTex->SetWrapModeV(Resources::TextureWrapMode::Repeat);
		noiseTex->SetMagFiltering(Resources::TextureFiltering::Nearest);
		noiseTex->SetMinFiltering(Resources::TextureFiltering::Nearest);
		noiseTex->SetData(ssaoNoise.data());
		noiseTex->IsMipMapped(false);
		noiseTex->Apply();
		m_renderContext.SSAONoiseTex = noiseTex;

		//Generate SSAO Framebuffer
		m_renderContext.SSAOBuff.reset();

		Buffers::FramebufferTexture ssaoTex;
		ssaoTex.format = Resources::TextureFormat::R8;
		ssaoTex.filteringMag = Resources::TextureFiltering::Nearest;
		ssaoTex.filteringMin = Resources::TextureFiltering::Nearest;

		Buffers::FramebufferTexture depthTex;
		depthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		settings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();;
		settings.mipMaped = false;
		settings.colorTextureAttachments = { ssaoTex };
		settings.depthStencilAttachment = depthTex;

		m_renderContext.SSAOBuff = Buffers::Framebufffer::Create(settings);
		m_renderContext.SSAOTex = m_renderContext.SSAOBuff->GetTexturePtr();
	}

	void DefRendererBackend::GeneratePostprocessContext()
	{
		m_renderContext.postprocessBuff.reset();
		m_renderContext.postprocessTex.reset();

		Buffers::FramebufferTexture texture;
		texture.format = Resources::TextureFormat::RGB16F;

		Buffers::FramebufferTexture depthTex;
		depthTex.format = Resources::TextureFormat::Depth16;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		settings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();
		settings.mipMaped = false;
		settings.colorTextureAttachments = { texture };
		settings.depthStencilAttachment = depthTex;

		m_renderContext.postprocessBuff = Buffers::Framebufffer::Create(settings);
		m_renderContext.postprocessTex = m_renderContext.postprocessBuff->GetTexturePtr();

		// Bloom
		// downscale
		for (int i = 0; i < BLOOM_SIZE; i++)
		{
			m_renderContext.bloomDownscaleBuff[i].reset();
			m_renderContext.bloomDownscaleTex[i].reset();

			Buffers::FramebufferTexture bloomTexture;
			bloomTexture.format = Resources::TextureFormat::RGBA16F;
			bloomTexture.wrapModeU = Resources::TextureWrapMode::Clamp;
			bloomTexture.wrapModeV = Resources::TextureWrapMode::Clamp;

			Buffers::FramebufferTexture bloomDepthTex;
			bloomDepthTex.format = Resources::TextureFormat::Depth16;

			Buffers::FramebufferSettings bloomSettings;
			bloomSettings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> (i + 1);
			bloomSettings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> (i + 1);
			bloomSettings.colorTextureAttachments = { bloomTexture };
			bloomSettings.depthStencilAttachment = bloomDepthTex;

			m_renderContext.bloomDownscaleBuff[i] = Buffers::Framebufffer::Create(bloomSettings);
			m_renderContext.bloomDownscaleTex[i] = m_renderContext.bloomDownscaleBuff[i]->GetTexturePtr();
		}

		m_renderContext.bloomBuff.reset();
		m_renderContext.bloomTex.reset();

		Buffers::FramebufferTexture bloomTexture;
		bloomTexture.format = Resources::TextureFormat::RGBA16F;
		bloomTexture.wrapModeU = Resources::TextureWrapMode::Clamp;
		bloomTexture.wrapModeV = Resources::TextureWrapMode::Clamp;

		Buffers::FramebufferTexture bloomDepthTex;
		bloomDepthTex.format = Resources::TextureFormat::Depth16;

		Buffers::FramebufferSettings bloomSettings;
		bloomSettings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		bloomSettings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();
		bloomSettings.colorTextureAttachments = { bloomTexture };
		bloomSettings.depthStencilAttachment = bloomDepthTex;

		m_renderContext.bloomBuff = Buffers::Framebufffer::Create(bloomSettings);
		m_renderContext.bloomTex = m_renderContext.bloomBuff->GetTexturePtr();
	}

	void DefRendererBackend::GenerategGBuffersContext()
	{
		m_renderContext.gBuffer.positionTex.reset();
		m_renderContext.gBuffer.albedoTex.reset();
		m_renderContext.gBuffer.normalsTex.reset();
		m_renderContext.gBuffer.aoTex.reset();
		m_renderContext.gBuffer.buffer.reset();

		//Create new one
		Buffers::FramebufferTexture gPos;
		gPos.format = Resources::TextureFormat::RGBA16F;
		gPos.filteringMag = Resources::TextureFiltering::Nearest;
		gPos.filteringMin = Resources::TextureFiltering::Nearest;

		Buffers::FramebufferTexture gAlbedo;
		gAlbedo.format = Resources::TextureFormat::RGBA16F;


		Buffers::FramebufferTexture gNormal;
		gNormal.format = Resources::TextureFormat::RGBA16F;
		gNormal.filteringMag = Resources::TextureFiltering::Nearest;
		gNormal.filteringMin = Resources::TextureFiltering::Nearest;

		Buffers::FramebufferTexture gAo;
		gAo.format = Resources::TextureFormat::RGBA16F;


		Buffers::FramebufferTexture gDepth;
		gDepth.format = Resources::TextureFormat::Depth32;
		gDepth.filteringMag = Resources::TextureFiltering::Nearest;
		gDepth.filteringMin = Resources::TextureFiltering::Nearest;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth() * 2;
		settings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight() * 2;
		settings.mipMaped = false;
		settings.colorTextureAttachments = { gPos, gAlbedo, gNormal, gAo };
		settings.depthStencilAttachment = gDepth;

		auto framebuffer = Buffers::Framebufffer::Create(settings);
		m_renderContext.gBuffer.positionTex = framebuffer->GetTexturePtr(0);
		m_renderContext.gBuffer.albedoTex = framebuffer->GetTexturePtr(1);
		m_renderContext.gBuffer.normalsTex = framebuffer->GetTexturePtr(2);
		m_renderContext.gBuffer.aoTex = framebuffer->GetTexturePtr(3);
		m_renderContext.gBuffer.buffer = framebuffer;


		//Output framebuffer
		Buffers::FramebufferTexture outputTex;
		outputTex.format = Resources::TextureFormat::RGBA16F;

		Buffers::FramebufferTexture outputDepth;
		outputDepth.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings outputSettings;
		outputSettings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		outputSettings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();
		outputSettings.mipMaped = false;
		outputSettings.colorTextureAttachments = outputTex;
		outputSettings.depthStencilAttachment = outputDepth;

		m_renderContext.otuputBuff = Buffers::Framebufffer::Create(outputSettings);
		m_renderContext.outputTex = m_renderContext.otuputBuff->GetTexturePtr(0);
	}

	void DefRendererBackend::GenerateShadowMaps()
	{
		//Spot Lights Mapping
		Buffers::FramebufferTexture spotDepthTex;
		spotDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings spotLightSettings;
		spotLightSettings.globalWidth = m_settings->spotLightCombineShadowMapSize;
		spotLightSettings.globalHeight = m_settings->spotLightCombineShadowMapSize;;
		spotLightSettings.mipMaped = false;
		spotLightSettings.depthStencilAttachment = spotDepthTex;

		m_renderContext.shadowMapSpotBuff = Buffers::Framebufffer::Create(spotLightSettings);
		m_renderContext.shadowMapSpotTex = m_renderContext.shadowMapSpotBuff->GetDepthTexturePtr();

		//Directional Light Mapping
		Buffers::FramebufferTexture dirDepthTex;
		dirDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings dirLightSettings;
		dirLightSettings.globalWidth = m_settings->dirLightCombineMapSize;
		dirLightSettings.globalHeight = m_settings->dirLightCombineMapSize;
		dirLightSettings.mipMaped = false;
		dirLightSettings.depthStencilAttachment = dirDepthTex;
		m_renderContext.shadowMapDirBuff = Buffers::Framebufffer::Create(dirLightSettings);
		m_renderContext.shadowMapDirTex = m_renderContext.shadowMapDirBuff->GetDepthTexturePtr();


		//Point Lights Mapping
		Buffers::FramebufferTexture pointTex;
		pointTex.format = Resources::TextureFormat::RGBA16F;
		Buffers::FramebufferTexture pointDepthTex;
		pointDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings pointLightSettings;
		pointLightSettings.globalWidth = m_settings->pointLightCombineShadowMapSize;
		pointLightSettings.globalHeight = m_settings->pointLightCombineShadowMapSize;;
		pointLightSettings.mipMaped = false;
		pointLightSettings.colorTextureAttachments = pointTex;
		pointLightSettings.depthStencilAttachment = pointDepthTex;
		m_renderContext.shadowMapPointBuff = Buffers::Framebufffer::Create(pointLightSettings);
		m_renderContext.shadowMapPointTex = m_renderContext.shadowMapPointBuff->GetTexturePtr(0);
		

		//Main Directional Light
		Buffers::FramebufferTexture mainDirDepthTex;
		mainDirDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings mainDirSettings;
		mainDirSettings.globalWidth = m_settings->mainLightShadowCombineMapSize;
		mainDirSettings.globalHeight = m_settings->mainLightShadowCombineMapSize;
		mainDirSettings.mipMaped = false;
		mainDirSettings.depthStencilAttachment = mainDirDepthTex;
		m_renderContext.shadowMapMainDirBuff = Buffers::Framebufffer::Create(mainDirSettings);
		m_renderContext.shadowMapMainDirTex = m_renderContext.shadowMapMainDirBuff->GetDepthTexturePtr();
	}

	void DefRendererBackend::RenderCubeMap(Resources::MaterialPtr p_material, const RenderContext* p_renderContext)
	{
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::LessEqual);

		p_material->SetProperty("view", p_renderContext->camera->GetViewMatrix());
		p_material->SetProperty("proj", p_renderContext->camera->GetProjectionMatrix());

		p_material->Bind();
		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray(), Primitives::TriangleStrip);
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();
		p_material->Unbind();
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);
	}

	void DefRendererBackend::RenderBackBuffer(Resources::ShaderPtr p_postProcessShader, const RenderContext* p_renderContext)
	{
		LowRenderer::EnableDepth(false);
		LowRenderer::EnableCullFace(false);

		p_renderContext->otuputBuff->Bind();

		p_postProcessShader->Bind();
		p_renderContext->postprocessTex->Bind(0);
		p_postProcessShader->SetUniformInt("backTex", 0);

		p_postProcessShader->SetUniformBool("enableBloom", p_renderContext->m_settings->enableBloom);
		p_renderContext->bloomTex->Bind(1);
		p_postProcessShader->SetUniformInt("bloomTex", 1);
	
		p_postProcessShader->SetUniformFloat("exposure", p_renderContext->m_settings->toneMappingExposure);

		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();
		p_postProcessShader->Unbind();
		p_renderContext->postprocessTex->Unbind();

		p_renderContext->otuputBuff->Unbind();
	}

	void DefRendererBackend::RenderTransparent(RenderObjectPtr p_object, const RenderContext* p_renderContext)
	{
		auto material = p_object->material;
		auto mesh = p_object->mesh;

		material->SetProperty("PIPELINE_VP_MAT", p_renderContext->camera->GetCameraMatrix());
		material->SetProperty("PIPELINE_CAMPOS", p_renderContext->camera->GetPosition());

		if (p_renderContext->IRMap && p_renderContext->prefilterMap && p_renderContext->brdfLUT)
		{
			material->SetTexture("PIPELINE_IRRADIANCE_MAP", p_renderContext->IRMap);
			material->SetTexture("PIPELINE_PREFILTER_MAP", p_renderContext->prefilterMap);
			material->SetTexture("PIPELINE_BRDF_LUT", p_renderContext->brdfLUT);
		}

		if (p_object->type == RenderObjectType::Mesh)
		{
			material->SetProperty("PIPELINE_MODEL_MAT", p_object->worldMat);
			material->SetProperty("PIPELINE_INTANCE_COUNT", 0);
			material->Bind();
			mesh->Bind();
			LowRenderer::Draw(mesh->GetVertexArray());
			mesh->Unbind();

			p_renderContext->frameInfo->drawTriangles += mesh->GetIndicesCount() / 3;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}
		else if (p_object->type == RenderObjectType::InstancedMesh)
		{
			material->SetPropertyArray("PIPELINE_MODEL_MAT_ARRAY[0]", p_object->worldMatrices.data(), p_object->worldMatrices.size());
			material->SetProperty("PIPELINE_INTANCE_COUNT", (int)p_object->instanceSize);
			material->Bind();
			mesh->Bind();
			LowRenderer::DrawInstanced(mesh->GetVertexArray(), p_object->instanceSize);
			mesh->Unbind();

			p_renderContext->frameInfo->drawTriangles += mesh->GetIndicesCount() / 3 * p_object->instanceSize;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}

		material->Unbind();
	}

	void DefRendererBackend::RenderSSAO(Resources::ShaderPtr p_SSAOShader, Resources::ShaderPtr p_BlurSSAOShader, const RenderContext* p_renderContext)
	{
		p_renderContext->SSAOBuff->Bind();
		p_SSAOShader->Bind();

		LowRenderer::Clear(ColorBuffer | DepthBuffer);
		LowRenderer::EnableDepth(false);
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);
		LowRenderer::EnableBlending(false);

		p_renderContext->gBuffer.positionTex->Bind(0);
		p_SSAOShader->SetUniformInt("positionMap", 0);

		p_renderContext->gBuffer.normalsTex->Bind(1);
		p_SSAOShader->SetUniformInt("normalMap", 1);

		p_renderContext->SSAONoiseTex->Bind(2);
		p_SSAOShader->SetUniformInt("texNoise", 2);

		p_SSAOShader->SetUniformMat4("projectionMat", p_renderContext->camera->GetProjectionMatrix());
		p_SSAOShader->SetUniformMat4("viewMat", p_renderContext->camera->GetViewMatrix());
		p_SSAOShader->SetUniformVec3Array("samples", p_renderContext->ssaoKernel.data(), p_renderContext->ssaoKernel.size());
		p_SSAOShader->SetUniformVec2("screenSize", PrCore::Math::vec2{ PrCore::Windowing::Window::GetMainWindow().GetWidth(), PrCore::Windowing::Window::GetMainWindow().GetHeight() });

		p_SSAOShader->SetUniformInt("kernelSize", p_renderContext->m_settings->SSAOKenrelSize);
		p_SSAOShader->SetUniformFloat("radius", p_renderContext->m_settings->SSAORadius);
		p_SSAOShader->SetUniformFloat("bias", p_renderContext->m_settings->SSAObias);
		p_SSAOShader->SetUniformFloat("magnitude", p_renderContext->m_settings->SSAOMagnitude);

		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();

		p_SSAOShader->Unbind();
		LowRenderer::EnableDepth(false);
		p_renderContext->SSAOBuff->Unbind();


		//Blur the SSAO
		p_renderContext->gBuffer.buffer->Bind();
		p_BlurSSAOShader->Bind();
		LowRenderer::SetColorMask(false, false, false, true);

		p_renderContext->SSAOTex->Bind(0);
		p_SSAOShader->SetUniformInt("ssaoInput", 0);
		p_SSAOShader->SetUniformInt("SSAOBlureSize", p_renderContext->m_settings->SSAOBlureSize);

		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();

		LowRenderer::SetColorMask(true, true, true, true);
		p_BlurSSAOShader->Unbind();
		p_renderContext->gBuffer.buffer->Unbind();
	}

	void DefRendererBackend::RenderFXAA(Resources::ShaderPtr p_FXAAShader, const RenderContext* p_renderContext)
	{
		p_FXAAShader->Bind();

		LowRenderer::Clear(ColorBuffer | DepthBuffer);
		LowRenderer::EnableDepth(false);
		LowRenderer::EnableBlending(false);

		p_renderContext->postprocessTex->Bind(0);
		p_FXAAShader->SetUniformInt("screenTexture", 0);
		p_FXAAShader->SetUniformBool("enableFXAA", p_renderContext->m_settings->enableFXAAA);

		p_FXAAShader->SetUniformVec2("inverseScreenSize", PrCore::Math::vec2{ 1.0f / PrCore::Windowing::Window::GetMainWindow().GetWidth(), 1.0f / PrCore::Windowing::Window::GetMainWindow().GetHeight() });
		p_FXAAShader->SetUniformFloat("edge_threshold_min", p_renderContext->m_settings->FXAAThreasholdMin);
		p_FXAAShader->SetUniformFloat("edge_threshold_max", p_renderContext->m_settings->FXAAThreasholdMax);
		p_FXAAShader->SetUniformFloat("edge_iterations", p_renderContext->m_settings->FXAAEdgeIterations);
		p_FXAAShader->SetUniformFloat("subpixel_quality", p_renderContext->m_settings->FXAASubpixelQuiality);

		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();

		p_FXAAShader->Unbind();
	}

	void DefRendererBackend::RenderFog(Resources::ShaderPtr p_fogShader, const RenderContext* p_renderContext)
	{
		p_renderContext->otuputBuff->Bind();
		p_fogShader->Bind();
		LowRenderer::Clear(ColorBuffer | DepthBuffer);
						LowRenderer::EnableBlending(true);

		p_renderContext->postprocessTex->Bind(0);
		p_fogShader->SetUniformInt("screenTexture", 0);

		p_renderContext->gBuffer.positionTex->Bind(1);
		p_fogShader->SetUniformInt("positionMap", 1);

		p_fogShader->SetUniformVec3("fogColor", p_renderContext->m_settings->fogColor);
		p_fogShader->SetUniformFloat("maxDistance", p_renderContext->m_settings->fogMaxDistance);
		p_fogShader->SetUniformFloat("densityFactor", p_renderContext->m_settings->fogDencity);

		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();

		p_renderContext->otuputBuff->Unbind();
		p_fogShader->Unbind();
	}

	void DefRendererBackend::RenderBloom(Resources::ShaderPtr p_downsample, Resources::ShaderPtr p_upsample, const RenderContext* p_renderContext)
	{
		// downsample

		float& threshold = p_renderContext->m_settings->bloomThreshold;
		float& knee = p_renderContext->m_settings->bloomKnee;

		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::Y))
			threshold += 0.1f;
		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::H))
			threshold -= 0.1f;

		p_downsample->Bind();
		p_renderContext->m_quadMesh->Bind();
		p_renderContext->postprocessTex->Bind(0);
		p_downsample->SetUniformInt("inputTex", 0);
		p_downsample->SetUniformVec4("threshold", glm::vec4(threshold, threshold - knee, 2.0f * knee, 0.25f * knee));

		for (int i = 0; i < BLOOM_SIZE; i++)
		{
			float width = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> (i + 1);
			float height = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> (i + 1);

			p_renderContext->bloomDownscaleBuff[i]->Bind();
			LowRenderer::Clear(ColorBuffer | DepthBuffer);
			p_downsample->SetUniformVec2("texelSize", PrCore::Math::vec2{ 1.0f / width, 1.0f / height});
			LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
			p_renderContext->frameInfo->drawCalls++;

			p_renderContext->bloomDownscaleTex[i]->Bind(0);
			p_downsample->SetUniformInt("inputTex", 0);
		}

		p_renderContext->m_quadMesh->Unbind();
		p_downsample->Unbind();


		// upsample
		LowRenderer::EnableBlending(true);
		LowRenderer::SetBlendingAlgorythm(BlendingAlgorithm::One, BlendingAlgorithm::One);

		p_upsample->Bind();
		p_renderContext->m_quadMesh->Bind();
		for (int i = BLOOM_SIZE - 1; i > 0; --i)
		{
			float width = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> (i + 1);
			float height = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> (i + 1);

			p_renderContext->bloomDownscaleBuff[PrCore::Math::max(i - 1, 0)]->Bind();

			p_renderContext->bloomDownscaleTex[i]->Bind(0);
			p_downsample->SetUniformInt("inputTex", 0);
			p_downsample->SetUniformVec2("texelSize", PrCore::Math::vec2{ 1.0f / width, 1.0f / height });
			LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
			p_renderContext->frameInfo->drawCalls++;

		}

		//Last draw into final blur
		float width = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> 1;
		float height = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> 1;

		p_renderContext->bloomBuff->Bind();
		LowRenderer::Clear(ColorBuffer | DepthBuffer);

		p_renderContext->bloomDownscaleTex[0]->Bind(0);
		p_downsample->SetUniformInt("inputTex", 0);
		p_downsample->SetUniformVec2("texelSize", PrCore::Math::vec2{ 1.0f / width, 1.0f / height });
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();
		p_renderContext->bloomBuff->Unbind();
		p_upsample->Unbind();

		LowRenderer::EnableBlending(false);
	}

	void DefRendererBackend::RenderLight(Resources::ShaderPtr p_lightShdr, DirLightObjectPtr p_mianDirectLight, std::vector<LightObjectPtr>* p_lights, const RenderContext* p_renderContext)
	{
		p_renderContext->otuputBuff->Bind();

		p_lightShdr->Bind();

		// Set PBR Textures
		p_renderContext->gBuffer.albedoTex->Bind(0);
		p_lightShdr->SetUniformInt("albedoMap", 0);

		p_renderContext->gBuffer.normalsTex->Bind(1);
		p_lightShdr->SetUniformInt("normalMap", 1);

		p_renderContext->gBuffer.positionTex->Bind(2);
		p_lightShdr->SetUniformInt("positionMap", 2);

		p_renderContext->gBuffer.aoTex->Bind(3);
		p_lightShdr->SetUniformInt("aoMap", 3);

		if (p_renderContext->IRMap && p_renderContext->prefilterMap && p_renderContext->brdfLUT)
		{
			p_renderContext->IRMap->Bind(4);
			p_lightShdr->SetUniformInt("PBR_irradianceMap", 4);

			p_renderContext->prefilterMap->Bind(5);
			p_lightShdr->SetUniformInt("PBR_prefilterMap", 5);

			p_renderContext->brdfLUT->Bind(6);
			p_lightShdr->SetUniformInt("PBR_brdfLUT", 6);
		}

		p_lightShdr->SetUniformVec3("camPos", p_renderContext->camera->GetPosition());
		p_lightShdr->SetUniformVec3("PBR_ambientColor", p_renderContext->m_settings->ambientColor);
		p_lightShdr->SetUniformFloat("PBR_cubemapIntensity", p_renderContext->m_settings->ambientIntensity);

		// Set Shadows
		// Main Directional Light
		p_lightShdr->SetUniformFloatArray("SHDW_borders", p_renderContext->m_settings->cascadeShadowBordersCamSpace, 4);
		p_lightShdr->SetUniformFloatArray("SHDW_RadiusRatio", p_renderContext->m_settings->cascadeShadowRadiusRatio, 4);

		if (p_mianDirectLight && p_renderContext->shadowMapMainDirTex)
		{
			p_lightShdr->SetUniformMat4("SHDW_MainDirLightMat", p_mianDirectLight->packedMat);
			p_lightShdr->SetUniformBool("SHDW_HasMainDirLight", true);
			p_lightShdr->SetUniformMat4Array("SHDW_MainDirLightViewMat", p_mianDirectLight->viewMatrices.data(), p_mianDirectLight->viewMatrices.size());
			p_lightShdr->SetUniformInt("SHDW_MainDirLightMapSize", p_renderContext->m_settings->mainLightShadowMapSize);
			p_lightShdr->SetUniformInt("SHDW_MainDirLightCombineMapSize", p_renderContext->m_settings->mainLightShadowCombineMapSize);
			p_lightShdr->SetUniformFloat("SHDW_BorderBlend", p_renderContext->m_settings->mainLightBlendDist);
			p_lightShdr->SetUniformFloat("SHDW_MainDirLightBias", p_renderContext->m_settings->mainLightShadowBias);
			p_lightShdr->SetUniformFloat("SHDW_MainDirLightSize", p_renderContext->m_settings->mainLightSize);

			p_renderContext->shadowMapMainDirTex->Bind(7);
			p_lightShdr->SetUniformInt("SHDW_MainDirLightMap", 7);
		}
		else
			p_lightShdr->SetUniformBool("SHDW_HasMainDirLight", false);

		//Directional Light
		std::vector<PrCore::Math::mat4> dirLightMat;
		std::vector<PrCore::Math::mat4> dirLightViewMat;
		std::vector<int>                dirLightIDs;

		//Point Light
		std::vector<PrCore::Math::mat4> pointlightMat;
		std::vector<int>                pointLighttexPos;

		//Spot Light
		std::vector<PrCore::Math::mat4> spotLightMat;
		std::vector<PrCore::Math::mat4> spotLightViewMat;
		std::vector<int>                spotLightIDs;

		for (auto& lightPtr : *p_lights)
		{
			if (lightPtr->GetType()== Resources::LightType::Directional)
			{
				const auto& lightObject = std::static_pointer_cast<DirLightObject>(lightPtr);
				dirLightMat.push_back(lightObject->packedMat);
				for (auto& viewMat : lightObject->viewMatrices)
					dirLightViewMat.push_back(viewMat);
				dirLightIDs.push_back(lightObject->shadowMapPos);
			}
			else if (lightPtr->GetType() == Resources::LightType::Point)
			{
				pointlightMat.push_back(lightPtr->packedMat);
				pointLighttexPos.push_back(lightPtr->shadowMapPos);
			}
			else if (lightPtr->GetType() == Resources::LightType::Spot)
			{
				const auto& lightObject = std::static_pointer_cast<SpotLightObject>(lightPtr);
				spotLightMat.push_back(lightObject->packedMat);
				spotLightViewMat.push_back(lightObject->viewMatrix);
				spotLightIDs.push_back(lightObject->shadowMapPos);
			}
		}

		// Diretional Light
		p_lightShdr->SetUniformInt("SHDW_DirLightNumber", dirLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_DirLightMat", dirLightMat.data(), dirLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_DirLightViewMat", dirLightViewMat.data(), dirLightViewMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_DirLightID", dirLightIDs.data(), dirLightIDs.size());
		p_lightShdr->SetUniformInt("SHDW_DirLightMapSize", p_renderContext->m_settings->dirLightShadowsMapSize);
		p_lightShdr->SetUniformInt("SHDW_DirLightCombineShadowMapSize", p_renderContext->m_settings->dirLightCombineMapSize);
		p_lightShdr->SetUniformFloat("SHDW_DirLightBias", p_renderContext->m_settings->dirLightShadowBias);
		p_lightShdr->SetUniformFloat("SHDW_DirLightSize", p_renderContext->m_settings->dirLightSize);
		p_renderContext->shadowMapDirTex->Bind(8);
		p_lightShdr->SetUniformInt("SHDW_DirLightMap", 8);

		// Point Light
		p_lightShdr->SetUniformInt("SHDW_PointLightNumber", pointlightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_PointLightMat", pointlightMat.data(), pointlightMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_PointLightID", pointLighttexPos.data(), pointLighttexPos.size());
		p_lightShdr->SetUniformInt("SHDW_PointLightMapSize", p_renderContext->m_settings->pointLightShadowMapSize);
		p_lightShdr->SetUniformInt("SHDW_PointCombineLightMapSize", p_renderContext->m_settings->pointLightCombineShadowMapSize);
		p_lightShdr->SetUniformFloat("SHDW_PointLightBias", p_renderContext->m_settings->pointLightShadowBias);
		p_lightShdr->SetUniformFloat("SHDW_PointLightSize", p_renderContext->m_settings->pointLightSize);
		p_renderContext->shadowMapPointTex->Bind(9);
		p_lightShdr->SetUniformInt("SHDW_PointLightMap", 9);

		// Spot Light
		p_lightShdr->SetUniformInt("SHDW_SpotLightNumber", spotLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_SpotLightMat", spotLightMat.data(), spotLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_SpotLightViewMat", spotLightViewMat.data(), spotLightViewMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_SpotLightID", spotLightIDs.data(), spotLightIDs.size());
		p_lightShdr->SetUniformInt("SHDW_SpotLightMapSize", p_renderContext->m_settings->spotLightShadowMapSize);
		p_lightShdr->SetUniformInt("SHDW_SpotLightCombineMapSize", p_renderContext->m_settings->spotLightCombineShadowMapSize);
		p_lightShdr->SetUniformFloat("SHDW_SpotLightBias", p_renderContext->m_settings->spotLightShadowBias);
		p_lightShdr->SetUniformFloat("SHDW_SpotLightSize", p_renderContext->m_settings->spotLightSize);
		p_renderContext->shadowMapSpotTex->Bind(10);
		p_lightShdr->SetUniformInt("SHDW_SpotLightMap", 10);

		p_renderContext->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->m_quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->m_quadMesh->Unbind();

		p_lightShdr->Unbind();
		p_renderContext->otuputBuff->Unbind();
	}

	void DefRendererBackend::GenerateIRMap()
	{
		m_renderContext.IRMap.reset();

		Buffers::FramebufferTexture texture;
		texture.format = Resources::TextureFormat::RGB16F;
		texture.cubeTexture = true;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = 32;
		settings.globalHeight = 32;
		settings.colorTextureAttachments = texture;

		auto framebuffer = Buffers::Framebufffer::Create(settings);

		auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("IrradianceMap.shader");
		auto cube = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
		auto cubemap = m_frame->cubemapObject->material->GetTexture("skybox");

		PrCore::Math::mat4 captureProjection = PrCore::Math::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		PrCore::Math::mat4 captureViews[] =
		{
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		shader->Bind();
		shader->SetUniformInt("environmentMap", 0);
		shader->SetUniformMat4("projection", captureProjection);
		cubemap->Bind();
		framebuffer->Bind();
		cube->Bind();

		for (int i = 0; i < 6; i++)
		{
			shader->SetUniformMat4("view", captureViews[i]);
			framebuffer->SetAttachmentDetails(0, i);

			LowRenderer::Clear(ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
			LowRenderer::Draw(cube->GetVertexArray());
		}

		m_renderContext.IRMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr(0));

		shader->Unbind();
		cubemap->Unbind();
		shader->Unbind();
		cube->Unbind();
		framebuffer->Unbind();

		PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("IrradianceMap.shader");
	}

	void DefRendererBackend::GeneratePrefilterMap()
	{
		m_renderContext.prefilterMap.reset();

		Buffers::FramebufferTexture texture;
		texture.format = Resources::TextureFormat::RGB16F;
		texture.filteringMin = Resources::TextureFiltering::LinearMipMapLinear;
		texture.filteringMag = Resources::TextureFiltering::Linear;
		texture.cubeTexture = true;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = 128;
		settings.globalHeight = 128;
		settings.mipMaped = true;
		settings.colorTextureAttachments = texture;

		auto framebuffer = Buffers::Framebufffer::Create(settings);

		auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("prefilteredCube.shader");
		auto cube = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
		auto cubemap = m_frame->cubemapObject->material->GetTexture("skybox");

		PrCore::Math::mat4 captureProjection = PrCore::Math::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		PrCore::Math::mat4 captureViews[] =
		{
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		shader->Bind();
		shader->SetUniformInt("environmentMap", 0);
		shader->SetUniformMat4("projection", captureProjection);
		cubemap->Bind();
		framebuffer->Bind();
		cube->Bind();

		unsigned int mipMapNumber = 5;
		for (int i = 0; i < mipMapNumber; i++)
		{
			float roughness = (float)i / (float)(mipMapNumber - 1);
			shader->SetUniformFloat("roughness", roughness);
			for (int j = 0; j < 6; j++)
			{
				shader->SetUniformMat4("view", captureViews[j]);
				framebuffer->SetAttachmentDetails(0, j, i);
				LowRenderer::Clear(ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
				LowRenderer::Draw(cube->GetVertexArray());
			}
		}

		m_renderContext.prefilterMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr());

		shader->Unbind();
		cubemap->Unbind();
		shader->Unbind();
		cube->Unbind();
		framebuffer->Unbind();

		PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>(shader->GetName());
	}

	void DefRendererBackend::GenerateLUTMap()
	{
		m_renderContext.brdfLUT.reset();

		Buffers::FramebufferTexture texture;
		texture.format = Resources::TextureFormat::RG16;

		Buffers::FramebufferSettings settings;
		settings.globalHeight = 512;
		settings.globalWidth = 512;
		settings.colorTextureAttachments = texture;

		auto framebuffer = Buffers::Framebufffer::Create(settings);

		auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("LUTMap.shader");
		auto quad = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Quad);

		shader->Bind();
		quad->Bind();
		framebuffer->Bind();

		LowRenderer::Clear(ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
		LowRenderer::Draw(quad->GetVertexArray());

		m_renderContext.brdfLUT = framebuffer->GetTexturePtr();

		shader->Unbind();
		quad->Unbind();
		framebuffer->Unbind();

		PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("LUTMap.shader");
	}

	PrCore::Math::vec4 DefRendererBackend::CalculateLightTexture(size_t p_lightID, size_t p_lightMapSize, size_t p_comboMapSize)
	{
		size_t comboMapSize = p_comboMapSize;

		size_t rowCount = comboMapSize / p_lightMapSize;
		size_t row = p_lightID % rowCount;
		size_t column = p_lightID / rowCount;

		return { p_lightMapSize, p_lightMapSize, row * p_lightMapSize, column * p_lightMapSize };
	}
}
