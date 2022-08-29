#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Defines.h"

using namespace PrCore::ECS;

ID::ID() : m_ID(0)
#ifdef _DEBUG
, DEBUG_INDEX(0),
DEBUG_VERSION(0)
#endif
{}

ID::ID(uint64_t p_ID) : m_ID(p_ID)
#ifdef _DEBUG
, DEBUG_INDEX(p_ID >> 32),
DEBUG_VERSION(uint32_t(p_ID))
#endif
{}

ID::ID(uint32_t p_index, uint32_t p_version) : m_ID(uint64_t(p_index) << 32 | uint64_t(p_version))
#ifdef _DEBUG
, DEBUG_INDEX(p_index),
DEBUG_VERSION(p_version)
#endif
{}