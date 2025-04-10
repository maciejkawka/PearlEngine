#pragma once

#include "Physics/Shape/IGeometry.h"
#include "Physics/Utils/PhysicsMath.h"
#include "Physics/Utils/INativePtrHandler.h"

namespace PrPhysics {

	enum class ShapeFlags
	{
		Simulation    = 1 << 0,
		SceneQuery    = 1 << 1,
		Trigger       = 1 << 2,
		Visualization = 1 << 3
	};
	DEFINE_ENUM_FLAG_OPERATORS(ShapeFlags);

	class IShape: public INativePtrHandle {
	public:
		virtual void              SetGeometry(const IGeometry& p_geometry) = 0;
		virtual const IGeometry&  GetGeometry() const = 0;

		virtual void              SetLocalPose(const Transform& p_transform) = 0;
		virtual const Transform&  GetLocalPose() const = 0;

		virtual void              SetContactOffset(float p_contactOffset) = 0;
		virtual float             GetContactOffset() const = 0;

		virtual void              SetFlags(ShapeFlags p_flag) = 0;
		virtual ShapeFlags        GetFlags() const = 0;

		virtual void              SetName(const char* p_name) = 0;
		virtual const char*       GetName() const = 0;
	};
	using IShapePtr = std::shared_ptr<IShape>;
}