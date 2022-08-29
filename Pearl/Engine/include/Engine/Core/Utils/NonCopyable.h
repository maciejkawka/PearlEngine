#pragma once

namespace PrCore::Utils{

	class NonCopyable {
	protected:
		NonCopyable() = default;
		virtual ~NonCopyable() = default;

		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator = (const NonCopyable&) = delete;
	};
}
