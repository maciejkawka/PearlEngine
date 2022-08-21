#pragma once
#include"Core/Utils/NonCopyable.h"

#include<cstdint>

namespace PrCore::Utils {

	using UUID = uint64_t;

	class UUIDGenerator : public NonCopyable{
	public:
		UUIDGenerator() = default;
		~UUIDGenerator() = default;

		UUID Generate() const;
		UUID operator()() const;
	};

}