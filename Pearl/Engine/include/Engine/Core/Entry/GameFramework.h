#pragma once

namespace PrCore {

	class IGameFramework {
	public:
		virtual void OnInitalize() = 0;
		virtual bool OnUpdate(float p_dt) = 0;
		virtual void OnTerminate() = 0;
	};
}

// To be defined in the game static lib. 
// Can return just nullptr if game lib is not required
extern  PrCore::IGameFramework*  CreateGameFramework();