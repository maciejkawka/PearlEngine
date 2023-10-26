#pragma once

namespace PrRenderer::Core
{
	class RenderCommand {
	public:
		RenderCommand() = default;
		virtual ~RenderCommand() = default;

		virtual void Invoke() = 0;
	};

	using RenderCommandPtr = std::shared_ptr<RenderCommand>;

#define BASIC_RENDER_COMMAND(CommandName, FunctionName, ...)             \
	class CommandName : public RenderCommand {                           \
	public:                                                              \
	typedef std::tuple<__VA_ARGS__> CommandArgs;                         \
		explicit CommandName(CommandArgs&& args) :                       \
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
		CommandArgs m_args;                                              \
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
}