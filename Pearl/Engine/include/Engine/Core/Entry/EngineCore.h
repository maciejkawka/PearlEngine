#pragma once

#include "IContext.h"
#include "GameFramework.h"

namespace PrCore {

	class Event;
	typedef std::shared_ptr<Event> EventPtr;

	class EngineCore {
	public:
		EngineCore(std::unique_ptr<IContext>&& p_context);

		bool Initalize(const EngineCoreParams& p_params);
		bool Run();
		void Terminate();

	private:
		std::unique_ptr<IContext>  m_context;
		IGameFramework*            m_game;

		const float m_physicsFixStep = 1.0f / 60.0f;
		float       m_physicsStepAccumulator;
	};
	using EngineCorePtr = std::unique_ptr<EngineCore>;

	// Use to create a Core
	EngineCorePtr CreateEngineCore(std::unique_ptr<IContext>&& p_context);
}