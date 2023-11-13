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

		//Cascade Shadow Mapping
		m_settings.cascadeShadowBorders[0] = 0.1f;
		m_settings.cascadeShadowBorders[1] = 0.2f;
		m_settings.cascadeShadowBorders[2] = 0.5f;
		m_settings.cascadeShadowBorders[3] = 1.0f;
		m_settings.cascadeShadowMapSize = 4096;
		m_renderData.CSM = std::make_unique<CascadeShadowMapper>(SHADOW_CASCADES_COUNT, m_settings.cascadeShadowMapSize);

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

		//Cascade Shadow Mapping
		if(m_frame->mainDirectLight)
		{
			auto mainLightMath = m_frame->mainDirectLight->lightMat;
			auto lightDir = PrCore::Math::vec3(mainLightMath[1][0], mainLightMath[1][1], mainLightMath[1][2]);

			m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
				{
					m_renderData.m_cascadeShadow->Bind();
					LowRenderer::Clear(ColorBuffer | DepthBuffer);
				}));

			//Recalculate CSM matrices if camera properties changed
			m_frame->renderFlag |= RendererFlag::RecalculateProjectionMatrix;
			if ((m_frame->renderFlag & RendererFlag::RecalculateProjectionMatrix) == RendererFlag::RecalculateProjectionMatrix)
			{
				for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
					m_renderData.CSM->SetBorder(m_settings.cascadeShadowBorders[i] * camera->GetFar(), i);
				m_renderData.CSM->SetCameraProj(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), camera->GetFar() * m_settings.cascadeShadowBorders[0]), 0);
				m_renderData.CSM->SetCameraProj(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings.cascadeShadowBorders[0], camera->GetFar() * m_settings.cascadeShadowBorders[1]), 1);
				m_renderData.CSM->SetCameraProj(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings.cascadeShadowBorders[1], camera->GetFar() * m_settings.cascadeShadowBorders[2]), 2);
				m_renderData.CSM->SetCameraProj(PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetFar() * m_settings.cascadeShadowBorders[2], camera->GetFar() * m_settings.cascadeShadowBorders[3]), 3);
			}
						
			for (int i = 0; i < SHADOW_CASCADES_COUNT; i++)
			{
				auto lightMat = m_renderData.CSM->ClaculateFrustrums(lightDir, camera->GetViewMatrix(), i);
				auto viewport = m_renderData.CSM->CalculateShadowMapCoords(i);
				m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
				m_commandQueue.push_back(CreateRC<RenderToCascadeShadowMapRC>(m_shadowMappingShader, lightMat, &m_frame->shadowCasters, &m_renderData));
			}

			m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
				{
					m_renderData.m_cascadeShadow->Unbind();
				}));
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(PrCore::Windowing::Window::GetMainWindow().GetWidth(),
				PrCore::Windowing::Window::GetMainWindow().GetHeight(), 0, 0));
		}

		//Shadow Mapping

		//Render to point lights shadow map
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderData.m_shadowMapPoint->Bind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));


		PrCore::Math::vec3 lightDirs[6] = {
			PrCore::Math::vec3(1,0,0),
			PrCore::Math::vec3(-1,0,0),
			PrCore::Math::vec3(0,1,0),
			PrCore::Math::vec3(0,-1,0),
			PrCore::Math::vec3(0,0,1),
			PrCore::Math::vec3(0,0,-1)
		};

		for (auto& light : m_frame->lights)
		{
			//auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), light.lightMat[3].w * 1.5f);
			//auto lightPos = PrCore::Math::vec3(light.lightMat[0].x, light.lightMat[0].y, light.lightMat[0].z);
			//auto forward = glm::vec3(1.0f, 0.0f, 0.0f);
			//auto lookAtMat = PrCore::Math::lookAt(lightPos, lightPos + forward, PrCore::Math::vec3(0.0f, 1.0f, 0.0f));
			//light.lightViewMat = lightProjMatrix * lookAtMat;

			//for (int i = 0; i < 6; i++)
			//{
			//	auto viewport = CalculatePointLightTexture(light.shadowMapPos + i);
			//	m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));

			//	auto rotation = PrCore::Math::rotation(forward, lightDirs[i]);
			//	auto rotationMatrix = PrCore::Math::mat4_cast(rotation);
			//	auto lightViewMatrix = lookAtMat * rotationMatrix;
			//	auto lightTransformMatrix = lightProjMatrix * lightViewMatrix;

			//	m_commandQueue.push_back(CreateRC<RenderToCascadeShadowMapRC>(m_shadowMappingShader, lightTransformMatrix, &m_frame->shadowCasters, &m_renderData));
			//}

			auto lightProjMatrix = PrCore::Math::perspective(PrCore::Math::radians(90.0f), 1.0f, camera->GetNear(), light.lightMat[3].w * 1.2f);
			auto lightPos = PrCore::Math::vec3(light.lightMat[0].x, light.lightMat[0].y, light.lightMat[0].z);

			auto lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			auto viewport = CalculatePointLightTexture(light.shadowMapPos + 0);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(-1.0f, 0.0f, 0.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculatePointLightTexture(light.shadowMapPos + 1);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 1.0f, 0.0), PrCore::Math::vec3(0.0f, 0.0f, 1.0f));
			viewport = CalculatePointLightTexture(light.shadowMapPos + 2);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, -1.0f, 0.0f), PrCore::Math::vec3(0.0f, 0.0f, -1.0f));
			viewport = CalculatePointLightTexture(light.shadowMapPos + 3);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, 1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculatePointLightTexture(light.shadowMapPos + 4);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));

			lightViewMatrix = PrCore::Math::lookAt(lightPos, lightPos + PrCore::Math::vec3(0.0f, 0.0f, -1.0f), PrCore::Math::vec3(0.0f, -1.0f, 0.0f));
			viewport = CalculatePointLightTexture(light.shadowMapPos + 5);
			m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
			m_commandQueue.push_back(CreateRC<RenderToPointShadowMapRC>(m_pointshadowMappingShader, lightProjMatrix * lightViewMatrix, lightPos, &m_frame->shadowCasters, &m_renderData));
		}

		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]
			{
				m_renderData.m_shadowMapPoint->Unbind();
				LowRenderer::Clear(ColorBuffer | DepthBuffer);
			}));
		m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(PrCore::Windowing::Window::GetMainWindow().GetWidth(),
			PrCore::Windowing::Window::GetMainWindow().GetHeight(), 0, 0));


		//Iterate Direct and Stop Lights





		//m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
		//	{
		//		LowRenderer::EnableCullFace(true);
		//		LowRenderer::EnableDepth(true);
		//
		//		m_renderData.m_shadowMapOther->Bind();
		//		LowRenderer::Clear(ColorBuffer | DepthBuffer);
		//		m_renderData.m_shadowMapPoint->Bind();
		//		LowRenderer::Clear(ColorBuffer | DepthBuffer);
		//	}));
		//for(auto& light : m_frame->lights)
		//{
		//	//Chceck if light cast shadows
		//	if (light.shadowMapPos != SIZE_MAX)
		//	{
		//		auto viewport = CalculateShadowMapCoords(light.shadowMapPos, m_settings.lightShadowMapSize, m_settings.lightShadowMapSize);
		//		m_commandQueue.push_back(CreateRC<LowRenderer::SetViewportRC>(viewport.x, viewport.y, viewport.z, viewport.w));
		//
		//		m_commandQueue.push_back(CreateRC<RenderToShadowMapRC>(m_shadowMappingShader, light, &m_frame->opaqueObjects, &m_renderData));
		//	}
		//}
		//---------------------------------


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
		m_commandQueue.push_back(CreateRC<RenderLightRC>(m_pbrLightShader, m_frame->mainDirectLight, &m_frame->lights, &m_renderData));

		//Transparent Forward Pass
		m_commandQueue.push_back(CreateRC<LambdaFunctionRC>([&]()
			{
				m_renderData.postProccesBuff->Bind();
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

	void DefRendererBackend::RenderToShadowMap(Resources::ShaderPtr p_shadowsShdr, const LightObject& p_lightObject, const std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData)
	{
		using namespace PrCore;

		p_shadowsShdr->Bind();

		auto lightMat = p_lightObject.lightMat;
		Math::mat4 projMat = Math::ortho(-10.0f, 10.0f, -10.0f, 10.0f, p_renderData->camera->GetFar(), p_renderData->camera->GetNear());
		auto lightPos = Math::vec3(lightMat[0][0], lightMat[0][1], lightMat[0][2]);
		auto lightDir = Math::vec3(lightMat[1][0], lightMat[1][1], lightMat[1][2]);
		Math::mat4 lightView = Math::lookAt(lightPos, lightPos + lightDir, Math::vec3(0.0f, 1.0f, 0.0f));
		Math::mat4 lightSpaceMatrix = projMat * lightView;

		p_shadowsShdr->SetUniformMat4("lightMatrix", lightSpaceMatrix);

		//frustrum culling
		//????
		//---------------

		//Draw all objects for the light
		for(auto object: *p_objects)
		{
			if(object->type == RenderObjectType::Mesh)
			{
				p_shadowsShdr->SetUniformMat4("modelMatrix", object->worldMat);
				LowRenderer::Draw(object->mesh->GetVertexArray());
			}
			else if(object->type == RenderObjectType::InstancedMesh)
			{
				p_shadowsShdr->SetUniformMat4Array("modelMatrixArray[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_shadowsShdr->SetUniformInt("instancedCount", object->instanceSize);
				LowRenderer::DrawInstanced(object->mesh->GetVertexArray(), object->instanceSize);
			}
		}

		p_shadowsShdr->Unbind();
	}

	void DefRendererBackend::RenderToCascadeShadowMap(Resources::ShaderPtr p_cascadesShadowsShdr, PrCore::Math::mat4& p_lightMatrix, std::list<RenderObjectPtr>* p_objects, const RenderData* p_renderData)
	{
		p_cascadesShadowsShdr->Bind();
		p_cascadesShadowsShdr->SetUniformMat4("lightMatrix", p_lightMatrix);

		for (auto object : *p_objects)
		{
			if (object->type == RenderObjectType::Mesh)
			{
				p_cascadesShadowsShdr->SetUniformMat4("modelMatrix", object->worldMat);
				p_cascadesShadowsShdr->SetUniformInt("instancedCount", 0);
				object->mesh->Bind();
				LowRenderer::Draw(object->mesh->GetVertexArray());
				object->mesh->Unbind();
			}
			else if (object->type == RenderObjectType::InstancedMesh)
			{
				p_cascadesShadowsShdr->SetUniformMat4Array("modelMatrixArray[0]", object->worldMatrices.data(), object->worldMatrices.size());
				p_cascadesShadowsShdr->SetUniformInt("instancedCount", object->instanceSize);
				object->mesh->Bind();
				LowRenderer::DrawInstanced(object->mesh->GetVertexArray(), object->instanceSize);
				object->mesh->Unbind();
			}
		}

		p_cascadesShadowsShdr->Unbind();
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
		//Normal shadow mapping
		//Other Lights Mapping
		Buffers::FramebufferTexture gDepth;
		gDepth.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings settings;
		settings.globalWidth = m_settings.comboShadowMap;
		settings.globalHeight = m_settings.comboShadowMap;;
		settings.mipMaped = false;
		settings.depthStencilAttachment = gDepth;

		m_renderData.m_shadowMapOther = Buffers::Framebufffer::Create(settings);

		//Point Lights Mapping
		Buffers::FramebufferTexture distColor;
		distColor.format = Resources::TextureFormat::RGBA16F;

		Buffers::FramebufferSettings settingsPoint;
		settingsPoint.globalWidth = m_settings.comboShadowMap;
		settingsPoint.globalHeight = m_settings.comboShadowMap;;
		settingsPoint.mipMaped = false;
		settingsPoint.colorTextureAttachments = distColor;
		settingsPoint.depthStencilAttachment = gDepth;
		m_renderData.m_shadowMapPoint = Buffers::Framebufffer::Create(settingsPoint);
		m_renderData.m_shadowMapPointTex = m_renderData.m_shadowMapPoint->GetTexturePtr(0);
		

		//Cascade shadow mapping
		Buffers::FramebufferTexture gDepthCascades;
		gDepthCascades.format = Resources::TextureFormat::Depth32;

		Buffers::FramebufferSettings settingsCascades;
		settingsCascades.globalWidth = m_settings.cascadeShadowMapSize * SHADOW_CASCADES_COUNT / 2;
		settingsCascades.globalHeight = m_settings.cascadeShadowMapSize * SHADOW_CASCADES_COUNT / 2;
		settingsCascades.mipMaped = false;
		settingsCascades.depthStencilAttachment = gDepthCascades;
		m_renderData.m_cascadeShadow = Buffers::Framebufffer::Create(settingsCascades);
		m_renderData.m_CSMShadowMap = m_renderData.m_cascadeShadow->GetDepthTexturePtr();
	}

	void DefRendererBackend::RenderCubeMap(Resources::MaterialPtr p_material, const RenderData* p_renderData)
	{
		p_renderData->postProccesBuff->Bind();

		p_material->SetProperty("view", p_renderData->camera->GetViewMatrix());
		p_material->SetProperty("proj", p_renderData->camera->GetProjectionMatrix());

		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::LessEqual);

		p_material->Bind();
		p_renderData->m_quadMesh->Bind();
		LowRenderer::Draw(p_renderData->m_quadMesh->GetVertexArray(), Primitives::TriangleStrip);
		p_renderData->m_quadMesh->Unbind();
		p_material->Unbind();
		LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);

		p_renderData->postProccesBuff->Unbind();
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

	void DefRendererBackend::RenderLight(Resources::ShaderPtr p_lightShdr, LightObjectPtr mianDirectLight, std::vector<LightObject>* p_lightMats, const RenderData* p_renderData)
	{
		p_renderData->postProccesBuff->Bind();

		p_lightShdr->Bind();
		//Bind Textures
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

		// Render Main CSM Light
		if (mianDirectLight && p_renderData->m_CSMShadowMap)
		{
			p_lightShdr->SetUniformMat4("CSM_mainLight", mianDirectLight->lightMat);
			p_lightShdr->SetUniformBool("CSM_hasMainLight", true);
			p_lightShdr->SetUniformMat4Array("CSM_viewMats[0]", p_renderData->CSM->GetLightMats(), 4);
			p_lightShdr->SetUniformFloatArray("CSM_borders[0]", p_renderData->CSM->GetBorders(), 4);
			p_lightShdr->SetUniformInt("CSM_mapSize", p_renderData->CSM->GetMapSize());

			p_renderData->m_CSMShadowMap->Bind(7);
			p_lightShdr->SetUniformInt("CSM_map", 7);
		}
		else
			p_lightShdr->SetUniformBool("CSM_hasMainLight", false);

		// Render Shadows
		{
			p_lightShdr->SetUniformInt("pointLightMapSize", 512);
			p_lightShdr->SetUniformInt("shadowMapSize", 8192);

			p_renderData->m_shadowMapPointTex->Bind(8);
			p_lightShdr->SetUniformInt("pointLightShadowMap", 8);
		}
		
		std::vector<PrCore::Math::mat4> lightMat;
		std::vector<PrCore::Math::mat4> lightViewMat;
		std::vector<int> lightIDs;
		for (auto light : *p_lightMats)
		{
			lightMat.push_back(light.lightMat);
			lightViewMat.push_back(light.lightViewMat);
			lightIDs.push_back(light.shadowMapPos);
		}

		p_lightShdr->SetUniformMat4Array("lightMat[0]", lightMat.data(), lightMat.size());
		p_lightShdr->SetUniformMat4Array("lightViewMatrices", lightViewMat.data(), lightViewMat.size());
		p_lightShdr->SetUniformIntArray("lightID", lightIDs.data(), lightIDs.size());
		p_lightShdr->SetUniformInt("lightNumber", lightMat.size());


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

	PrCore::Math::vec4 DefRendererBackend::CalculatePointLightTexture(size_t p_lightID)
	{
		size_t width = m_settings.comboShadowMap;
		size_t height = m_settings.comboShadowMap;

		size_t rowCount = width / m_settings.pointLightShadowMapSize;
		size_t row = p_lightID % rowCount;
		size_t column = p_lightID / rowCount;

		return { m_settings.pointLightShadowMapSize, m_settings.pointLightShadowMapSize, row * m_settings.pointLightShadowMapSize, column * m_settings.pointLightShadowMapSize };
	}
}
