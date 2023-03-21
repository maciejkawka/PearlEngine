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

namespace PrCore {
	namespace Math_PrTypes {
		class Plane {
		public:
			Plane() :
				m_normal(glm::vec3(0.0f, 1.0f, 0.0f)),
				m_distance(0.0f)
			{}

			Plane(const glm::vec3& p_normal, float p_distance, bool p_normalise)
			{
				if (p_normalise)
				{
					float length = PrCore::Math::sqrt(PrCore::Math::dot(p_normal, p_normal));
					m_normal = p_normal / length;
					m_distance = p_distance / length;
				}
				else
				{
					m_normal = p_normal;
					m_distance = p_distance;
				}
			}

			void SetNormal(const glm::vec3& p_normal) { m_normal = p_normal; }
			void SetDistance(float p_distance) { m_distance = p_distance; }

			glm::vec3 GetNormal() { return m_normal; }
			float GetDistance() { return m_distance; }

		private:
			glm::vec3 m_normal;
			float m_distance;
		};
	}
}



///Matrix Layout
///
/// 00  10  20  30
///	01  11  21  31
///	02  12  22  32
///	03  13  23  33