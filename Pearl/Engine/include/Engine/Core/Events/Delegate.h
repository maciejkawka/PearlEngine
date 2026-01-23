#pragma once

#include "Core/Utils/Assert.h"

#include <functional>

namespace PrCore {

	template<typename T>
	class Delegate;

	template<typename Ret, typename... Args>
	class Delegate<Ret(Args...)> {
	public:
		Delegate() = default;

		template<auto Func, typename T>
		void Connect(T* p_obj)
		{
			m_object = p_obj;
			m_func = [p_obj](Args... p_args)
			{
				return (p_obj->*Func)(std::forward<Args>(p_args)...);
			};
		}

		Ret operator()(Args... p_args) const
		{
			return Invoke(std::forward<Args>(p_args)...);
		}

		Ret Invoke(Args... p_args) const
		{
			PR_ASSERT(m_func, "Delegate function is nullptr!");
			return m_func(p_args...);
		}

		bool operator==(const Delegate& p_other) const
		{
			return m_object == p_other.m_object &&
				p_other.m_func && m_func &&
				p_other.m_func.target<Ret(Args...)>() == m_func.target<Ret(Args...)>();
		}

		std::function<Ret(Args...)> m_func{ nullptr };
		void*                       m_object{ nullptr };
	};
}