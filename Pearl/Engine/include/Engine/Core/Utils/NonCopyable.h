#pragma once

namespace PrCore::Utils{

	class NonCopyable {
	protected:
		NonCopyable() = default;
		~NonCopyable() = default;

		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator = (const NonCopyable&) = delete;
	};
}
