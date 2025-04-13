#pragma once

#include "Physics/Shape/IShape.h"
#include "Physics/Shape/IGeometry.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	class PhysShape : public IShape {
	public:
		PhysShape(physx::PxShape* p_shape);
		~PhysShape();

		void             SetGeometry(const IGeometry& p_geometry) override;
		const IGeometry& GetGeometry() const override;

		void             SetLocalPose(const Transform& p_transform) override;
		const Transform& GetLocalPose() const override;

		void  SetContactOffset(float p_contactOffset) override;
		float GetContactOffset() const override;

		void       SetFlags(ShapeFlags p_flag) override;
		ShapeFlags GetFlags() const override;

		void        SetName(const char* p_name) override;
		const char* GetName() const override;

		void*  GetNativePtr() override;
		void   ReleaseNativePtr() override;

	private:
		physx::PxShape* m_impl;
	};

}