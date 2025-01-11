#pragma once
#include "Core/Utils/Assert.h"

namespace PrCore::Utils {

	template<class T>
	class Singleton {
	public:
		Singleton(Singleton<T>&) = delete;
		Singleton(Singleton<T>&&) = delete;
		Singleton& operator=(const Singleton<T>&) = delete;
		Singleton& operator=(Singleton<T>&&) = delete;

		inline static void Init()
		{
			PR_ASSERT(s_instance == nullptr, "Object " + std::string(typeid(T).name()) + "is already initiated");
			PRLOG_INFO("Init {0}", typeid(T).name());

			s_instance = new T();
		}

		template<typename... Args>
		inline static void Init(Args... p_args)
		{
			PR_ASSERT(s_instance == nullptr, "Object " + std::string(typeid(T).name()) + "is already initiated");
			PRLOG_INFO("Init {0}", typeid(T).name());

			s_instance = new T(p_args...);
		}

		inline static void Terminate()
		{
			PR_ASSERT(s_instance != nullptr, "Object " + std::string(typeid(T).name()) + "is already initiated");
			PRLOG_INFO("Terminate {0}", typeid(T).name());
			delete s_instance;
			s_instance = nullptr;
		}

		inline static T& GetInstance()
		{
			PR_ASSERT(s_instance != nullptr, "Object " + std::string(typeid(T).name()) + "is not initiated");
			return *s_instance;
		}

		inline static T* GetInstancePtr()
		{
			PR_ASSERT(s_instance != nullptr, "Object " + std::string(typeid(T).name()) + "is not initiated");
			return s_instance;
		}

	protected:
		Singleton() = default;
		~Singleton() = default;

		inline static T* s_instance = nullptr;
	};
}