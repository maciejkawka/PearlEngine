#pragma once

#include "Core/Utils/ISerializable.h"

namespace PrCore {

	class BaseComponent: public Utils::ISerializable {
	public:
		BaseComponent() = default;
		virtual ~BaseComponent() = default;
	};
}
