#pragma once

namespace PrRenderer::Core {

	class Context {
	public:
		virtual ~Context() {}

		virtual void Init() = 0;

		inline bool IsActive() { return m_isActive; }

	protected:
		bool m_isActive;
	};
}
