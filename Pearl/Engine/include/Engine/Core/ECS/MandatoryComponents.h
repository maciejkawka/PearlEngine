#pragma once
#include"Core/Utils/UUID.h"
#include"Core/ECS/BaseComponent.h"
#include <string>

namespace PrCore::ECS {

	class UUIDComponent: BaseComponent {
	public:
		Utils::UUID UUID;
	};

	class NameComponent : BaseComponent {
	public:
		std::string name;
	};

	class TagComponent : BaseComponent {
	public:
		std::string tag;
	};
}
