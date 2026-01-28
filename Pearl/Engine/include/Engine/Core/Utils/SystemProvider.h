#pragma once

#include "ISystem.h"
#include "Assert.h"

#include <tuple>
#include <array>

namespace PrCore
{
	namespace Utils
	{
		class Clock;
		class ILogger;
	}

	class EventManager;
	class FileSystem;
	class JobSystem;
	class ThreadSystem;
	class ResourceSystem;
	class IWindow;
	class InputManager;
}

namespace PrPhysics
{
	class PhysicsSystem;
}

namespace PrRenderer
{
	class IRenderFrontend;
}

// Tuple with all systems used in the engine,
// extend this list when new system is added.
// IMPORTANT!
// If system is not going to be replaced or mocked in unit tests just use the concrete implementation in here
// There is no need to provide the interface if it is not necessery
using EngineSystems = std::tuple<
	PrCore::EventManager,
	PrCore::FileSystem,
	PrCore::IWindow,
	PrCore::InputManager,
	PrCore::JobSystem,
	PrCore::ThreadSystem,
	PrCore::ResourceSystem,
	PrCore::Utils::Clock,
	PrCore::Utils::ILogger,
	PrRenderer::IRenderFrontend,
	PrPhysics::PhysicsSystem
>;

// PrSystems is in global namespace for an easy usage. 
// It PrSystemsrom Register call untill Unregister call
// To get the system across the engine call PrSystems::Get<T>()
class PrSystems {
private:
	template <class T, class Tuple>
	struct SystemId;

	template <class T, class... Types>
	struct SystemId<T, std::tuple<T, Types...>> {
		static const std::size_t value = 0;
	};

	template <class T, class U, class... Types>
	struct SystemId<T, std::tuple<U, Types...>> {
		static_assert(SystemId<T, std::tuple<Types...>>::value < std::tuple_size_v<EngineSystems>, "System does not exist!");
		static const std::size_t value = 1 + SystemId<T, std::tuple<Types...>>::value;
	};

public:
	// Initalizes and register system that does not have abstracted interface
	template<typename Impl, typename... Args>
	static Impl* Register(Args... p_args)
	{
		PR_ASSERT((!m_systems[SystemId<Impl, EngineSystems>::value]), "System already registered!");

		Impl* instance = new Impl(p_args...);
		m_systems[SystemId<Impl, EngineSystems>::value] = instance;
		return instance;
	}

	// Initalizes and register system with abstracted interface
	template<typename Interface, typename Impl, typename... Args>
	static Interface* Register(Args... p_args)
	{
		static_assert(std::is_base_of_v<Interface, Impl>, "System implementation does not extend the interface!");
		PR_ASSERT((!m_systems[SystemId<Interface, EngineSystems>::value]), "System already registered!");

		Interface* instance = new Impl(p_args...);
		m_systems[SystemId<Interface, EngineSystems>::value] = instance;
		return instance;
	}

	template<typename Interface>
	static void Unregister()
	{
		auto pSystem = m_systems[SystemId<Interface, EngineSystems>::value];
		PR_ASSERT(pSystem, "System is not registered!");

		if (pSystem)
		{
			m_systems[SystemId<Interface, EngineSystems>::value] = nullptr;
			delete pSystem;
		}
	}

	template<typename Interface>
	static constexpr Interface* Get()
	{
		return static_cast<Interface*>(m_systems[SystemId<Interface, EngineSystems>::value]);
	}

private:
	inline static std::array<PrCore::Utils::ISystem*, std::tuple_size_v<EngineSystems>> m_systems{};
};
