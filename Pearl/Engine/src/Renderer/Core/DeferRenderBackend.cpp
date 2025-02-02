#include "Core/Common/pearl_pch.h"
#include "Renderer/Core/DeferRenderBackend.h"

#include "Core/Events/WindowEvents.h"
#include "Renderer/Core/LowRenderer.h"
#include "Core/Resources/ResourceSystem.h"
#include "Core/Windowing/Window.h"
#include <Core/Events/EventManager.h>
#include <random>

#include "Core/Input/InputManager.h"
#include "Core/Utils/Clock.h"

namespace PrRenderer::Core
{
	DeferRenderBackend::DeferRenderBackend(RendererSettingsPtr& p_settings) :
		IRenderBackend(p_settings)
	{
		//Prepare shaders
		m_shadowMappingShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/shadows/generic_shadow_mapping.shader");
		m_ToneMappingShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/front_pass.shader");
		m_pbrLightShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/light_pass.shader");
		m_pointshadowMappingShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/shadows/point_shadow_mapping.shader");
		m_SSAOShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/ssao.shader");
		m_SSAOBlurShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/ssao_blur.shader");
		m_FXAAShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/fxaa.shader");
		m_fogShdr = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/logarithmic _fog.shader");
		m_downsample = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/downsample.shader");
		m_upsample = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/deffered/upsample.shader");

		m_renderContext.quadMesh = Resources::Mesh::CreatePrimitive(Resources::Quad);
		m_renderContext.settings = m_settings;

		m_screenWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		m_screenHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();

		// Prepare bffers
		GenerategGBuffersContext();
		GenerateShadowMaps();
		GenerateSSAOContext();
		GeneratePostprocessContext();

		// Set events
		PrCore::Events::EventListener windowResizedListener;
		windowResizedListener.connect<&DeferRenderBackend::OnWindowResize>(this);
		PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	}

	void DeferRenderBackend::PreparePipeline()
	{
		SCOPE_HIGH_TIMER_CALLBACK(
			{ m_frame->frameInfo.timeEvents.push_back({ "PreparePipeline", time }); });

		// Temp variables
		const auto camera = m_frame->camera;
		auto clock = PrCore::Utils::Clock::GetInstancePtr();
		m_renderContext.frameInfo = &m_frame->frameInfo;
		m_renderContext.camera = camera;
		m_renderContext.camera->RecalculateMatrices();


		// Begin pipeline preparations
		// Clear Back Buffer
		PushCommand(CreateRC<LowRenderer::EnableDepthRC>(true));
		PushCommand(CreateRC<LowRenderer::EnableBlendingRC>(false));
		PushCommand(CreateRC<LowRenderer::EnableCullFaceRC>(true));
		PushCommand(CreateRC<LowRenderer::ClearRC>(ColorBuffer | DepthBuffer));

		// Calculate PBR Reflection if cubemap changed
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
			m_renderContext.IRMap.reset();
			m_renderContext.prefilterMap.reset();
			m_renderContext.brdfLUT.reset();
		}

		// Shadow Mapping
		//---------------------------------
		// Camera settings has changed, recalculate CSM matrices
		if((m_frame->renderFlag & RendererFlag::CameraPerspectiveRecalculate) == RendererFlag::CameraPerspectiveRecalculate)
		{
			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
				m_settings->cascadeShadowBordersCamSpace[i] = m_settings->cascadeShadowBorders[i] * camera->GetFar();

			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), camera->GetFar() * m_settings->cascadeShadowBorders[0]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings->cascadeShadowBorders[0], camera->GetFar() * m_settings->cascadeShadowBorders[1]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings->cascadeShadowBorders[1], camera->GetFar() * m_settings->cascadeShadowBorders[2]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings->cascadeShadowBorders[2], camera->GetFar() * m_settings->cascadeShadowBorders[3]));
		}

		// Main Directional Light
		if(m_frame->mainDirectLight && m_frame->mainDirectLight->castShadow)
		{
			const auto mainLight = m_frame->mainDirectLight;
			const auto lightDir = mainLight->GetDirection();

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
				PushCommand(CreateRC<RenderToShadowMapRC>(m_shadowMappingShdr.GetData(), lightMat, m_frame->mainDirectLight, &m_frame->shadowCasters, &m_renderContext));
			}

			m_settings->cascadeShadowRadiusRatio[1] = m_settings->cascadeShadowRadiusRatio[0] / m_settings->cascadeShadowRadiusRatio[1];
			m_settings->cascadeShadowRadiusRatio[2] = m_settings->cascadeShadowRadiusRatio[0] / m_settings->cascadeShadowRadiusRatio[2];
			m_settings->cascadeShadowRadiusRatio[3] = m_settings->cascadeShadowRadiusRatio[0] / m_settings->cascadeShadowRadiusRatio[3];
			m_settings->cascadeShadowRadiusRatio[0] = 1.0f;
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
				auto lightMat = m_CSMUtility.ClaculateFrustrums(m_settings->cascadeShadowRadiusRatio[i], i, lightDir, camera->GetViewMatrix(), m_settings->dirLightShadowsMapSize, m_settings->dirLightCascadeExtend);
				auto viewport = CalculateLightTexture(light->shadowMapPos * SHADOW_CASCADES_COUNT + i, m_settings->dirLightShadowsMapSize, m_settings->dirLightCombineMapSize);
				light->viewMatrices.push_back(lightMat);

				PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
				PushCommand(CreateRC<RenderToShadowMapRC>(m_shadowMappingShdr.GetData(), lightMat, lightPtr, &m_frame->shadowCasters, &m_renderContext));
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

		for (auto lightPtr : m_frame->lights)
		{
			if (lightPtr->GetType() != Resources::LightType::Point)
				continue;

			if (!lightPtr->castShadow)
				continue;

			auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, 0.001f, lightPtr->GetRange() * 1.2f);
			auto lightPos = lightPtr->GetPosition();
			auto shadowMapPos = lightPtr->shadowMapPos;

			auto lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			auto viewport = CalculateLightTexture(shadowMapPos + 0, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(-1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(shadowMapPos + 1, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 1.0f, 0.0), PrCore::Math::vec3(0.0f, 0.0f, 1.0f));
			viewport = CalculateLightTexture(shadowMapPos + 2, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, -1.0f, 0.0f), PrCore::Math::vec3(0.0f, 0.0f, -1.0f));
			viewport = CalculateLightTexture(shadowMapPos + 3, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, 1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(shadowMapPos + 4, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, -1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(shadowMapPos + 5, m_settings->pointLightShadowMapSize, m_settings->pointLightCombineShadowMapSize);
			PushCommand(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			PushCommand(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));
		}
		

		// Spotlights
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
			PushCommand(CreateRC<RenderToShadowMapRC>(m_shadowMappingShdr.GetData(), lightProjMatrix * lightViewMatrix, lightPtr, &m_frame->shadowCasters, &m_renderContext));
		}

		PushCommand(CreateRC<LowRenderer::SetViewportRC>(PrCore::Windowing::Window::GetMainWindow().GetWidth(),
			PrCore::Windowing::Window::GetMainWindow().GetHeight(), 0, 0));

		//---------------------------------

		// Opaque Objects
		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.gBuffer.buffer->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
				LowRenderer::EnableCullFace(true);
				LowRenderer::EnableDepth(true);
			}));

		for (auto object : m_frame->opaqueObjects)
			PushCommand(CreateRC<RenderOpaqueRC>(object, &m_renderContext));

		PushCommand(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderContext.gBuffer.buffer->Unbind();
			}));

		// Clear output buffer
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
			PushCommand(CreateRC<RenderSSAORC>(m_SSAOShdr.GetData(), m_SSAOBlurShdr.GetData(), &m_renderContext));

		// PBR Light Pass
		PushCommand(CreateRC<RenderLightRC>(m_pbrLightShdr.GetData(), m_frame->mainDirectLight, &m_frame->lights, &m_renderContext));

		// Transparent Forward Pass
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
		//---------------------------------

		// Post Processing
		// Fog
		if (m_settings->enableFog)
		{
			PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
			PushCommand(CreateRC<RenderFogRC>(m_fogShdr.GetData(), &m_renderContext));
		}

		// Bloom
		if(m_settings->enableBloom)
		{
			PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
			PushCommand(CreateRC<RenderBloomRC>(m_downsample.GetData(), m_upsample.GetData(), &m_renderContext));
		}

		// Tone Mapping
		PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
		PushCommand(CreateRC<RenderToneMappingRC>(m_ToneMappingShdr.GetData(), &m_renderContext));

		//Render Debug
		PushCommand(CreateRC<RenderDebugRC>(&m_frame->debugObjects, &m_renderContext));

		// FXAA Anti-Aliasing and Render front
		PushCommand(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderContext.otuputBuff, m_renderContext.postprocessBuff, Buffers::FramebufferMask::ColorBufferBit));
		PushCommand(CreateRC<RenderFXAARC>(m_FXAAShdr.GetData(), &m_renderContext));
	}

	void DeferRenderBackend::Render()
	{
		SCOPE_HIGH_TIMER_CALLBACK(
			{ m_frame->frameInfo.timeEvents.push_back({ "RenderOnCPU", time }); });

		while (!m_commandQueue.empty())
		{
			m_commandQueue.front()->Invoke();
			m_commandQueue.pop_front();
		}
	}

	void DeferRenderBackend::PostRender()
	{
	}

	void DeferRenderBackend::OnWindowResize(PrCore::Events::EventPtr p_event)
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

	void DeferRenderBackend::RenderOpaque(RenderObjectPtr p_object, const RenderContext* p_renderContext)
	{
		const auto& renderVA = p_object->vertexArrayPtr;
		const auto& material = p_object->material;
		const auto& submesh = p_object->subMesh;
		const auto& shader = p_object->material->GetShader();

		p_renderContext->gBuffer.buffer->Bind();

		material->Bind();
		renderVA->Bind();

		shader->SetUniformMat4("PIPELINE_VP_MAT", p_renderContext->camera->GetCameraMatrix());
		shader->SetUniformFloat("PIPELINE_NEAR", p_renderContext->camera->GetNear());
		shader->SetUniformFloat("PIPELINE_FAR", p_renderContext->camera->GetFar());

		if (p_object->type == RenderObjectType::Mesh)
		{
			shader->SetUniformMat4("PIPELINE_MODEL_MAT", p_object->worldMat);
			shader->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);
	
			LowRenderer::Draw(renderVA, submesh.indicesCount, submesh.firstIndex);

			p_renderContext->frameInfo->drawTriangles += submesh.indicesCount / 3;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}
		else if(p_object->type == RenderObjectType::InstancedMesh)
		{
			shader->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", p_object->worldMatrices.data(), p_object->worldMatrices.size());
			shader->SetUniformInt("PIPELINE_INTANCE_COUNT", p_object->instanceSize);

			LowRenderer::DrawInstanced(renderVA, p_object->instanceSize);

			p_renderContext->frameInfo->drawTriangles += renderVA->GetIndexBuffer()->GetSize() / 3 * p_object->instanceSize;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}

		renderVA->Unbind();
		material->Unbind();
	}

	void DeferRenderBackend::RenderToShadowMap(Resources::ShaderPtr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData)
	{
		const auto frustrum = Frustrum(p_lightMatrix);
		bool skipCulling = p_light->GetType() == Resources::LightType::Directional;

		p_shaderPtr->Bind();
		p_shaderPtr->SetUniformMat4("PIPELINE_LIGHT_MAT", p_lightMatrix);

		for (auto& object : *p_objects)
		{
			const auto& renderVA = object->vertexArrayShadowPtr ? object->vertexArrayShadowPtr : object->vertexArrayPtr;
			auto submesh = object->vertexArrayShadowPtr ? Resources::SubMesh() : object->subMesh;

			if (object->type == RenderObjectType::Mesh)
			{
				// Cull if out of camera
				if (!skipCulling && !object->boxVolume.IsOnFrustrum(frustrum, object->worldMat))
					continue;

				p_shaderPtr->SetUniformMat4("PIPELINE_MODEL_MAT", object->worldMat);
				p_shaderPtr->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);
				renderVA->Bind();
				LowRenderer::Draw(renderVA, submesh.indicesCount, submesh.firstIndex);
				p_renderData->frameInfo->drawCalls++;
				renderVA->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_shaderPtr->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_shaderPtr->SetUniformInt("PIPELINE_INTANCE_COUNT", object->instanceSize);
				renderVA->Bind();
				LowRenderer::DrawInstanced(renderVA, object->instanceSize);
				p_renderData->frameInfo->drawCalls++;
				renderVA->Unbind();
			}
		}

		p_shaderPtr->Unbind();
	}

	void DeferRenderBackend::RenderToPointShadowMap(Resources::ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightView, LightObjectPtr p_light, std::list<RenderObjectPtr>* p_objects, const RenderContext* p_renderData)
	{
		const auto lightPos = p_light->GetPosition();
		const auto frustrum = Frustrum(p_lightView);

		p_pointShadowMapShader->Bind();
		p_pointShadowMapShader->SetUniformMat4("PIPELINE_LIGHT_MAT", p_lightView);
		p_pointShadowMapShader->SetUniformVec3("PIPELINE_LIGHT_POS", lightPos);

		for (auto& object : *p_objects)
		{
			const auto& renderVA = object->vertexArrayShadowPtr ? object->vertexArrayShadowPtr : object->vertexArrayPtr;
			auto submesh = object->vertexArrayShadowPtr ? Resources::SubMesh() : object->subMesh;

			if (object->type == RenderObjectType::Mesh)
			{
				// Cull if out of camera
				if (!object->boxVolume.IsOnFrustrum(frustrum, object->worldMat))
					continue;

				// Naive way to cull object, this should be a AABB-SPHERE Intersection in the future
				//PrCore::Math::vec3 objectPos{ object->worldMat[3].x, object->worldMat[3].y, object->worldMat[3].z };
				//auto dist = PrCore::Math::length(objectPos - lightPos);
				//if (dist > p_light->GetRange())
					//continue;

				p_pointShadowMapShader->SetUniformMat4("PIPELINE_MODEL_MAT", object->worldMat);
				p_pointShadowMapShader->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);
				renderVA->Bind();
				LowRenderer::Draw(renderVA, submesh.indicesCount, submesh.firstIndex);
				renderVA->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_pointShadowMapShader->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_pointShadowMapShader->SetUniformInt("PIPELINE_INTANCE_COUNT", object->instanceSize);
				renderVA->Bind();
				LowRenderer::DrawInstanced(renderVA, object->instanceSize);
				renderVA->Unbind();
			}
		}

		p_pointShadowMapShader->Unbind();
	}

	void DeferRenderBackend::GenerateSSAOContext()
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
		for (unsigned int i = 0; i < 16; ++i)
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
		noiseTex->SetMipMap(false);
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
		settings.globalWidth = m_screenWidth;
		settings.globalHeight = m_screenHeight;
		settings.mipMaped = false;
		settings.colorTextureAttachments = { ssaoTex };
		settings.depthStencilAttachment = depthTex;

		m_renderContext.SSAOBuff = Buffers::Framebufffer::Create(settings);
		m_renderContext.SSAOTex = m_renderContext.SSAOBuff->GetTexturePtr();
	}

	void DeferRenderBackend::GeneratePostprocessContext()
	{
		m_renderContext.postprocessBuff.reset();
		m_renderContext.postprocessTex.reset();

		Buffers::FramebufferTexture texture;
		texture.format = Resources::TextureFormat::RGB16F;

		Buffers::FramebufferTexture depthTex;
		depthTex.format = Resources::TextureFormat::Depth16;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = m_screenWidth;
		settings.globalHeight = m_screenHeight;
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
			bloomSettings.globalWidth = m_screenWidth >> (i + 1);
			bloomSettings.globalHeight = m_screenHeight >> (i + 1);
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
		bloomSettings.globalWidth = m_screenWidth;
		bloomSettings.globalHeight = m_screenHeight;
		bloomSettings.colorTextureAttachments = { bloomTexture };
		bloomSettings.depthStencilAttachment = bloomDepthTex;

		m_renderContext.bloomBuff = Buffers::Framebufffer::Create(bloomSettings);
		m_renderContext.bloomTex = m_renderContext.bloomBuff->GetTexturePtr();
	}

	void DeferRenderBackend::GenerategGBuffersContext()
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
		settings.globalWidth = m_screenWidth * 2; // Doubled size to imitate MSAA
		settings.globalHeight = m_screenHeight * 2; // Doubled size to imitate MSAA
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
		outputSettings.globalWidth = m_screenWidth;
		outputSettings.globalHeight = m_screenHeight;
		outputSettings.mipMaped = false;
		outputSettings.colorTextureAttachments = outputTex;
		outputSettings.depthStencilAttachment = outputDepth;

		m_renderContext.otuputBuff = Buffers::Framebufffer::Create(outputSettings);
		m_renderContext.outputTex = m_renderContext.otuputBuff->GetTexturePtr(0);
	}

	void DeferRenderBackend::GenerateShadowMaps()
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

	void DeferRenderBackend::RenderCubeMap(Resources::MaterialPtr p_material, const RenderContext* p_renderContext)
	{
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::LessEqual);

		p_material->SetProperty("view", p_renderContext->camera->GetViewMatrix());
		p_material->SetProperty("proj", p_renderContext->camera->GetProjectionMatrix());

		p_material->Bind();
		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();
		p_material->Unbind();
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);
	}

	void DeferRenderBackend::RenderToneMapping(Resources::ShaderPtr p_toneMapShader, const RenderContext* p_renderContext)
	{
		LowRenderer::EnableDepth(false);
		LowRenderer::EnableCullFace(false);

		p_renderContext->otuputBuff->Bind();

		p_toneMapShader->Bind();
		p_renderContext->postprocessTex->Bind(0);
		p_toneMapShader->SetUniformInt("backTex", 0);

		p_toneMapShader->SetUniformBool("enableBloom", p_renderContext->settings->enableBloom);
		p_renderContext->bloomTex->Bind(1);
		p_toneMapShader->SetUniformInt("bloomTex", 1);
	
		p_toneMapShader->SetUniformFloat("exposure", p_renderContext->settings->toneMappingExposure);

		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();
		p_toneMapShader->Unbind();
		p_renderContext->postprocessTex->Unbind();

		p_renderContext->otuputBuff->Unbind();
	}

	void DeferRenderBackend::RenderTransparent(RenderObjectPtr p_object, const RenderContext* p_renderContext)
	{
		const auto& renderVA = p_object->vertexArrayPtr;
		const auto& material = p_object->material;
		const auto& submesh = p_object->subMesh;
		const auto& shader = p_object->material->GetShader();

		if (p_renderContext->IRMap && p_renderContext->prefilterMap && p_renderContext->brdfLUT)
		{
			material->SetTexture("PBR_irradianceMap", p_renderContext->IRMap);
			material->SetTexture("PBR_prefilterMap", p_renderContext->prefilterMap);
			material->SetTexture("PBR_brdfLUT", p_renderContext->brdfLUT);
		}
		else
		{
			material->SetTexture("PBR_irradianceMap", nullptr);
			material->SetTexture("PBR_prefilterMap", nullptr);
			material->SetTexture("PBR_brdfLUT", nullptr);
		}

		material->Bind();
		renderVA->Bind();

		shader->SetUniformMat4("PIPELINE_VP_MAT", p_renderContext->camera->GetCameraMatrix());
		shader->SetUniformVec3("PIPELINE_CAMPOS", p_renderContext->camera->GetPosition());

		shader->SetUniformVec3("PBR_ambientColor", p_renderContext->settings->ambientColor);
		shader->SetUniformFloat("PBR_cubemapIntensity", p_renderContext->settings->ambientIntensity);

		if (p_object->type == RenderObjectType::Mesh)
		{
			shader->SetUniformMat4("PIPELINE_MODEL_MAT", p_object->worldMat);
			shader->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);

			LowRenderer::Draw(renderVA, submesh.indicesCount, submesh.firstIndex);
			
			p_renderContext->frameInfo->drawTriangles += submesh.indicesCount / 3;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}
		else if (p_object->type == RenderObjectType::InstancedMesh)
		{
			shader->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", p_object->worldMatrices.data(), p_object->worldMatrices.size());
			shader->SetUniformInt("PIPELINE_INTANCE_COUNT", (int)p_object->instanceSize);

			LowRenderer::DrawInstanced(renderVA, p_object->instanceSize);

			p_renderContext->frameInfo->drawTriangles += renderVA->GetIndexBuffer()->GetSize() / 3 * p_object->instanceSize;
			p_renderContext->frameInfo->objectDrawCalls++;
			p_renderContext->frameInfo->drawCalls++;
		}

		renderVA->Unbind();
		material->Unbind();
	}

	void DeferRenderBackend::RenderSSAO(Resources::ShaderPtr p_SSAOShader, Resources::ShaderPtr p_BlurSSAOShader, const RenderContext* p_renderContext)
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

		p_SSAOShader->SetUniformInt("kernelSize", p_renderContext->settings->SSAOKenrelSize);
		p_SSAOShader->SetUniformFloat("radius", p_renderContext->settings->SSAORadius);
		p_SSAOShader->SetUniformFloat("bias", p_renderContext->settings->SSAObias);
		p_SSAOShader->SetUniformFloat("magnitude", p_renderContext->settings->SSAOMagnitude);

		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();

		p_SSAOShader->Unbind();
		LowRenderer::EnableDepth(false);
		p_renderContext->SSAOBuff->Unbind();


		//Blur the SSAO
		p_renderContext->gBuffer.buffer->Bind();
		p_BlurSSAOShader->Bind();
		LowRenderer::SetColorMask(false, false, false, true);

		p_renderContext->SSAOTex->Bind(0);
		p_SSAOShader->SetUniformInt("ssaoInput", 0);
		p_SSAOShader->SetUniformInt("SSAOBlureSize", p_renderContext->settings->SSAOBlureSize);

		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();

		LowRenderer::SetColorMask(true, true, true, true);
		p_BlurSSAOShader->Unbind();
		p_renderContext->gBuffer.buffer->Unbind();
	}

	void DeferRenderBackend::RenderFXAA(Resources::ShaderPtr p_FXAAShader, const RenderContext* p_renderContext)
	{
		p_FXAAShader->Bind();

		LowRenderer::Clear(ColorBuffer | DepthBuffer);
		LowRenderer::EnableDepth(false);
		LowRenderer::EnableBlending(false);

		p_renderContext->postprocessTex->Bind(0);
		p_FXAAShader->SetUniformInt("screenTexture", 0);
		p_FXAAShader->SetUniformBool("enableFXAA", p_renderContext->settings->enableFXAAA);

		p_FXAAShader->SetUniformVec2("inverseScreenSize", PrCore::Math::vec2{ 1.0f / PrCore::Windowing::Window::GetMainWindow().GetWidth(), 1.0f / PrCore::Windowing::Window::GetMainWindow().GetHeight() });
		p_FXAAShader->SetUniformFloat("edge_threshold_min", p_renderContext->settings->FXAAThreasholdMin);
		p_FXAAShader->SetUniformFloat("edge_threshold_max", p_renderContext->settings->FXAAThreasholdMax);
		p_FXAAShader->SetUniformFloat("edge_iterations", p_renderContext->settings->FXAAEdgeIterations);
		p_FXAAShader->SetUniformFloat("subpixel_quality", p_renderContext->settings->FXAASubpixelQuiality);

		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();

		p_FXAAShader->Unbind();
	}

	void DeferRenderBackend::RenderFog(Resources::ShaderPtr p_fogShader, const RenderContext* p_renderContext)
	{
		p_renderContext->otuputBuff->Bind();
		p_fogShader->Bind();
		LowRenderer::Clear(ColorBuffer | DepthBuffer);
						LowRenderer::EnableBlending(true);

		p_renderContext->postprocessTex->Bind(0);
		p_fogShader->SetUniformInt("screenTexture", 0);

		p_renderContext->gBuffer.positionTex->Bind(1);
		p_fogShader->SetUniformInt("positionMap", 1);

		p_fogShader->SetUniformVec3("fogColor", p_renderContext->settings->fogColor);
		p_fogShader->SetUniformFloat("maxDistance", p_renderContext->settings->fogMaxDistance);
		p_fogShader->SetUniformFloat("densityFactor", p_renderContext->settings->fogDencity);

		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();

		p_renderContext->otuputBuff->Unbind();
		p_fogShader->Unbind();
	}

	void DeferRenderBackend::RenderBloom(Resources::ShaderPtr p_downsampleShader, Resources::ShaderPtr p_upsampleShader, const RenderContext* p_renderContext)
	{
		// downsample
		float& threshold = p_renderContext->settings->bloomThreshold;
		float& knee = p_renderContext->settings->bloomKnee;

		p_downsampleShader->Bind();
		p_renderContext->quadMesh->Bind();
		p_renderContext->postprocessTex->Bind(0);
		p_downsampleShader->SetUniformInt("inputTex", 0);
		p_downsampleShader->SetUniformVec4("threshold", glm::vec4(threshold, threshold - knee, 2.0f * knee, 0.25f * knee));

		for (int i = 0; i < BLOOM_SIZE; i++)
		{
			float width = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> (i + 1);
			float height = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> (i + 1);

			p_renderContext->bloomDownscaleBuff[i]->Bind();
			LowRenderer::Clear(ColorBuffer | DepthBuffer);
			p_downsampleShader->SetUniformVec2("texelSize", PrCore::Math::vec2{ 1.0f / width, 1.0f / height});
			LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
			p_renderContext->frameInfo->drawCalls++;

			p_renderContext->bloomDownscaleTex[i]->Bind(0);
			p_downsampleShader->SetUniformInt("inputTex", 0);
		}

		p_renderContext->quadMesh->Unbind();
		p_downsampleShader->Unbind();


		// upsample
		LowRenderer::EnableBlending(true);
		LowRenderer::SetBlendingAlgorythm(BlendingAlgorithm::One, BlendingAlgorithm::One);

		p_upsampleShader->Bind();
		p_renderContext->quadMesh->Bind();
		for (int i = BLOOM_SIZE - 1; i > 0; --i)
		{
			float width = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> (i + 1);
			float height = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> (i + 1);

			p_renderContext->bloomDownscaleBuff[PrCore::Math::max(i - 1, 0)]->Bind();

			p_renderContext->bloomDownscaleTex[i]->Bind(0);
			p_downsampleShader->SetUniformInt("inputTex", 0);
			p_downsampleShader->SetUniformVec2("texelSize", PrCore::Math::vec2{ 1.0f / width, 1.0f / height });
			LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
			p_renderContext->frameInfo->drawCalls++;

		}

		//Last draw into final blur
		float width = PrCore::Windowing::Window::GetMainWindow().GetWidth() >> 1;
		float height = PrCore::Windowing::Window::GetMainWindow().GetHeight() >> 1;

		p_renderContext->bloomBuff->Bind();
		LowRenderer::Clear(ColorBuffer | DepthBuffer);

		p_renderContext->bloomDownscaleTex[0]->Bind(0);
		p_downsampleShader->SetUniformInt("inputTex", 0);
		p_downsampleShader->SetUniformVec2("texelSize", PrCore::Math::vec2{ 1.0f / width, 1.0f / height });
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		p_renderContext->quadMesh->Unbind();
		p_renderContext->bloomBuff->Unbind();
		p_upsampleShader->Unbind();

		LowRenderer::EnableBlending(false);
	}

	void DeferRenderBackend::RenderDebug(RenderObjectVector* p_debugObjects, const RenderContext* p_renderContext)
	{
		p_renderContext->otuputBuff->Bind();

		for (auto& object : *p_debugObjects)
		{
			auto material = object->material;
			auto shader = material->GetShader();
			auto renderVA = object->vertexArrayPtr;
			auto primitives = object->wiredframe ? Primitives::LineStrip : Primitives::Triangles;

			material->Bind();
			shader->SetUniformMat4("PIPELINE_VP_MAT", p_renderContext->camera->GetCameraMatrix());
			shader->SetUniformFloat("PIPELINE_NEAR", p_renderContext->camera->GetNear());
			shader->SetUniformFloat("PIPELINE_FAR", p_renderContext->camera->GetFar());

			if (object->type == RenderObjectType::Mesh)
			{
				shader->SetUniformMat4("PIPELINE_MODEL_MAT", object->worldMat);
				shader->SetUniformInt("PIPELINE_INTANCE_COUNT", 0);

				renderVA->Bind();
				
				LowRenderer::Draw(renderVA, 0, 0, primitives);
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				shader->SetUniformMat4Array("PIPELINE_MODEL_MAT_ARRAY[0]", object->worldMatrices.data(), object->worldMatrices.size());
				shader->SetUniformInt("PIPELINE_INTANCE_COUNT", object->instanceSize);

				renderVA->Bind();
				LowRenderer::DrawInstanced(renderVA, object->instanceSize, primitives);
			}

			material->Unbind();
			renderVA->Unbind();
		}

		p_renderContext->otuputBuff->Unbind();
	}

	void DeferRenderBackend::RenderLight(Resources::ShaderPtr p_lightShdr, DirLightObjectPtr p_mianDirectLight, std::vector<LightObjectPtr>* p_lights, const RenderContext* p_renderContext)
	{
		p_renderContext->otuputBuff->Bind();

		p_lightShdr->Bind();

		// Set PBR Textures
		// Binding textures starting from 4 to avoid uneccessery slot switching
		// First 4 slots can be used for object rendering
		p_renderContext->gBuffer.albedoTex->Bind(4);
		p_lightShdr->SetUniformInt("albedoMap", 4);

		p_renderContext->gBuffer.normalsTex->Bind(5);
		p_lightShdr->SetUniformInt("normalMap", 5);

		p_renderContext->gBuffer.positionTex->Bind(6);
		p_lightShdr->SetUniformInt("positionMap", 6);

		p_renderContext->gBuffer.aoTex->Bind(7);
		p_lightShdr->SetUniformInt("aoMap", 7);

		if (p_renderContext->IRMap && p_renderContext->prefilterMap && p_renderContext->brdfLUT)
		{
			p_renderContext->IRMap->Bind(8);
			p_lightShdr->SetUniformInt("PBR_irradianceMap", 8);

			p_renderContext->prefilterMap->Bind(9);
			p_lightShdr->SetUniformInt("PBR_prefilterMap", 9);

			p_renderContext->brdfLUT->Bind(10);
			p_lightShdr->SetUniformInt("PBR_brdfLUT", 10);
		}

		p_lightShdr->SetUniformVec3("PIPELINE_CAMPOS", p_renderContext->camera->GetPosition());
		p_lightShdr->SetUniformVec3("PBR_ambientColor", p_renderContext->settings->ambientColor);
		p_lightShdr->SetUniformFloat("PBR_cubemapIntensity", p_renderContext->settings->ambientIntensity);

		// Set Shadows
		// Main Directional Light
		p_lightShdr->SetUniformFloatArray("SHDW_borders", p_renderContext->settings->cascadeShadowBordersCamSpace, 4);
		p_lightShdr->SetUniformFloatArray("SHDW_RadiusRatio", p_renderContext->settings->cascadeShadowRadiusRatio, 4);

		if (p_mianDirectLight && p_renderContext->shadowMapMainDirTex)
		{
			p_lightShdr->SetUniformMat4("SHDW_MainDirLightMat", p_mianDirectLight->packedMat);
			p_lightShdr->SetUniformBool("SHDW_HasMainDirLight", true);
			p_lightShdr->SetUniformBool("SHDW_MainDirLightShadow", p_mianDirectLight->castShadow);
			p_lightShdr->SetUniformMat4Array("SHDW_MainDirLightViewMat", p_mianDirectLight->viewMatrices.data(), p_mianDirectLight->viewMatrices.size());
			p_lightShdr->SetUniformInt("SHDW_MainDirLightMapSize", p_renderContext->settings->mainLightShadowMapSize);
			p_lightShdr->SetUniformInt("SHDW_MainDirLightCombineMapSize", p_renderContext->settings->mainLightShadowCombineMapSize);
			p_lightShdr->SetUniformFloat("SHDW_BorderBlend", p_renderContext->settings->mainLightBlendDist);
			p_lightShdr->SetUniformFloat("SHDW_MainDirLightBias", p_renderContext->settings->mainLightShadowBias);
			p_lightShdr->SetUniformFloat("SHDW_MainDirLightSize", p_renderContext->settings->mainLightSize);

			p_renderContext->shadowMapMainDirTex->Bind(11);
			p_lightShdr->SetUniformInt("SHDW_MainDirLightMap", 11);
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
		p_lightShdr->SetUniformInt("SHDW_DirLightMapSize", p_renderContext->settings->dirLightShadowsMapSize);
		p_lightShdr->SetUniformInt("SHDW_DirLightCombineShadowMapSize", p_renderContext->settings->dirLightCombineMapSize);
		p_lightShdr->SetUniformFloat("SHDW_DirLightBias", p_renderContext->settings->dirLightShadowBias);
		p_lightShdr->SetUniformFloat("SHDW_DirLightSize", p_renderContext->settings->dirLightSize);
		p_renderContext->shadowMapDirTex->Bind(12);
		p_lightShdr->SetUniformInt("SHDW_DirLightMap", 12);

		// Point Light
		p_lightShdr->SetUniformInt("SHDW_PointLightNumber", pointlightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_PointLightMat", pointlightMat.data(), pointlightMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_PointLightID", pointLighttexPos.data(), pointLighttexPos.size());
		p_lightShdr->SetUniformInt("SHDW_PointLightMapSize", p_renderContext->settings->pointLightShadowMapSize);
		p_lightShdr->SetUniformInt("SHDW_PointCombineLightMapSize", p_renderContext->settings->pointLightCombineShadowMapSize);
		p_lightShdr->SetUniformFloat("SHDW_PointLightBias", p_renderContext->settings->pointLightShadowBias);
		p_lightShdr->SetUniformFloat("SHDW_PointLightSize", p_renderContext->settings->pointLightSize);
		p_renderContext->shadowMapPointTex->Bind(13);
		p_lightShdr->SetUniformInt("SHDW_PointLightMap", 13);

		// Spot Light
		p_lightShdr->SetUniformInt("SHDW_SpotLightNumber", spotLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_SpotLightMat", spotLightMat.data(), spotLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_SpotLightViewMat", spotLightViewMat.data(), spotLightViewMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_SpotLightID", spotLightIDs.data(), spotLightIDs.size());
		p_lightShdr->SetUniformInt("SHDW_SpotLightMapSize", p_renderContext->settings->spotLightShadowMapSize);
		p_lightShdr->SetUniformInt("SHDW_SpotLightCombineMapSize", p_renderContext->settings->spotLightCombineShadowMapSize);
		p_lightShdr->SetUniformFloat("SHDW_SpotLightBias", p_renderContext->settings->spotLightShadowBias);
		p_lightShdr->SetUniformFloat("SHDW_SpotLightSize", p_renderContext->settings->spotLightSize);
		p_renderContext->shadowMapSpotTex->Bind(14);
		p_lightShdr->SetUniformInt("SHDW_SpotLightMap", 14);

		p_renderContext->quadMesh->Bind();
		LowRenderer::Draw(p_renderContext->quadMesh->GetVertexArray());
		p_renderContext->frameInfo->drawCalls++;

		if (p_renderContext->IRMap && p_renderContext->prefilterMap && p_renderContext->brdfLUT)
		{
			p_renderContext->IRMap->Unbind(8);
			p_renderContext->prefilterMap->Unbind(9);
			p_renderContext->brdfLUT->Unbind(10);
		}

		p_renderContext->quadMesh->Unbind();

		p_lightShdr->Unbind();
		p_renderContext->otuputBuff->Unbind();
	}

	void DeferRenderBackend::GenerateIRMap()
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

		auto shader = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/cubemap/irradiance_map.shader");
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
	}

	void DeferRenderBackend::GeneratePrefilterMap()
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

		auto shader = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/cubemap/prefiltered_cube.shader");
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
	}

	void DeferRenderBackend::GenerateLUTMap()
	{
		m_renderContext.brdfLUT.reset();

		Buffers::FramebufferTexture texture;
		texture.format = Resources::TextureFormat::RG16;

		Buffers::FramebufferSettings settings;
		settings.globalHeight = 512;
		settings.globalWidth = 512;
		settings.colorTextureAttachments = texture;

		auto framebuffer = Buffers::Framebufffer::Create(settings);

		auto shader = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/cubemap/lut_map.shader");
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
	}

	PrCore::Math::vec4 DeferRenderBackend::CalculateLightTexture(size_t p_lightID, size_t p_lightMapSize, size_t p_comboMapSize)
	{
		size_t comboMapSize = p_comboMapSize;

		size_t rowCount = comboMapSize / p_lightMapSize;
		size_t row = p_lightID % rowCount;
		size_t column = p_lightID / rowCount;

		return { p_lightMapSize, p_lightMapSize, row * p_lightMapSize, column * p_lightMapSize };
	}
}