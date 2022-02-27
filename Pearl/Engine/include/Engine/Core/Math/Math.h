#pragma once
#define GLM_FORCE_AVX2
#define GLM_FORCE_CXX17
#define GLM_FORCE_INLINE
#include"glm/glm.hpp"
#include"glm/gtc/quaternion.hpp"
#include"glm/gtx/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace PrCore {
	namespace Math = glm;
}