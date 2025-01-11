namespace PrCore::Threading {

	template<typename Func, typename... Args>
	JobStatePtr JobSystem::Schedule(std::string_view p_name, Func&& p_function, Args&&... p_args)
	{
		static_assert(std::is_pointer_v<Func> && std::is_function_v<std::remove_pointer_t<Func>>
			|| std::is_invocable_v<Func, Args...>
			|| std::is_member_function_pointer_v<Func>, "Func has to be a function pointer, member function pointer or lambda");

		auto jobState = std::make_shared<JobState>();
		JobDesc jobDesc;
		jobDesc.id = m_nextJobId++;
		jobDesc.name = p_name;
		jobDesc.functionPtr = std::bind(p_function, std::forward<Args>(p_args)...);
		jobDesc.state = jobState;

		// acquire active queue and increment the value
		size_t activeWorker = 0;
		size_t nextActiveWorker = 0;
		do {
			activeWorker = m_activeWorker.load();
			nextActiveWorker = (activeWorker + 1) % m_workers.size();

		} while (!m_activeWorker.compare_exchange_weak(activeWorker, nextActiveWorker));

		m_workers[activeWorker]->AddJobRequest(std::move(jobDesc));
		m_workers[activeWorker]->Notify();

		return jobState;
	}
}