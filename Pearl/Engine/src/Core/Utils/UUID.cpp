#include"Core/Common/pearl_pch.h"

#include<random>

#include"Core/Utils/UUID.h"

using namespace PrCore::Utils;


static std::random_device s_randomDevice;
static std::mt19937_64 s_mt(s_randomDevice());

PrCore::Utils::UUID UUIDGenerator::Generate() const
{
	return s_mt();
}

PrCore::Utils::UUID UUIDGenerator::operator()() const
{
	return Generate();
}
