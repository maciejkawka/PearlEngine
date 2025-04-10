#pragma once

#include "Core/Utils/Singleton.h"
#include "Physics/Core/IFactory.h"

namespace PrPhysics {

	// Struct with initial system settings
	struct PhysicsSettings
	{
		PrCore::Math::vec3 gravity = { 0.0f, -9.81f, 0.0f };
	};

	struct PhysicsStatistics
	{
		size_t staticBodies;
		size_t dynamicBodies;
		size_t kinematicBodies;

		size_t activeConstrains;
		size_t activeDynamicBodies;
		size_t activeStaticBodies;
		size_t activeKinematicBodies;
	};

	class PhysicsSystem : public PrCore::Utils::Singleton<PhysicsSystem> {
	public:
		PhysicsSystem(const PhysicsSettings& p_settings);
		~PhysicsSystem();

		// Scene Manipulations
		bool AddActor(IActorPtr p_actor);
		void RemoveActor(IActorPtr p_actor, bool p_wakeOnLastTouch = true);

		void                 SetGravity(const PrCore::Math::vec3& p_gravity);
		 PrCore::Math::vec3  GetGravity() const;

		void  SetBounceThresholdVelocity(float p_threshold);
		float GetBounceThresholdVelocity() const;

		void  SetFrictionOffsetThreshold(float p_threshold);
		float GetFrictionOffsetThreshold() const;

		// Simulation
		PhysicsStatistics GetStatistics() const { return PhysicsStatistics{}; }

		void Simulate(float p_dt);
		void FetchResults();

		// Factory
		IFactory* GetFactory() const;

		IRigidStaticPtr      CreateRigidStatic(const Transform& p_transform);
		IRigidBodyDynamicPtr CreateRigidDynamic(const Transform& p_transform);
		IShapePtr            CreateShape(const IGeometry& p_geometery, const Material& p_mat, bool isExclusive = false, ShapeFlags p_flags = ShapeFlags::Visualization | ShapeFlags::SceneQuery | ShapeFlags::Simulation);

		// Queries

	private:
		std::unique_ptr<IFactory> m_physicsFactory;
	};
}
