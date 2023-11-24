#include "Core/Common/pearl_pch.h"
#include "Renderer/Core/DefRendererBackend.h"

#include "Core/Events/WindowEvents.h"
#include "Renderer/Core/LowRenderer.h"
#include "Core/Resources/ResourceLoader.h"
#include "Core/Windowing/Window.h"
#include <Core/Events/EventManager.h>

namespace PrRenderer::Core
{
	DefRendererBackend::DefRendererBackend(const RendererSettings& p_settings) :
		IRendererBackend(p_settings)
	{
		//Prepare shaders
		m_shadowMappingShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Shadows/ShadowMapping.shader");
		m_postProcesShdr = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/RenderFront.shader");
		m_pbrLightShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/lightPass.shader");
		m_pointshadowMappingShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Shadows/PointShadowMapping.shader");

		m_renderData.m_quadMesh = Resources::Mesh::CreatePrimitive(Resources::Quad);

		GenerategBuffers();
		GenerateShadowMaps();
		
		//Set events
		PrCore::Events::EventListener windowResizedListener;
		windowResizedListener.connect<&DefRendererBackend::OnWindowResize>(this);
		PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	}

	void DefRendererBackend::PreRender()
	{
		//Temp variables
		const auto camera = m_frame->camera;

		//Prepare camera
		m_renderData.camera = camera;
		m_renderData.camera->RecalculateMatrices();

		//Rendering
		//---------------------------------

		//Clear back buffer
		m_commandQueue.push_back(CreateRC<LowRenderer::EnableDepthRC>(true));
		m_commandQueue.push_back(CreateRC<LowRenderer::EnableBlendingRC>(false));
		m_commandQueue.push_back(CreateRC<LowRenderer::EnableCullFaceRC>(true));
		m_commandQueue.push_back(CreateRC<LowRenderer::ClearRC>(ColorBuffer | DepthBuffer));

		//Calculate PBR Reflection if cubemap changed
		if((m_frame->renderFlag & RendererFlag::RerenderCubeMap) == RendererFlag::RerenderCubeMap && m_frame->cubemapObject)
		{
			GenerateIRMap();
			GeneratePrefilterMap();
			GenerateLUTMap();
		}


		//Shadow Mapping
		//---------------------------------
		static bool calculateProjs = true;
		if(calculateProjs)
		{
			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
				m_settings.cascadeShadowBordersCamSpace[i] = m_settings.cascadeShadowBorders[i] * camera->GetFar();

			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), camera->GetFar() * m_settings.cascadeShadowBorders[0]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings.cascadeShadowBorders[0], camera->GetFar() * m_settings.cascadeShadowBorders[1]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings.cascadeShadowBorders[1], camera->GetFar() * m_settings.cascadeShadowBorders[2]));
			m_CSMUtility.m_cameraProjs.push_back(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings.cascadeShadowBorders[2], camera->GetFar() * m_settings.cascadeShadowBorders[3]));

			calculateProjs = false;
		}


		//Main Directional Light
		if(m_frame->mainDirectLight)
		{
			auto mainLightMath = m_frame->mainDirectLight->lightMat;
			auto lightDir = PrCore::Math::vec3(mainLightMath[1][0], mainLightMath[1][1], mainLightMath[1][2]);

			m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
				{
					m_renderData.m_shadowMapMainDirBuff->Bind();
					LowRenderer::Clear(ColorBuffer | DepthBuffer);
				}));

			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
			{
				auto lightMat = m_CSMUtility.ClaculateFrustrums(m_settings.cascadeShadowRadiusRatio[i] ,i, lightDir, camera->GetViewMatrix(), 4096.0f, 4.0f);
				auto viewport = CalculateLightTexture(i, m_settings.mainLightShadowMapSize, m_settings.mainLightShadowCombineMapSize);
				m_frame->mainDirectLight->lightViewMats.push_back(lightMat);

				m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
				m_commandQueue.push_back(CreateRC<RenderToShadowMapRC>(m_shadowMappingShader, lightMat, &m_frame->shadowCasters, &m_renderData));
			}
		}


		//Directional Light
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderData.m_shadowMapDirBuff->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));

		for (auto& light : m_frame->lights)
		{
			if (light.lightMat[0].w != 0)
				continue;

			auto lightDir = PrCore::Math::vec3(light.lightMat[1][0], light.lightMat[1][1], light.lightMat[1][2]);

			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
			{
				auto lightMat = m_CSMUtility.ClaculateFrustrums(m_settings.cascadeShadowRadiusRatio[i], i, lightDir, camera->GetViewMatrix(), 2048.0f, 6.0f);
				auto viewport = CalculateLightTexture(light.shadowMapPos * SHADOW_CASCADES_COUNT + i, m_settings.dirLightShadowsMapSize, m_settings.dirLightCombineMapSize);
				light.lightViewMats.push_back(lightMat);

				m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
				m_commandQueue.push_back(CreateRC<RenderToShadowMapRC>(m_shadowMappingShader, lightMat, &m_frame->shadowCasters, &m_renderData));
			}

			m_settings.cascadeShadowRadiusRatio[1] = m_settings.cascadeShadowRadiusRatio[0] / m_settings.cascadeShadowRadiusRatio[1];
			m_settings.cascadeShadowRadiusRatio[2] = m_settings.cascadeShadowRadiusRatio[0] / m_settings.cascadeShadowRadiusRatio[2];
			m_settings.cascadeShadowRadiusRatio[3] = m_settings.cascadeShadowRadiusRatio[0] / m_settings.cascadeShadowRadiusRatio[3];
			m_settings.cascadeShadowRadiusRatio[0] = 1.0f;
		}


		//Point Lights
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderData.m_shadowMapPointBuff->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));

		for (auto& light : m_frame->lights)
		{
			if(light.lightMat[0].w !=  1)
				continue;

			auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), light.lightMat[3].w * 1.2f);
			auto lightPos = PrCore::Math::vec3(light.lightMat[0].x, light.lightMat[0].y, light.lightMat[0].z);

			auto lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			auto viewport = CalculateLightTexture(light.shadowMapPos + 0, m_settings.pointLightShadowMapSize, m_settings.pointLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(-1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(light.shadowMapPos + 1, m_settings.pointLightShadowMapSize, m_settings.pointLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 1.0f, 0.0), PrCore::Math::vec3(0.0f, 0.0f, 1.0f));
			viewport = CalculateLightTexture(light.shadowMapPos + 2, m_settings.pointLightShadowMapSize, m_settings.pointLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, -1.0f, 0.0f), PrCore::Math::vec3(0.0f, 0.0f, -1.0f));
			viewport = CalculateLightTexture(light.shadowMapPos + 3, m_settings.pointLightShadowMapSize, m_settings.pointLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, 1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(light.shadowMapPos + 4, m_settings.pointLightShadowMapSize, m_settings.pointLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, -1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculateLightTexture(light.shadowMapPos + 5, m_settings.pointLightShadowMapSize, m_settings.pointLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));
		}
		

		//Spot Lights
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderData.m_shadowMapSpotBuff->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));

		for (auto& light : m_frame->lights)
		{
			if (light.lightMat[0].w != 2)
				continue;

			auto lightPos = PrCore::Math::vec3(light.lightMat[0].x, light.lightMat[0].y, light.lightMat[0].z);
			auto lightDir = PrCore::Math::vec3(light.lightMat[1][0], light.lightMat[1][1], light.lightMat[1][2]);

			auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(120.0f), 1.0f, camera->GetNear(), camera->GetFar());
			auto rotationQaut = PrCore::Math::quat(PrCore::Math::radians(lightDir));
			auto lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + lightDir, PrCore::Math::vec3(0.0f, 0.0f, 1.0f));
			light.lightViewMats.push_back(lightProjMatrix* lightViewMatrix);

			auto viewport = CalculateLightTexture(light.shadowMapPos, m_settings.pointLightShadowMapSize, m_settings.spotLightCombineShadowMapSize);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToShadowMapRC>(m_shadowMappingShader, lightProjMatrix * lightViewMatrix, &m_frame->shadowCasters, &m_renderData));
		}

		m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(PrCore::Windowing::Window::GetMainWindow().GetWidth(),
			PrCore::Windowing::Window::GetMainWindow().GetHeight(), 0, 0));


		//gBuffer Pass
		//---------------------------------
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderData.gBuffer.buffer->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
				LowRenderer::EnableCullFace(true);
				LowRenderer::EnableDepth(true);
			}));

		for (auto object : m_frame->opaqueObjects)
			m_commandQueue.push_back(CreateRC<RenderToGBufferRC>(object, &m_renderData));

		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderData.gBuffer.buffer->Unbind();
			}));
		//---------------------------------

		//Clear Postprocess buffer
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderData.postProccesBuff->Bind();
				LowRenderer::EnableDepth(false);
				LowRenderer::EnableCullFace(false);
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
				m_renderData.postProccesBuff->Unbind();
			}));
		//---------------------------------

		//Draw Cubemap
		if(m_frame->cubemapObject)
			m_commandQueue.push_back(CreateRC<RenderCubeMapRC>(m_frame->cubemapObject->material, &m_renderData));

		//Copy depth buffer to postprocess buffer
		m_commandQueue.push_back(CreateRC<LowRenderer::BlitFrameBuffersRC>(m_renderData.gBuffer.buffer, m_renderData.postProccesBuff, Buffers::FramebufferMask::DepthBufferBit));

		//PBR Light Pass
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]() { m_renderData.postProccesBuff->Bind(); }));
		m_commandQueue.push_back(CreateRC<RenderLightRC>(m_pbrLightShader, m_frame->mainDirectLight, &m_frame->lights, &m_renderData, &m_settings));

		//Transparent Forward Pass
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
			{
				LowRenderer::EnableDepth(true);
				LowRenderer::EnableCullFace(true);
				LowRenderer::EnableBlending(true);
				LowRenderer::SetBlendingAlgorythm(BlendingAlgorithm::SourceAlpha, BlendingAlgorithm::OneMinusSourceAlpha);
			}));
		for (auto object : m_frame->transpatrentObjects)
			m_commandQueue.push_back(CreateRC<RenderTransparentRC>(object, &m_renderData));
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderData.postProccesBuff->Unbind();
				LowRenderer::EnableDepth(false);
				LowRenderer::EnableCullFace(false);
				LowRenderer::EnableBlending(false);
			}));
		//---------------------------------

		//Post process and render to back buffer
		m_commandQueue.push_back(CreateRC<RenderPostProcessRC>(m_postProcesShdr, &m_renderData));
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

		GenerategBuffers();
		m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(m_screenWidth, m_screenHeight, 0, 0));
	}

	void DefRendererBackend::RenderToGBuffer(RenderObjectPtr p_object, const RenderData* p_renderData)
	{
		//Temporary
		auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/gBuffer.shader");
		auto materialgBuffer = std::make_shared<Resources::Material>(PrRenderer::Resources::Material(shader));
		materialgBuffer->CopyPropertiesFrom(*p_object->material);
		//

		auto mesh = p_object->mesh;

		p_renderData->gBuffer.buffer->Bind();
		materialgBuffer->SetProperty("VPMatrix", p_renderData->camera->GetCameraMatrix());
		materialgBuffer->SetProperty("nearPlane", p_renderData->camera->GetNear());
		materialgBuffer->SetProperty("farPlane", p_renderData->camera->GetFar());

		if (p_object->type == RenderObjectType::Mesh)
		{
			materialgBuffer->SetProperty("modelMatrix", p_object->worldMat);

			materialgBuffer->Bind();
			mesh->Bind();
			LowRenderer::Draw(mesh->GetVertexArray());
			materialgBuffer->Unbind();
			mesh->Unbind();
		}
		else if(p_object->type == RenderObjectType::InstancedMesh)
		{
			materialgBuffer->Bind();
			mesh->Bind();
			LowRenderer::DrawInstanced(mesh->GetVertexArray(), p_object->instanceSize);
			materialgBuffer->Unbind();
			mesh->Unbind();
		}
	}

	void DefRendererBackend::RenderToShadowMap(Resources::ShaderPtr p_shaderPtr, PrCore::Math::mat4& p_lightMatrix, std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData)
	{
		p_shaderPtr->Bind();
		p_shaderPtr->SetUniformMat4("lightMatrix", p_lightMatrix);

		for (auto object : *p_objects)
		{
			if (object->type == RenderObjectType::Mesh)
			{
				p_shaderPtr->SetUniformMat4("modelMatrix", object->worldMat);
				p_shaderPtr->SetUniformInt("instancedCount", 0);
				object->mesh->Bind();
				LowRenderer::Draw(object->mesh->GetVertexArray());
				object->mesh->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_shaderPtr->SetUniformMat4Array("modelMatrixArray[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_shaderPtr->SetUniformInt("instancedCount", object->instanceSize);
				object->mesh->Bind();
				LowRenderer::DrawInstanced(object->mesh->GetVertexArray(), object->instanceSize);
				object->mesh->Unbind();
			}
		}

		p_shaderPtr->Unbind();
	}

	void DefRendererBackend::RenderToPointShadowMap(Resources::ShaderPtr p_pointShadowMapShader, PrCore::Math::mat4& p_lightMatrix, PrCore::Math::vec3& p_lightPos, std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData)
	{
		p_pointShadowMapShader->Bind();
		p_pointShadowMapShader->SetUniformMat4("lightMatrix", p_lightMatrix);
		p_pointShadowMapShader->SetUniformVec3("lightPos", p_lightPos);

		for (auto object : *p_objects)
		{
			if (object->type == RenderObjectType::Mesh)
			{
				p_pointShadowMapShader->SetUniformMat4("modelMatrix", object->worldMat);
				p_pointShadowMapShader->SetUniformInt("instancedCount", 0);
				object->mesh->Bind();
				LowRenderer::Draw(object->mesh->GetVertexArray());
				object->mesh->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_pointShadowMapShader->SetUniformMat4Array("modelMatrixArray[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_pointShadowMapShader->SetUniformInt("instancedCount", object->instanceSize);
				object->mesh->Bind();
				LowRenderer::DrawInstanced(object->mesh->GetVertexArray(), object->instanceSize);
				object->mesh->Unbind();
			}
		}

		p_pointShadowMapShader->Unbind();
	}

	void DefRendererBackend::GenerategBuffers()
	{
		//Explicitly reset old one
		m_renderData.gBuffer.positionTex.reset();
		m_renderData.gBuffer.albedoTex.reset();
		m_renderData.gBuffer.normalsTex.reset();
		m_renderData.gBuffer.aoTex.reset();
		m_renderData.gBuffer.buffer.reset();

		//Create new one
		Buffers::FramebufferTexture gPos;
		gPos.format = Resources::TextureFormat::RGBA16F;

		Buffers::FramebufferTexture gAlbedo;
		gAlbedo.format = Resources::TextureFormat::RGBA32;

		Buffers::FramebufferTexture gNormal;
		gNormal.format = Resources::TextureFormat::RGBA16F;

		Buffers::FramebufferTexture gAo;
		gAo.format = Resources::TextureFormat::RGBA16F;

		Buffers::FramebufferTexture gDepth;
		gDepth.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		settings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();;
		settings.mipMaped = false;
		settings.colorTextureAttachments = { gPos, gAlbedo, gNormal, gAo };
		settings.depthStencilAttachment = gDepth;

		auto framebuffer = Buffers::Framebufffer::Create(settings);
		m_renderData.gBuffer.positionTex = framebuffer->GetTexturePtr(0);
		m_renderData.gBuffer.albedoTex = framebuffer->GetTexturePtr(1);
		m_renderData.gBuffer.normalsTex = framebuffer->GetTexturePtr(2);
		m_renderData.gBuffer.aoTex = framebuffer->GetTexturePtr(3);
		m_renderData.gBuffer.buffer = framebuffer;


		//Combine post process framebuffer
		Buffers::FramebufferTexture postProcessColor;
		postProcessColor.format = Resources::TextureFormat::RGBA16F;

		Buffers::FramebufferTexture postProcessDepth;
		postProcessDepth.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings postProcessSettings;
		postProcessSettings.globalWidth = PrCore::Windowing::Window::GetMainWindow().GetWidth();
		postProcessSettings.globalHeight = PrCore::Windowing::Window::GetMainWindow().GetHeight();;
		postProcessSettings.mipMaped = false;
		postProcessSettings.colorTextureAttachments = postProcessColor;
		postProcessSettings.depthStencilAttachment = postProcessDepth;

		m_renderData.postProccesBuff = Buffers::Framebufffer::Create(postProcessSettings);
		m_renderData.postProccesTex = m_renderData.postProccesBuff->GetTexturePtr(0);
	}

	void DefRendererBackend::GenerateShadowMaps()
	{
		//Spot Lights Mapping
		Buffers::FramebufferTexture spotDepthTex;
		spotDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings spotLightSettings;
		spotLightSettings.globalWidth = m_settings.spotLightCombineShadowMapSize;
		spotLightSettings.globalHeight = m_settings.spotLightCombineShadowMapSize;;
		spotLightSettings.mipMaped = false;
		spotLightSettings.depthStencilAttachment = spotDepthTex;

		m_renderData.m_shadowMapSpotBuff = Buffers::Framebufffer::Create(spotLightSettings);
		m_renderData.m_shadowMapSpotTex = m_renderData.m_shadowMapSpotBuff->GetDepthTexturePtr();

		//Directional Light Mapping
		Buffers::FramebufferTexture dirDepthTex;
		dirDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings dirLightSettings;
		dirLightSettings.globalWidth = m_settings.dirLightCombineMapSize;
		dirLightSettings.globalHeight = m_settings.dirLightCombineMapSize;
		dirLightSettings.mipMaped = false;
		dirLightSettings.depthStencilAttachment = dirDepthTex;
		m_renderData.m_shadowMapDirBuff = Buffers::Framebufffer::Create(dirLightSettings);
		m_renderData.m_shadowMapDirTex = m_renderData.m_shadowMapDirBuff->GetDepthTexturePtr();


		//Point Lights Mapping
		Buffers::FramebufferTexture pointTex;
		pointTex.format = Resources::TextureFormat::RGBA16F;
		Buffers::FramebufferTexture pointDepthTex;
		pointDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings pointLightSettings;
		pointLightSettings.globalWidth = m_settings.pointLightCombineShadowMapSize;
		pointLightSettings.globalHeight = m_settings.pointLightCombineShadowMapSize;;
		pointLightSettings.mipMaped = false;
		pointLightSettings.colorTextureAttachments = pointTex;
		pointLightSettings.depthStencilAttachment = pointDepthTex;
		m_renderData.m_shadowMapPointBuff = Buffers::Framebufffer::Create(pointLightSettings);
		m_renderData.m_shadowMapPointTex = m_renderData.m_shadowMapPointBuff->GetTexturePtr(0);
		

		//Main Directional Light
		Buffers::FramebufferTexture mainDirDepthTex;
		mainDirDepthTex.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings mainDirSettings;
		mainDirSettings.globalWidth = m_settings.mainLightShadowCombineMapSize;
		mainDirSettings.globalHeight = m_settings.mainLightShadowCombineMapSize;
		mainDirSettings.mipMaped = false;
		mainDirSettings.depthStencilAttachment = mainDirDepthTex;
		m_renderData.m_shadowMapMainDirBuff = Buffers::Framebufffer::Create(mainDirSettings);
		m_renderData.m_shadowMapMainDirTex = m_renderData.m_shadowMapMainDirBuff->GetDepthTexturePtr();
	}

	void DefRendererBackend::RenderCubeMap(Resources::MaterialPtr p_material, const RenderData* p_renderData)
	{
		p_material->SetProperty("view", p_renderData->camera->GetViewMatrix());
		p_material->SetProperty("proj", p_renderData->camera->GetProjectionMatrix());

		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::LessEqual);

		p_material->Bind();
		p_renderData->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderData->m_quadMesh->GetVertexArray(), Primitives::TriangleStrip);
		p_renderData->m_quadMesh->Unbind();
		p_material->Unbind();
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);
	}

	void DefRendererBackend::RenderPostProcess(Resources::ShaderPtr p_postProcessShader, const RenderData* p_renderData)
	{
		LowRenderer::EnableDepth(false);
		LowRenderer::EnableCullFace(false);

		p_postProcessShader->Bind();
		p_renderData->postProccesTex->Bind(0);
		p_postProcessShader->SetUniformInt("lightTex", 0);
		p_renderData->m_quadMesh->Bind();

		LowRenderer::Draw(p_renderData->m_quadMesh->GetVertexArray());

		p_renderData->gBuffer.albedoTex->Unbind(0);
		p_renderData->m_quadMesh->Unbind();
		p_postProcessShader->Unbind();
	}

	void DefRendererBackend::RenderTransparent(RenderObjectPtr p_object, const RenderData* p_renderData)
	{
		auto material = p_object->material;
		auto mesh = p_object->mesh;

		material->SetProperty("VPMatrix", p_renderData->camera->GetCameraMatrix());
		material->SetProperty("camPos", p_renderData->camera->GetPosition());

		if (p_renderData->IRMap && p_renderData->prefilterMap && p_renderData->brdfLUT)
		{
			material->SetTexture("irradianceMap", p_renderData->IRMap);
			material->SetTexture("prefilterMap", p_renderData->prefilterMap);
			material->SetTexture("brdfLUT", p_renderData->brdfLUT);
		}

		if (p_object->type == RenderObjectType::Mesh)
		{
			material->SetProperty("modelMatrix", p_object->worldMat);
			material->SetProperty("instancedCount", 0);
			material->Bind();
			mesh->Bind();
			LowRenderer::Draw(mesh->GetVertexArray());
			mesh->Unbind();
		}
		else if (p_object->type == RenderObjectType::InstancedMesh)
		{
			material->SetPropertyArray("modelMatrixArray[0]", p_object->worldMatrices.data(), p_object->worldMatrices.size());
			material->SetProperty("instancedCount", (int)p_object->instanceSize);
			material->Bind();
			mesh->Bind();
			LowRenderer::DrawInstanced(mesh->GetVertexArray(), p_object->instanceSize);
			mesh->Unbind();
		}

		material->Unbind();
	}

	void DefRendererBackend::RenderLight(Resources::ShaderPtr p_lightShdr, LightObjectPtr mianDirectLight, std::vector<LightObject>* p_lightMats, const RenderData* p_renderData, const RendererSettings* p_settings)
	{
		p_renderData->postProccesBuff->Bind();

		p_lightShdr->Bind();

		// Set PBR Textures
		p_renderData->gBuffer.albedoTex->Bind(0);
		p_lightShdr->SetUniformInt("albedoMap", 0);

		p_renderData->gBuffer.normalsTex->Bind(1);
		p_lightShdr->SetUniformInt("normalMap", 1);

		p_renderData->gBuffer.positionTex->Bind(2);
		p_lightShdr->SetUniformInt("positionMap", 2);

		p_renderData->gBuffer.aoTex->Bind(3);
		p_lightShdr->SetUniformInt("aoMap", 3);

		if (p_renderData->IRMap && p_renderData->prefilterMap && p_renderData->brdfLUT)
		{
			p_renderData->IRMap->Bind(4);
			p_lightShdr->SetUniformInt("PBR_irradianceMap", 4);

			p_renderData->prefilterMap->Bind(5);
			p_lightShdr->SetUniformInt("PBR_prefilterMap", 5);

			p_renderData->brdfLUT->Bind(6);
			p_lightShdr->SetUniformInt("PBR_brdfLUT", 6);
		}

		p_lightShdr->SetUniformVec3("camPos", p_renderData->camera->GetPosition());



		// Set Shadows
		// Main Directional Light
		p_lightShdr->SetUniformFloatArray("SHDW_borders", p_settings->cascadeShadowBordersCamSpace, 4);
		p_lightShdr->SetUniformFloatArray("SHDW_RadiusRatio", p_settings->cascadeShadowRadiusRatio, 4);

		if (mianDirectLight && p_renderData->m_shadowMapMainDirTex)
		{
			p_lightShdr->SetUniformMat4("SHDW_MainDirLightMat", mianDirectLight->lightMat);
			p_lightShdr->SetUniformBool("SHDW_HasMainDirLight", true);
			p_lightShdr->SetUniformMat4Array("SHDW_MainDirLightViewMat", mianDirectLight->lightViewMats.data(), mianDirectLight->lightViewMats.size());
			p_lightShdr->SetUniformInt("SHDW_MainDirLightMapSize", p_settings->mainLightShadowMapSize);
			p_lightShdr->SetUniformInt("SHDW_MainDirLightCombineMapSize", p_settings->mainLightShadowCombineMapSize);
			p_renderData->m_shadowMapMainDirTex->Bind(7);
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
		for (auto light : *p_lightMats)
		{
			// Light does not cast shadow
			if (light.shadowMapPos == SIZE_MAX)
				continue;

			if (light.lightMat[0].w == 0)
			{
				dirLightMat.push_back(light.lightMat);
				for (auto& viewMat : light.lightViewMats)
					dirLightViewMat.push_back(viewMat);
				dirLightIDs.push_back(light.shadowMapPos);
			}
			else if (light.lightMat[0].w == 1)
			{
				pointlightMat.push_back(light.lightMat);
				pointLighttexPos.push_back(light.shadowMapPos);
			}
			else if (light.lightMat[0].w == 2)
			{
				spotLightMat.push_back(light.lightMat);
				spotLightViewMat.push_back(light.lightViewMats[0]);
				spotLightIDs.push_back(light.shadowMapPos);
			}
		}

		//Diretional Light
		p_lightShdr->SetUniformInt("SHDW_DirLightNumber", dirLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_DirLightMat", dirLightMat.data(), dirLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_DirLightViewMat", dirLightViewMat.data(), dirLightViewMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_DirLightID", dirLightIDs.data(), dirLightIDs.size());
		p_lightShdr->SetUniformInt("SHDW_DirLightMapSize", p_settings->dirLightShadowsMapSize);
		p_lightShdr->SetUniformInt("SHDW_DirLightCombineShadowMapSize", p_settings->dirLightCombineMapSize);
		p_renderData->m_shadowMapDirTex->Bind(8);
		p_lightShdr->SetUniformInt("SHDW_DirLightMap", 8);

		//Point Light
		p_lightShdr->SetUniformInt("SHDW_PointLightNumber", pointlightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_PointLightMat", pointlightMat.data(), pointlightMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_PointLightID", pointLighttexPos.data(), pointLighttexPos.size());
		p_lightShdr->SetUniformInt("SHDW_PointLightMapSize", p_settings->pointLightShadowMapSize);
		p_lightShdr->SetUniformInt("SHDW_PointCombineLightMapSize", p_settings->pointLightCombineShadowMapSize);
		p_renderData->m_shadowMapPointTex->Bind(9);
		p_lightShdr->SetUniformInt("SHDW_PointLightMap", 9);

		//Spot Light
		p_lightShdr->SetUniformInt("SHDW_SpotLightNumber", spotLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_SpotLightMat", spotLightMat.data(), spotLightMat.size());
		p_lightShdr->SetUniformMat4Array("SHDW_SpotLightViewMat", spotLightViewMat.data(), spotLightViewMat.size());
		p_lightShdr->SetUniformIntArray("SHDW_SpotLightID", spotLightIDs.data(), spotLightIDs.size());
		p_lightShdr->SetUniformInt("SHDW_SpotLightMapSize", p_settings->spotLightShadowMapSize);
		p_lightShdr->SetUniformInt("SHDW_SpotLightCombineMapSize", p_settings->spotLightCombineShadowMapSize);
		p_renderData->m_shadowMapSpotTex->Bind(10);
		p_lightShdr->SetUniformInt("SHDW_SpotLightMap", 10);

		p_renderData->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderData->m_quadMesh->GetVertexArray());
		p_renderData->m_quadMesh->Unbind();

		p_lightShdr->Unbind();
		p_renderData->postProccesBuff->Unbind();
	}

	void DefRendererBackend::GenerateIRMap()
	{
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

		m_renderData.IRMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr(0));

		shader->Unbind();
		cubemap->Unbind();
		shader->Unbind();
		cube->Unbind();
		framebuffer->Unbind();

		PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("IrradianceMap.shader");
	}

	void DefRendererBackend::GeneratePrefilterMap()
	{
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

		m_renderData.prefilterMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr());

		shader->Unbind();
		cubemap->Unbind();
		shader->Unbind();
		cube->Unbind();
		framebuffer->Unbind();

		PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>(shader->GetName());
	}

	void DefRendererBackend::GenerateLUTMap()
	{
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

		m_renderData.brdfLUT = framebuffer->GetTexturePtr();

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
