#pragma once

#include"Renderer/Core/MeshRenderObject.h"
#include"Renderer/Resources/Mesh.h"

namespace PrRenderer::Core
{
	class RenderCommand {
	public:
		RenderCommand() = default;
		virtual ~RenderCommand() = default;

		virtual void Invoke() = 0;
	};

	using RenderCommandPtr = std::shared_ptr<RenderCommand>;

#define BASIC_RENDER_COMMAND(CommandName, FunctionName, ...)			 \
	class CommandName : public RenderCommand {					         \
	public:                                                              \
	typedef std::tuple<__VA_ARGS__> CommandArgs;                         \
		explicit CommandName(CommandArgs&& args) :                                \
		m_args(std::forward<CommandArgs>(args))                          \
		{}                                                               \
		void Invoke() override                                           \
		{                                                                \
		auto func = [](auto&&... args) {                                 \
                return FunctionName(std::forward<decltype(args)>(args)...); \
            };                                                           \
			std::apply(func, m_args);                                    \
		}                                                                \
	private:                                                             \
		CommandArgs m_args;												 \
	}

#define REGISTER_RENDER_COMMAND(CommandName, FunctionName, ...) \
	BASIC_RENDER_COMMAND(CommandName ## RC, FunctionName, __VA_ARGS__)

	inline void LambdaFunction(std::function<void(void)> p_lambda)
	{
		p_lambda();
	}
	REGISTER_RENDER_COMMAND(LambdaFunction, LambdaFunction, std::function<void(void)>);

	template<class CommandType, typename... Args>
	inline RenderCommandPtr CreateRC(Args&&... args)
	{
		return std::make_shared<CommandType>(std::make_tuple(std::forward<Args>(args)...));
	}











	///////
	///TO REMOVE

	class Camera;
	struct RenderData;

	class RenderCommandOld {
	public:
		RenderCommandOld() = delete;
		RenderCommandOld(const RenderData* p_renderData):
		m_renderData(p_renderData)
		{}
		virtual ~RenderCommandOld() = default;

		virtual void Invoke() = 0;

	protected:
		const RenderData* m_renderData;
	};

	class StateChangeRC: public RenderCommandOld {
	public:
		using StateChangeFunction = std::function<void()>;
		StateChangeRC(StateChangeFunction p_stateCommand):
		RenderCommandOld(nullptr)
		{}

		void Invoke() override;

	private:
		StateChangeFunction m_stateCommand;
	};

	class MeshRenderRC: public RenderCommandOld {
	public:
		MeshRenderRC() = delete;
		MeshRenderRC(MeshRenderObject&& p_meshRenderObject, const RenderData* p_renderData):
			RenderCommandOld(p_renderData),
			m_meshRenderObject(p_meshRenderObject)
		{}

		void Invoke() override;

	protected:
		MeshRenderObject m_meshRenderObject;
	};

	class InstancedMeshesRC : public RenderCommandOld {
	public:
		InstancedMeshesRC() = delete;
		InstancedMeshesRC(InstancedMeshObject&& p_instancedMeshObject, const RenderData* p_renderData) :
			RenderCommandOld(p_renderData),
			m_instancedMeshObject(p_instancedMeshObject)
		{}

		void Invoke() override;

	protected:
		InstancedMeshObject m_instancedMeshObject;
	};

	class CubemapRenderRC: public RenderCommandOld {
	public:
		CubemapRenderRC() = delete;
		CubemapRenderRC(PrRenderer::Resources::MaterialPtr p_cubemapMat, const RenderData* p_renderData):
			RenderCommandOld(p_renderData),
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
}//#pragma once
//
//#include"Renderer/Core/MeshRenderObject.h"
//#include"Renderer/Resources/Mesh.h"
//
//namespace PrRenderer::Core
//{
//	class Camera;
//	struct RenderData;
//
//	class RenderCommandOld {
//	public:
//		RenderCommandOld() = delete;
//		RenderCommandOld(const RenderData* p_renderData):
//		m_renderData(p_renderData)
//		{}
//		virtual ~RenderCommandOld() = default;
//
//		virtual void Invoke() = 0;
//
//	protected:
//		const RenderData* m_renderData;
//	};
//
//#define BASIC_RENDER_COMMAND(CommandName, FunctionName, ...)			 \
//	class CommandName : public RenderCommandOld {					         \
//	public:                                                              \
//	typedef std::tuple<__VA_ARGS__> CommandArgs;						 \
//		CommandName(CommandArgs&& args) :                                \
//		RenderCommandOld(nullptr),                                          \
//		m_args(std::forward<CommandArgs>(args))                          \
//		{}                                                               \
//		void Invoke() override                                           \
//		{                                                                \
//		auto func = [](auto&&... args) {                                 \
//                return FunctionName(std::forward<decltype(args)>(args)...); \
//            };                                                           \
//			std::apply(func, m_args);                                    \
//		}                                                                \
//	private:                                                             \
//		CommandArgs m_args;												 \
//	};
//
//
//
//	class StateChangeRC: public RenderCommandOld {
//	public:
//		using StateChangeFunction = std::function<void()>;
//		StateChangeRC(StateChangeFunction p_stateCommand):
//		RenderCommandOld(nullptr)
//		{}
//
//		void Invoke() override;
//
//	private:
//		StateChangeFunction m_stateCommand;
//	};
//
//	class MeshRenderRC: public RenderCommandOld {
//	public:
//		MeshRenderRC() = delete;
//		MeshRenderRC(MeshRenderObject&& p_meshRenderObject, const RenderData* p_renderData):
//			RenderCommandOld(p_renderData),
//			m_meshRenderObject(p_meshRenderObject)
//		{}
//
//		void Invoke() override;
//
//	protected:
//		MeshRenderObject m_meshRenderObject;
//	};
//
//	class InstancedMeshesRC : public RenderCommandOld {
//	public:
//		InstancedMeshesRC() = delete;
//		InstancedMeshesRC(InstancedMeshObject&& p_instancedMeshObject, const RenderData* p_renderData) :
//			RenderCommandOld(p_renderData),
//			m_instancedMeshObject(p_instancedMeshObject)
//		{}
//
//		void Invoke() override;
//
//	protected:
//		InstancedMeshObject m_instancedMeshObject;
//	};
//
//	class CubemapRenderRC: public RenderCommandOld {
//	public:
//		CubemapRenderRC() = delete;
//		CubemapRenderRC(PrRenderer::Resources::MaterialPtr p_cubemapMat, const RenderData* p_renderData):
//			RenderCommandOld(p_renderData),
//			m_cubemapMat(p_cubemapMat)
//		{
//			if(m_cubemapMesh == nullptr)
//				m_cubemapMesh = Resources::Mesh::CreatePrimitive(Resources::Quad);
//		}
//
//		void Invoke() override;
//
//	protected:
//		//Maintain static quad for all cubemaps
//		inline static Resources::MeshPtr m_cubemapMesh = nullptr;
//		Resources::MaterialPtr m_cubemapMat;
//	};
//}