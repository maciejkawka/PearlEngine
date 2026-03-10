#pragma once

namespace PrCore {

	class IGameFramework {
	public:
		virtual void OnInitalize() = 0;
		virtual bool OnUpdate(float p_dt) = 0;
		virtual void OnTerminate() = 0;
	};
}