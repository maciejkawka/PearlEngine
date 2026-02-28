#pragma once

#include <vector>
#include <string_view>

#include "Core/Resources/IResource.h"

namespace PrAudio {

	class ISoundBank : public PrCore::IResourceData {
	public:
		virtual ~ISoundBank() = default;

		virtual std::string_view GetPath() = 0;

		virtual void PreloadAll() = 0;
		virtual void UnloadAll() = 0;

		virtual int GetEventCount() = 0;
		virtual int GetBusCount() = 0;

		virtual const std::vector<std::string>& GetEventList() const = 0;
		virtual const std::vector<std::string>& GetBusList() const = 0;
	};
	REGISTRER_RESOURCE_HANDLE(ISoundBank);

	using ISoundBankPtr = std::shared_ptr<ISoundBank>;
}