#pragma once

namespace PrRenderer::Core {

	class Context {
	public:
		Context():
		m_isActive(false)
		{}

		virtual ~Context() = default;

		virtual void Init() = 0;

		inline bool IsActive() const { return m_isActive; }

	protected:
		bool m_isActive;
	};
}
