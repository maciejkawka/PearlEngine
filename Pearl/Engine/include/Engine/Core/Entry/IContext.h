#pragma once

#include <string>

namespace PrCore {

	struct EngineCoreParams
	{
	};

	class IContext {
	public:
		virtual ~IContext() = default;

		virtual bool OnInitalize(const EngineCoreParams& p_params) = 0;
		virtual void OnTerminate() = 0;

		// Add more later like on UI PostUpdate etc.
		virtual bool OnUpdate(float p_dt) = 0;

		virtual bool ShouldClose() = 0;
	};
}
