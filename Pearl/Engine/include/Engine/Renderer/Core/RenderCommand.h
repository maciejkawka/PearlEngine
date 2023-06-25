#pragma once

#include"Renderer/Core/MeshRenderObject.h"
#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::Core
{
	class Camera;
	struct RenderData;

	class RenderCommand {
	public:
		RenderCommand() = delete;
		RenderCommand(const RenderData* p_renderData):
		m_renderData(p_renderData)
		{}
		virtual ~RenderCommand() = default;

		virtual void Invoke() = 0;

	protected:
		const RenderData* m_renderData;
	};

	class StateChangeRC: public RenderCommand {
	public:
		using StateChangeFunction = std::function<void()>;
		StateChangeRC(StateChangeFunction p_stateCommand):
		RenderCommand(nullptr)
		{}

		void Invoke() override;

	private:
		StateChangeFunction m_stateCommand;
	};

	class MeshRenderRC: public RenderCommand {
	public:
		MeshRenderRC() = delete;
		MeshRenderRC(MeshRenderObject&& p_meshRenderObject, const RenderData* p_renderData):
			RenderCommand(p_renderData),
			m_meshRenderObject(p_meshRenderObject)
		{}

		void Invoke() override;

	protected:
		MeshRenderObject m_meshRenderObject;
	};

	class InstancedMeshesRC : public RenderCommand {
	public:
		InstancedMeshesRC() = delete;
		InstancedMeshesRC(InstancedMeshObject&& p_instancedMeshObject, const RenderData* p_renderData) :
			RenderCommand(p_renderData),
			m_instancedMeshObject(p_instancedMeshObject)
		{}

		void Invoke() override;

	protected:
		InstancedMeshObject m_instancedMeshObject;
	};

	class CubemapRenderRC: public RenderCommand {
	public:
		CubemapRenderRC() = delete;
		CubemapRenderRC(PrRenderer::Resources::MaterialPtr p_cubemapMat, const RenderData* p_renderData):
			RenderCommand(p_renderData),
			m_cubemapMat(p_cubemapMat)
		{
			if(m_cubemapMesh == nullptr)
				m_cubemapMesh = Resources::Mesh::CreatePrimitive(Resources::Quad);
		}

		void Invoke() override;

	protected:
		//Maintain static quad for all cubemaps
		inline static Resources::MeshPtr m_cubemapMesh = nullptr;
		Resources::MaterialPtr m_cubemapMat;
	};
}