#pragma once
#include "Core/Math/Math.h"

namespace PrRenderer::Core {

	class Frustrum {
	public:
		Frustrum() = default;
		Frustrum(const PrCore::Math::mat4& p_projMat, const PrCore::Math::mat4& p_viewMat);

		PrCore::Math_PrTypes::Plane GetPlane(int p_index) const;

	private:
		//Planes order
		//Right, Left, Bottom, Top, Near, Far
		PrCore::Math_PrTypes::Plane m_planes[6];
	};

	class BoundingVolume {
	public:
		BoundingVolume();
		BoundingVolume(PrCore::Math::vec3 p_center, PrCore::Math::vec3 p_size);
		virtual ~BoundingVolume() {}
		virtual bool IsOnFrustrum(const Frustrum& p_frustrum, PrCore::Math::mat4 p_transform) const = 0;

		const PrCore::Math::vec3& GetCenter() const { return m_center; }
		const PrCore::Math::vec3& GetSize() const { return m_size; }

	protected:
		PrCore::Math::vec3 m_center;
		PrCore::Math::vec3 m_size;
	};

	class SphereVolume: public BoundingVolume {
	public:
		SphereVolume();
		SphereVolume(const std::vector<PrCore::Math::vec3>& p_vertices);
		SphereVolume(const PrCore::Math::vec3& p_center, float p_radius);

		bool IsOnFrustrum(const Frustrum& p_frustrum, PrCore::Math::mat4 p_transform) const override;

		float GetRadius() const { return m_radius; }

	private:
		float m_radius;
	};

	class BoxVolume: public BoundingVolume {
	public:
		BoxVolume();
		BoxVolume(const std::vector<PrCore::Math::vec3>& p_vertices);
		BoxVolume(const PrCore::Math::vec3& p_min, const PrCore::Math::vec3& p_max);

		bool IsOnFrustrum(const Frustrum& p_frustrum, PrCore::Math::mat4 p_transform) const override;

		const PrCore::Math::vec3& GetMin() const { return m_min; }
		const PrCore::Math::vec3& GetMax() const  { return m_max; }
		const PrCore::Math::vec3& GetExtends() const { return m_extends; }

	private:
		PrCore::Math::vec3 m_min;
		PrCore::Math::vec3 m_max;
		PrCore::Math::vec3 m_extends;
	};
}