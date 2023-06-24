#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Camera.h"
#include"Renderer/Core/RenderCommand.h"
#include"Renderer/Core/LowRenderer.h"

using namespace PrRenderer::Core;

void MeshRenderRC::Invoke(const Camera* p_camera)
{
	PrCore::Math::mat4 VPMatrix = PrCore::Math::mat4(1.0f);

	auto material = m_meshRenderObject.material;
	auto mesh = m_meshRenderObject.mesh;

	if (material->HasProperty("camPos"))
		material->SetProperty("camPos", p_camera->GetPosition());

	if (material->HasProperty("VPMatrix"))
		material->SetProperty("VPMatrix", p_camera->GetCameraMatrix());

	if (material->HasProperty("modelMatrix"))
		material->SetProperty("modelMatrix", m_meshRenderObject.worldMat);

	if (material->HasProperty("MVP"))
		material->SetProperty("MVP", VPMatrix * m_meshRenderObject.worldMat);

	if (material->HasProperty("ambientColor"))
		material->SetProperty("ambientColor", m_renderData.ambientColor);

	//Render cubemap reflections
	if (m_renderData.hasCubeMap)
	{
		material->SetTexture("irradianceMap", m_renderData.irradianceMap);
		material->SetTexture("prefilterMap", m_renderData.prefilterMap);
		material->SetTexture("brdfLUT", m_renderData.brdfLUT);
	}

	//Render light
	auto lightData = m_renderData.lightData;
	if (!lightData.empty())
	{
		if (material->HasProperty("lightMat[0]"))
			material->SetPropertyArray("lightMat[0]", lightData.data(), lightData.size());

		if (material->HasProperty("lightNumber"))
			material->SetProperty("lightNumber", (int)lightData.size());
	}

	material->Bind();
	mesh->Bind();
	
	LowRenderer::Draw(mesh->GetVertexArray());
	material->Unbind();
	mesh->Unbind();
}

void InstancedMeshesRC::Invoke(const Camera* p_camera)
{
	PrCore::Math::mat4 VPMatrix = PrCore::Math::mat4(1.0f);

	auto material = m_instancedMeshObject.material;
	auto mesh = m_instancedMeshObject.mesh;

	if (material->HasProperty("camPos"))
		material->SetProperty("camPos", p_camera->GetPosition());

	if (material->HasProperty("VPMatrix"))
		material->SetProperty("VPMatrix", p_camera->GetCameraMatrix());

	if (material->HasProperty("modelMatrixArray[0]"))
		material->SetPropertyArray("modelMatrixArray[0]", m_instancedMeshObject.wordMatrices.data(), m_instancedMeshObject.wordMatrices.size());

	if (material->HasProperty("ambientColor"))
		material->SetProperty("ambientColor", m_renderData.ambientColor);

	//Render cubemap reflections
	if (m_renderData.hasCubeMap)
	{
		material->SetTexture("irradianceMap", m_renderData.irradianceMap);
		material->SetTexture("prefilterMap", m_renderData.prefilterMap);
		material->SetTexture("brdfLUT", m_renderData.brdfLUT);
	}

	//Render light
	auto lightData = m_renderData.lightData;
	if (!lightData.empty())
	{
		if (material->HasProperty("lightMat[0]"))
			material->SetPropertyArray("lightMat[0]", lightData.data(), lightData.size());

		if (material->HasProperty("lightNumber"))
			material->SetProperty("lightNumber", (int)lightData.size());
	}

	material->Bind();
	mesh->Bind();

	LowRenderer::DrawInstanced(mesh->GetVertexArray(), m_instancedMeshObject.instanceCount);
	material->Unbind();
	mesh->Unbind();
}

void CubemapRenderRC::Invoke(const Camera* p_camera)
{
	m_cubemapMat->SetProperty("view", p_camera->GetViewMatrix());
	m_cubemapMat->SetProperty("proj", p_camera->GetProjectionMatrix());

	LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::LessEqual);

	m_cubemapMat->Bind();
	m_cubemapMesh->Bind();
	LowRenderer::Draw(m_cubemapMesh->GetVertexArray(), Primitives::TriangleStrip);
	m_cubemapMesh->Unbind();
	m_cubemapMat->Unbind();
	LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);
}

void TransparentMeshRenderRC::Invoke(const Camera* p_camera)
{
	LowRenderer::EnableBlending(true);
	LowRenderer::SetBlendingAlgorythm(BlendingAlgorithm::SourceAlpha, BlendingAlgorithm::OneMinusSourceAlpha);

	PrCore::Math::mat4 VPMatrix = PrCore::Math::mat4(1.0f);

	auto material = m_meshRenderObject.material;
	auto mesh = m_meshRenderObject.mesh;

	if (material->HasProperty("camPos"))
		material->SetProperty("camPos", p_camera->GetPosition());

	if (material->HasProperty("VPMatrix"))
		material->SetProperty("VPMatrix", p_camera->GetCameraMatrix());

	if (material->HasProperty("modelMatrix"))
		material->SetProperty("modelMatrix", m_meshRenderObject.worldMat);

	if (material->HasProperty("MVP"))
		material->SetProperty("MVP", VPMatrix * m_meshRenderObject.worldMat);

	if (material->HasProperty("ambientColor"))
		material->SetProperty("ambientColor", m_renderData.ambientColor);

	material->SetProperty("transparent", true);

	//Render cubemap reflections
	if (m_renderData.hasCubeMap)
	{
		material->SetTexture("irradianceMap", m_renderData.irradianceMap);
		material->SetTexture("prefilterMap", m_renderData.prefilterMap);
		material->SetTexture("brdfLUT", m_renderData.brdfLUT);
	}

	//Render light
	auto lightData = m_renderData.lightData;
	if (!lightData.empty())
	{
		if (material->HasProperty("lightMat[0]"))
			material->SetPropertyArray("lightMat[0]", lightData.data(), lightData.size());

		if (material->HasProperty("lightNumber"))
			material->SetProperty("lightNumber", (int)lightData.size());
	}

	material->Bind();
	mesh->Bind();
	
	LowRenderer::Draw(mesh->GetVertexArray());
	material->Unbind();
	mesh->Unbind();
	LowRenderer::EnableBlending(false);
}
