#pragma once

#include"Renderer/Core/MeshRenderObject.h"
#include"Renderer/Resources/Cubemap.h"
#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::Core
{
	class Camera;

	class RenderCommand {
	public:
		virtual ~RenderCommand() = default;
		virtual void Invoke(const Camera* p_camera) = 0;
	};


	struct RenderData {
		const std::vector<PrCore::Math::mat4>& lightData;
		const PrRenderer::Resources::CubemapPtr irradianceMap;
		const PrRenderer::Resources::CubemapPtr prefilterMap;
		const PrRenderer::Resources::TexturePtr brdfLUT;
		const PrCore::Math::vec3& ambientColor;

		bool hasCubeMap;
	};

	class MeshRenderRC: public RenderCommand {
	public:
		MeshRenderRC() = delete;
		MeshRenderRC(MeshRenderObject&& p_meshRenderObject, const RenderData& p_renderData):
			m_meshRenderObject(p_meshRenderObject),
			m_renderData(p_renderData)
		{}
		~MeshRenderRC()
		{}

		void Invoke(const Camera* p_camera) override;

	protected:
		MeshRenderObject m_meshRenderObject;
		RenderData m_renderData;
	};

	class TransparentMeshRenderRC: public MeshRenderRC {
	public:
		TransparentMeshRenderRC() = delete;
		TransparentMeshRenderRC(MeshRenderObject&& p_meshRenderObject, const RenderData& p_renderData):
		MeshRenderRC(std::move(p_meshRenderObject), p_renderData)
		{}

		void Invoke(const Camera* p_camera) override;
	};

	class InstancedMeshesRC : public MeshRenderRC {
		InstancedMeshesRC() = delete;
		InstancedMeshesRC(MeshRenderObject&& p_meshRenderObject, const RenderData& p_renderData) :
			MeshRenderRC(std::move(p_meshRenderObject), p_renderData)
		{}

		void Invoke(const Camera* p_camera) override;
	};

	class CubemapRenderRC: public RenderCommand {
	public:
		CubemapRenderRC() = delete;
		CubemapRenderRC(PrRenderer::Resources::MaterialPtr p_cubemapMat):
			m_cubemapMat(p_cubemapMat)
		{
			if(m_cubemapMesh == nullptr)
				m_cubemapMesh = Resources::Mesh::CreatePrimitive(Resources::Quad);
		}

		void Invoke(const Camera* p_camera) override;

	protected:
		//Maintain static quad for all cubemaps
		inline static Resources::MeshPtr m_cubemapMesh = nullptr;
		Resources::MaterialPtr m_cubemapMat;
	};
}