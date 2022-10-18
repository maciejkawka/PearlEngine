#pragma once
#include"Core/Utils/ISerializable.h"

namespace PrCore::ECS {

	class BaseComponent: public Utils::ISerializable {
	public:
		BaseComponent() = default;
	};
}
