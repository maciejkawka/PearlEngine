#include "Core/Common/pearl_pch.h"

#include "Physics/Core/PhysicsSystem.h"
#include "Physics/Actor/IActor.h"
#include "Physics/Impl/ConvertUtils.h"

#include "Core/Utils/Logger.h"

#include "PhysX/PxPhysicsAPI.h"
#include "Physics/Impl/PhysFactory.h"
#include "Physics/Impl/PhysCollisionDispatcher.h"

using namespace PrPhysics;
using namespace physx;

#define PVD_HOST "127.0.0.1"

// Implementation PhysX Variables
static PxDefaultErrorCallback s_defaultErrorCallback;
static PxDefaultAllocator     s_defaultAllocatorCallback;
static PxFoundation*          s_foundation = nullptr;
static PxPhysics*             s_physics = nullptr;
static PxPvd*                 s_pvd = nullptr;

static PxDefaultCpuDispatcher*          s_dispatcher = nullptr;
static PxScene*                         s_scene = nullptr;
static PxSimulationEventCallback*       s_simulationCallback = nullptr;

PhysicsSystem::PhysicsSystem(const PhysicsSettings& p_settings)
{
	s_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocatorCallback, s_defaultErrorCallback);
	if (!s_foundation)
		PRLOG_ERROR("Physics failed to initalize! No fundation created!");

	s_pvd = PxCreatePvd(*s_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	s_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	s_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_foundation, PxTolerancesScale(), true, s_pvd);
	if (!s_physics)
		PRLOG_ERROR("Physics failed to initalize! PxCreatePhysics failed!");

	s_dispatcher = PxDefaultCpuDispatcherCreate(2);
	s_simulationCallback = new CollisionDispatcher();

	PxSceneDesc sceneDesc(s_physics->getTolerancesScale());
	sceneDesc.gravity = ToPxVec3(p_settings.gravity);
	sceneDesc.cpuDispatcher = s_dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.simulationEventCallback = s_simulationCallback;
	sceneDesc.filterShader = ContactReportFilterShader;
	s_scene = s_physics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = s_scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	m_physicsFactory = std::make_unique<PhysFactory>(s_physics);
}

PhysicsSystem::~PhysicsSystem()
{
	m_physicsFactory.reset();

	delete s_simulationCallback;
	PX_RELEASE(s_scene)
	PX_RELEASE(s_dispatcher)
	PX_RELEASE(s_physics);

	if (s_pvd)
	{
		PxPvdTransport* transport = s_pvd->getTransport();
		PX_RELEASE(s_pvd);
		PX_RELEASE(transport);
	}

	PX_RELEASE(s_foundation);
}

void PhysicsSystem::Simulate(float p_dt)
{
	s_scene->simulate(p_dt);
}

void PhysicsSystem::FetchResults()
{
	s_scene->fetchResults(true);
}

bool PhysicsSystem::AddActor(IActorPtr p_actor)
{
	return s_scene->addActor(*static_cast<PxActor*>(p_actor->GetNativePtr()));
}

void PhysicsSystem::RemoveActor(IActorPtr p_actor, bool p_wakeOnLastTouch /*= true*/)
{
	s_scene->removeActor(*static_cast<PxActor*>(p_actor->GetNativePtr()), p_wakeOnLastTouch);
}

void PhysicsSystem::SetGravity(const PrCore::Math::vec3& p_gravity)
{
	s_scene->setGravity(ToPxVec3(p_gravity));
}

PrCore::Math::vec3 PhysicsSystem::GetGravity() const
{
	return ToVec3(s_scene->getGravity());
}

float PhysicsSystem::GetBounceThresholdVelocity() const
{
	return s_scene->getBounceThresholdVelocity();
}

void PhysicsSystem::SetBounceThresholdVelocity(float p_threshold)
{
	s_scene->setBounceThresholdVelocity(p_threshold);
}

void PhysicsSystem::SetFrictionOffsetThreshold(float p_threshold)
{
	s_scene->setFrictionOffsetThreshold(p_threshold);
}

float PhysicsSystem::GetFrictionOffsetThreshold() const
{
	return s_scene->getFrictionOffsetThreshold();
}

PrPhysics::IFactory* PhysicsSystem::GetFactory() const
{
	return m_physicsFactory.get();
}

PrPhysics::IRigidStaticPtr PhysicsSystem::CreateRigidStatic(const Transform& p_transform)
{
	return m_physicsFactory->CreateRigidStatic(p_transform);
}

PrPhysics::IRigidBodyDynamicPtr PhysicsSystem::CreateRigidDynamic(const Transform& p_transform)
{
	return m_physicsFactory->CreateRigidDynamic(p_transform);
}

PrPhysics::IShapePtr PhysicsSystem::CreateShape(const IGeometry& p_geometery, const Material& p_mat, bool isExclusive /*= false*/, ShapeFlags p_flags /*= ShapeFlags::Visualization | ShapeFlags::SceneQuery | ShapeFlags::Simulation*/)
{
	return m_physicsFactory->CreateShape(p_geometery, p_mat, isExclusive, p_flags);
}

PrPhysics::PhysicsStatistics PhysicsSystem::GetStatistics() const
{
	return PhysicsStatistics{};
}
