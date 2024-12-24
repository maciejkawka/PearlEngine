#include <CommonUnitTest/Common/common.h>

#include "Core/Resources/IResourceDataLoader.h"
#include "Core/Resources/IResource.h"
#include "Core/Resources/ResourceDatabase.h"
#include "Core/Resources/ResourceSystem.h"

#include"Core/Utils/UUID.h"
#include "Core/Utils/Logger.h"
#include "Core/Events/EventManager.h"

using namespace PrCore::Resources;

class TestResource : public IResourceData {
public:
	TestResource() = default;
	~TestResource() = default;

	int a;
	int b;
	int c;

	size_t GetByteSize() const override
	{
		constexpr size_t size = sizeof(TestResource);
		return size;
	}
};
REGISTRER_RESOURCE_HANDLE(TestResource);
using TestResourcePtr = std::shared_ptr<TestResource>;

class TestLoader : public IResourceDataLoader {
public:
	IResourceDataPtr LoadResource(const std::string& p_path) override
	{
		auto data = std::make_shared<TestResource>();

		data->a = 1;
		data->b = 2;
		data->c = 3;

		return data;
	}

	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}

	// Mock Method to avoid saving on disk
	MOCK_METHOD(bool, SaveResourceOnDisc, (IResourceDataPtr, const std::string&), (override));
};

class Test1Loader : public IResourceDataLoader {
public:
	IResourceDataPtr LoadResource(const std::string& p_path) override
	{
		auto data = std::make_shared<TestResource>();

		data->a = 4;
		data->b = 5;
		data->c = 6;

		return data;
	}

	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}

	// Mock Method to avoid saving on disk
	MOCK_METHOD(bool, SaveResourceOnDisc, (IResourceDataPtr, const std::string&), (override));
};

class CustomTestLoader : public IResourceDataLoader {
public:
	IResourceDataPtr LoadResource(const std::string& p_path) override
	{
		auto data = std::make_shared<TestResource>();
		data->a = 10;
		data->b = 20;
		data->c = 30;

		return data;
	}

	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}

	// Mock Method to avoid saving on disk
	MOCK_METHOD(bool, SaveResourceOnDisc, (IResourceDataPtr, const std::string&), (override));
};

class ResourceSystemTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Utils::Logger::Init();
		PrCore::Events::EventManager::Init();
	}

	static void TearDownTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Events::EventManager::Terminate();
		PrCore::Utils::Logger::Terminate();
	}
};

TEST_F(ResourceSystemTest, DatabaseBasicLogic)
{
	ResourceDatabase dataBase;

	dataBase.SetMemoryBudget(5);
	EXPECT_EQ(dataBase.GetMemoryBudget(), 5);

	auto loader = std::make_unique<TestLoader>();
	auto loader1 = std::make_unique<Test1Loader>();
	auto loaderPtr = loader.get();
	auto loader1Ptr = loader1.get();
	dataBase.RegisterLoader(".test", std::move(loader));
	dataBase.RegisterLoader(".test1", std::move(loader1));

	EXPECT_EQ(dataBase.GetLoader(".test"), loaderPtr);
	EXPECT_EQ(dataBase.GetLoader(".test1"), loader1Ptr);

	dataBase.UnregisterLoader(".test");
	EXPECT_EQ(dataBase.GetLoader(".test"), nullptr);

	dataBase.UnregisterAllLoaders();
	EXPECT_EQ(dataBase.GetLoader(".test"), nullptr);
	EXPECT_EQ(dataBase.GetLoader(".test1"), nullptr);
}

TEST_F(ResourceSystemTest, LoadFromFile)
{
	ResourceDatabase dataBase;
	dataBase.SetMemoryBudget(48);

	auto loader = std::make_unique<TestLoader>();
	dataBase.RegisterLoader(".test", std::move(loader));

	auto loader1 = std::make_unique<Test1Loader>();
	dataBase.RegisterLoader(".test1", std::move(loader1));

	// Load the first
	ResourceDescPtr resource = dataBase.Load("Foo.test");
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->size == sizeof(TestResource));
	EXPECT_TRUE(resource->state == ResourceState::Loaded);
	// Check if LoadEvent was fired

	TestResourcePtr resourceData = std::static_pointer_cast<TestResource>(resource->data);
	EXPECT_EQ(resourceData->a, 1);
	EXPECT_EQ(resourceData->b, 2);
	EXPECT_EQ(resourceData->c, 3);

	// Load the second
	ResourceDescPtr resource1 = dataBase.Load("Bar.test1");
	EXPECT_TRUE(resource1->filePath == "bar.test1");
	EXPECT_TRUE(resource1->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource1->size == sizeof(TestResource));
	EXPECT_TRUE(resource1->state == ResourceState::Loaded);
	// Check if LoadEvent was fired

	TestResourcePtr resourceData1 = std::static_pointer_cast<TestResource>(resource1->data);
	EXPECT_EQ(resourceData1->a, 4);
	EXPECT_EQ(resourceData1->b, 5);
	EXPECT_EQ(resourceData1->c, 6);

	// Load with custom loader
	auto customLoader = std::make_shared<CustomTestLoader>();
	ResourceDescPtr resource2 = dataBase.Load("Dar.test", customLoader);
	EXPECT_TRUE(resource2->filePath == "dar.test");
	EXPECT_TRUE(resource2->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource2->size == sizeof(TestResource));
	EXPECT_TRUE(resource2->state == ResourceState::Loaded);
	// Check if LoadEvent was fired

	TestResourcePtr resourceData2 = std::static_pointer_cast<TestResource>(resource2->data);
	EXPECT_EQ(resourceData2->a, 10);
	EXPECT_EQ(resourceData2->b, 20);
	EXPECT_EQ(resourceData2->c, 30);

	EXPECT_EQ(dataBase.GetMemoryUsage(), sizeof(TestResource) * 3);
}

TEST_F(ResourceSystemTest, UnloadFromFile)
{
	ResourceDatabase dataBase;
	auto loader = std::make_unique<TestLoader>();
	dataBase.RegisterLoader(".test", std::move(loader));

	// Load and Unload
	ResourceDescPtr resource = dataBase.Load("Foo.test");
	dataBase.Unload("Foo.test");
	EXPECT_TRUE(resource->data == nullptr);
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->state == ResourceState::Unloaded);
	EXPECT_TRUE(resource->size == 0);
	// Check if UnloadEvent was fired
	
	//Load Again
	dataBase.Load("Foo.test");
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->size == sizeof(TestResource));
	EXPECT_TRUE(resource->state == ResourceState::Loaded);
	EXPECT_TRUE(resource->data != nullptr);
	
	//Unload by ID
	dataBase.Unload(resource->id);
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->size == 0);
	EXPECT_TRUE(resource->state == ResourceState::Unloaded);
	EXPECT_TRUE(resource->data == nullptr);
	// Check if UnloadEvent was fired
	
	// Unload unregistered, should trigger warning
	dataBase.Unload("Unregistered.unregistered");
	// Check if warning triggered
	dataBase.Unload(PrCore::Utils::UUIDGenerator().Generate());
	// Check if warning triggered

	// Load a few resources and unload all
	resource = dataBase.Load("Foo.test");
	ResourceDescPtr resource1 = dataBase.Load("Foo1.test");
	ResourceDescPtr resource2 = dataBase.Load("Foo2.test");
	ResourceDescPtr resource3 = dataBase.Load("Foo3.test");
	// Check if CacheMissEvent triggered
	dataBase.UnloadAll();
	EXPECT_TRUE(resource->data == nullptr);
	EXPECT_TRUE(resource->state == ResourceState::Unloaded);
	EXPECT_TRUE(resource1->data == nullptr);
	EXPECT_TRUE(resource1->state == ResourceState::Unloaded);
	EXPECT_TRUE(resource2->data == nullptr);
	EXPECT_TRUE(resource2->state == ResourceState::Unloaded);
	EXPECT_TRUE(resource3->data == nullptr);
	EXPECT_TRUE(resource3->state == ResourceState::Unloaded);
	// Check if UnloadEvent was fired 4 times

	EXPECT_TRUE(dataBase.GetMemoryUsage() == 0);
}

TEST_F(ResourceSystemTest, RegisterFromFile)
{
	ResourceDatabase dataBase;
	auto loader = std::make_unique<TestLoader>();
	dataBase.RegisterLoader(".test", std::move(loader));

	// Register resource
	auto resource = dataBase.Register("Foo.test");
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->size == 0);
	EXPECT_TRUE(resource->state == ResourceState::Registered);
	EXPECT_TRUE(resource->data == nullptr);

	// Load by ID
	resource = dataBase.Load(resource->id);
	EXPECT_TRUE(resource->state == ResourceState::Loaded);
	EXPECT_TRUE(resource->size == sizeof(TestResource));

	// Unregister without unloading
	dataBase.Remove(resource->id);
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->size == 0);
	EXPECT_TRUE(resource->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource->data == nullptr);
	// Check if UnloadEvent was fired

	// Unregister with unloading first
	resource = dataBase.Register("Foo.test");
	resource = dataBase.Load(resource->id);
	dataBase.Unload(resource->id);
	dataBase.Remove(resource->id);
	EXPECT_TRUE(resource->filePath == "foo.test");
	EXPECT_TRUE(resource->origin == ResourceOrigin::File);
	EXPECT_TRUE(resource->size == 0);
	EXPECT_TRUE(resource->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource->data == nullptr);
	// UnloadEvent should not be fired

	// Register a lot and remove all
	resource = dataBase.Register("Foo.test");
	ResourceDescPtr resource1 = dataBase.Register("Foo1.test");
	ResourceDescPtr resource2 = dataBase.Register("Foo2.test");
	ResourceDescPtr resource3 = dataBase.Register("Foo3.test");
	dataBase.RemoveAll();
	EXPECT_TRUE(resource->data == nullptr);
	EXPECT_TRUE(resource->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource1->data == nullptr);
	EXPECT_TRUE(resource1->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource2->data == nullptr);
	EXPECT_TRUE(resource2->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource3->data == nullptr);
	EXPECT_TRUE(resource3->state == ResourceState::Unmanaged);

	// Register the same, it should return the previously registered spelling should does not matter
	auto sameRes = dataBase.Register("Same.test");
	auto sameRes1 = dataBase.Register("same.test");
	EXPECT_EQ(sameRes->id, sameRes1->id);
	// Check if logged warning
	
	// Unregister not registered 
	bool success = dataBase.Remove(PrCore::Utils::UUIDGenerator().Generate());
	EXPECT_FALSE(success);
	// Check if warning triggered
}

TEST_F(ResourceSystemTest, GetFromFile)
{
	ResourceDatabase dataBase;
	auto loader = std::make_unique<TestLoader>();
	dataBase.RegisterLoader(".test", std::move(loader));

	// Get with loading by ID and path
	auto resource = dataBase.Register("Foo.test");
	dataBase.Load(resource->id);
	auto getResourceId = dataBase.Get(resource->id);
	EXPECT_TRUE(getResourceId->filePath == "foo.test");
	EXPECT_TRUE(getResourceId->origin == ResourceOrigin::File);
	EXPECT_TRUE(getResourceId->size == sizeof(TestResource));
	EXPECT_TRUE(getResourceId->state == ResourceState::Loaded);
	EXPECT_TRUE(getResourceId->data != nullptr);

	auto getResourcePath = dataBase.Get("Foo.test");
	EXPECT_TRUE(getResourcePath->filePath == "foo.test");
	EXPECT_TRUE(getResourcePath->origin == ResourceOrigin::File);
	EXPECT_TRUE(getResourcePath->size == sizeof(TestResource));
	EXPECT_TRUE(getResourcePath->state == ResourceState::Loaded);
	EXPECT_TRUE(getResourcePath->data != nullptr);

	
	// Get without loading by ID and path
	resource = dataBase.Register("Foo.test");
	getResourceId = dataBase.Get(resource->id);
	EXPECT_TRUE(getResourceId->filePath == "foo.test");
	EXPECT_TRUE(getResourceId->origin == ResourceOrigin::File);
	EXPECT_TRUE(getResourceId->size == sizeof(TestResource));
	EXPECT_TRUE(getResourceId->state == ResourceState::Loaded);
	EXPECT_TRUE(getResourceId->data != nullptr);
	// Check if FireCacheMiss event was triggered

	getResourcePath = dataBase.Get("Foo.test");
	EXPECT_TRUE(getResourcePath->filePath == "foo.test");
	EXPECT_TRUE(getResourcePath->origin == ResourceOrigin::File);
	EXPECT_TRUE(getResourcePath->size == sizeof(TestResource));
	EXPECT_TRUE(getResourcePath->state == ResourceState::Loaded);
	EXPECT_TRUE(getResourcePath->data != nullptr);
	// FireCacheMiss should not be triggered


	// Get unregistered, should trigger warning
	auto missingRes = dataBase.Get("Unregistered.unregistered");
	EXPECT_TRUE(missingRes == nullptr);
	// Check if warning triggered
	missingRes = dataBase.Get(PrCore::Utils::UUIDGenerator().Generate());
	EXPECT_TRUE(missingRes == nullptr);
	// Check if warning triggered
}

TEST_F(ResourceSystemTest, ResourcesFromMemory)
{
	ResourceDatabase dataBase;
	auto loader = std::make_unique<TestLoader>();
	dataBase.RegisterLoader(".test", std::move(loader));

	auto resourceData = std::make_shared<TestResource>();
	resourceData->a = 1;
	resourceData->b = 2;
	resourceData->c = 3;

	// Register resource
	auto resource = dataBase.Register(resourceData);
	EXPECT_TRUE(resource->filePath.empty());
	EXPECT_TRUE(resource->origin == ResourceOrigin::Memory);
	EXPECT_TRUE(resource->size == sizeof(TestResource));
	EXPECT_TRUE(resource->state == ResourceState::Registered);
	EXPECT_TRUE(resource->data == resourceData);

	// Try load and unload a resource from memory, it is not possible it returns resourceDesc instead
	auto missingRes = dataBase.Load(resource->id);
	EXPECT_TRUE(missingRes->filePath.empty());
	EXPECT_TRUE(missingRes->origin == ResourceOrigin::Memory);
	EXPECT_TRUE(missingRes->size == sizeof(TestResource));
	EXPECT_TRUE(missingRes->state == ResourceState::Registered);
	EXPECT_TRUE(missingRes->data == resourceData);
	// Check if warning triggered

	dataBase.Unload(resource->id);
	// Check if warning triggered


	// Get registered resource
	auto getResource = dataBase.Get(resource->id);
	EXPECT_TRUE(getResource->filePath.empty());
	EXPECT_TRUE(getResource->origin == ResourceOrigin::Memory);
	EXPECT_TRUE(getResource->size == sizeof(TestResource));
	EXPECT_TRUE(getResource->state == ResourceState::Registered);
	EXPECT_TRUE(getResource->data == resourceData);


	// Unregister resource
	dataBase.Remove(resource->id);
	EXPECT_TRUE(resource->filePath.empty());
	EXPECT_TRUE(resource->origin == ResourceOrigin::Memory);
	EXPECT_TRUE(resource->size == 0);
	EXPECT_TRUE(resource->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource->data == nullptr);
	// Data is still not deleted
	EXPECT_TRUE(resourceData != nullptr);

	// Remove all registered
	resource = dataBase.Register(std::make_shared<TestResource>());
	ResourceDescPtr resource1 = dataBase.Register(std::make_shared<TestResource>());
	ResourceDescPtr resource2 = dataBase.Register(std::make_shared<TestResource>());
	ResourceDescPtr resource3 = dataBase.Register(std::make_shared<TestResource>());
	// Check if CacheMissEvent triggered
	dataBase.RemoveAll();
	EXPECT_TRUE(resource->data == nullptr);
	EXPECT_TRUE(resource->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource1->data == nullptr);
	EXPECT_TRUE(resource1->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource2->data == nullptr);
	EXPECT_TRUE(resource2->state == ResourceState::Unmanaged);
	EXPECT_TRUE(resource3->data == nullptr);
	EXPECT_TRUE(resource3->state == ResourceState::Unmanaged);


	// Save to file
	auto loaderPtr = static_cast<TestLoader*>(dataBase.GetLoader(".test"));
	EXPECT_CALL(*loaderPtr, SaveResourceOnDisc(::testing::_, "save.test")).Times(2).WillRepeatedly(::testing::Return(true));

	auto resourceSave = dataBase.Register(resourceData);
	dataBase.SaveToFile(resourceSave->id, "Save.test");
	EXPECT_TRUE(resourceSave->filePath.empty());
	EXPECT_TRUE(resourceSave->origin == ResourceOrigin::Memory);
	EXPECT_TRUE(resourceSave->size == sizeof(TestResource));
	EXPECT_TRUE(resourceSave->state == ResourceState::Registered);
	EXPECT_TRUE(resourceSave->data == resourceData);

	// Calling with wrong extension will break
	EXPECT_DEATH(dataBase.SaveToFile(resourceSave->id, "save.shouldNotWork"), ".*");

	auto savedAndLoadedResource = dataBase.SaveToFileAndLoad(resourceSave->id, "Save.test");
	EXPECT_TRUE(savedAndLoadedResource->filePath == "save.test");
	EXPECT_TRUE(savedAndLoadedResource->origin == ResourceOrigin::File);
	EXPECT_TRUE(savedAndLoadedResource->size == sizeof(TestResource));
	EXPECT_TRUE(savedAndLoadedResource->state == ResourceState::Loaded);
}

TEST_F(ResourceSystemTest, GeneralResourceSystem)
{
	PrCore::Resources::ResourceSystem::Init();
	auto resourceSystem = PrCore::Resources::ResourceSystem::GetInstancePtr();

	auto dataBase = std::make_unique<ResourceDatabase>();
	auto loader = std::make_unique<TestLoader>();
	auto loaderPtr = loader.get();
	dataBase->RegisterLoader(".test", std::move(loader));

	resourceSystem->RegisterDatabase<TestResource>(std::move(dataBase));

	// FileResource operations
	TestResourceHandle resHandle = resourceSystem->Register<TestResource>("Foo.test");
	EXPECT_TRUE(resHandle.GetPath() == "foo.test");
	EXPECT_TRUE(resHandle.GetOrigin() == ResourceOrigin::File);
	EXPECT_TRUE(resHandle.GetSize() == 0);
	EXPECT_TRUE(resHandle.GetState() == ResourceState::Registered);
	EXPECT_TRUE(resHandle.IsValid());
	EXPECT_TRUE(resHandle == nullptr);

	resourceSystem->Load<TestResource>(resHandle.GetID());
	EXPECT_TRUE(resHandle.GetPath() == "foo.test");
	EXPECT_TRUE(resHandle.GetOrigin() == ResourceOrigin::File);
	EXPECT_TRUE(resHandle.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(resHandle.GetState() == ResourceState::Loaded);
	EXPECT_TRUE(resHandle.IsValid());
	EXPECT_TRUE(resHandle != nullptr);
	EXPECT_TRUE(resHandle->a == 1);
	EXPECT_TRUE(resHandle->b == 2);
	EXPECT_TRUE(resHandle->c == 3);

	resourceSystem->Unload<TestResource>(resHandle.GetID());
	EXPECT_TRUE(resHandle.GetPath() == "foo.test");
	EXPECT_TRUE(resHandle.GetOrigin() == ResourceOrigin::File);
	EXPECT_TRUE(resHandle.GetSize() == 0);
	EXPECT_TRUE(resHandle.GetState() == ResourceState::Unloaded);
	EXPECT_TRUE(resHandle.IsValid());
	EXPECT_TRUE(resHandle == nullptr);

	TestResourceHandle resHandleGet = resourceSystem->Get<TestResource>(resHandle.GetID());
	EXPECT_TRUE(resHandleGet.GetPath() == "foo.test");
	EXPECT_TRUE(resHandleGet.GetOrigin() == ResourceOrigin::File);
	EXPECT_TRUE(resHandleGet.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(resHandleGet.GetState() == ResourceState::Loaded);
	EXPECT_TRUE(resHandleGet.IsValid());
	EXPECT_TRUE(resHandleGet != nullptr);
	EXPECT_TRUE(resHandleGet->a == 1);
	EXPECT_TRUE(resHandleGet->b == 2);
	EXPECT_TRUE(resHandleGet->c == 3);

	resourceSystem->Remove<TestResource>(resHandle.GetID());
	EXPECT_TRUE(resHandle.GetPath() == "foo.test");
	EXPECT_TRUE(resHandle.GetOrigin() == ResourceOrigin::File);
	EXPECT_TRUE(resHandle.GetSize() == 0);
	EXPECT_TRUE(resHandle.GetState() == ResourceState::Unmanaged);
	EXPECT_TRUE(resHandle.IsValid());
	EXPECT_TRUE(resHandle == nullptr);


	// MemoryResource operations
	auto resourceData = std::make_shared<TestResource>();
	resourceData->a = 10;
	resourceData->b = 20;
	resourceData->c = 30;

	TestResourceHandle resMemoryHandle = resourceSystem->Register<TestResource>(resourceData);
	EXPECT_TRUE(resMemoryHandle.GetPath().empty());
	EXPECT_TRUE(resMemoryHandle.GetOrigin() == ResourceOrigin::Memory);
	EXPECT_TRUE(resMemoryHandle.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(resMemoryHandle.GetState() == ResourceState::Registered);
	EXPECT_TRUE(resMemoryHandle.IsValid());
	EXPECT_TRUE(resMemoryHandle != nullptr);
	EXPECT_TRUE(resMemoryHandle->a == 10);
	EXPECT_TRUE(resMemoryHandle->b == 20);
	EXPECT_TRUE(resMemoryHandle->c == 30);

	TestResourceHandle resMemoryHandleLoad = resourceSystem->Load<TestResource>(resMemoryHandle.GetID());
	EXPECT_TRUE(resMemoryHandleLoad.GetPath().empty());
	EXPECT_TRUE(resMemoryHandleLoad.GetOrigin() == ResourceOrigin::Memory);
	EXPECT_TRUE(resMemoryHandleLoad.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(resMemoryHandleLoad.GetState() == ResourceState::Registered);
	EXPECT_TRUE(resMemoryHandleLoad.IsValid());
	EXPECT_TRUE(resMemoryHandle != nullptr);
	EXPECT_TRUE(resMemoryHandleLoad->a == 10);
	EXPECT_TRUE(resMemoryHandleLoad->b == 20);
	EXPECT_TRUE(resMemoryHandleLoad->c == 30);

	TestResourceHandle resMemoryHandleGet = resourceSystem->Get<TestResource>(resMemoryHandle.GetID());
	EXPECT_TRUE(resMemoryHandleGet.GetPath().empty());
	EXPECT_TRUE(resMemoryHandleGet.GetOrigin() == ResourceOrigin::Memory);
	EXPECT_TRUE(resMemoryHandleGet.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(resMemoryHandleGet.GetState() == ResourceState::Registered);
	EXPECT_TRUE(resMemoryHandleGet.IsValid());
	EXPECT_TRUE(resMemoryHandle != nullptr);
	EXPECT_TRUE(resMemoryHandleGet->a == 10);
	EXPECT_TRUE(resMemoryHandleGet->b == 20);
	EXPECT_TRUE(resMemoryHandleGet->c == 30);

	resourceSystem->Unload<TestResource>(resMemoryHandle.GetID());
	resourceSystem->Remove<TestResource>(resMemoryHandle.GetID());
	EXPECT_TRUE(resMemoryHandle.GetPath().empty());
	EXPECT_TRUE(resMemoryHandle.GetOrigin() == ResourceOrigin::Memory);
	EXPECT_TRUE(resMemoryHandle.GetSize() == 0);
	EXPECT_TRUE(resMemoryHandle.GetState() == ResourceState::Unmanaged);
	EXPECT_TRUE(resMemoryHandle.IsValid());
	EXPECT_TRUE(resMemoryHandle == nullptr);	

	
	// Save to file
	EXPECT_CALL(*loaderPtr, SaveResourceOnDisc(::testing::_, "save.test")).Times(2).WillRepeatedly(::testing::Return(true));

	auto resourceSave = resourceSystem->Register<TestResource>(resourceData);
	resourceSystem->SaveToFile<TestResource>(resourceSave.GetID(), "Save.test");
	EXPECT_TRUE(resourceSave.GetPath().empty());
	EXPECT_TRUE(resourceSave.GetOrigin() == ResourceOrigin::Memory);
	EXPECT_TRUE(resourceSave.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(resourceSave.GetState() == ResourceState::Registered);
	EXPECT_TRUE(resourceSave.GetData() == resourceData);

	// Calling with wrong extension will break
	EXPECT_DEATH(resourceSystem->SaveToFile<TestResource>(resourceSave.GetID(), "Save.shouldNotWork"), ".*");

	auto savedAndLoadedResource = resourceSystem->SaveToFileAndLoad<TestResource>(resourceSave.GetID(), "Save.test");
	EXPECT_TRUE(savedAndLoadedResource.GetPath() == "save.test");
	EXPECT_TRUE(savedAndLoadedResource.GetOrigin() == ResourceOrigin::File);
	EXPECT_TRUE(savedAndLoadedResource.GetSize() == sizeof(TestResource));
	EXPECT_TRUE(savedAndLoadedResource.GetState() == ResourceState::Loaded);

	// Copy
	auto resourceToCopy = resourceSystem->Get<TestResource>(savedAndLoadedResource.GetID());
	auto copiedResource = resourceSystem->Copy<TestResource>(resourceToCopy.GetID());
	EXPECT_EQ(copiedResource->a, resourceToCopy->a);
	EXPECT_EQ(copiedResource->b, resourceToCopy->b);
	EXPECT_EQ(copiedResource->c, resourceToCopy->c);

	auto copiedResource1 = resourceSystem->Copy<TestResource>(savedAndLoadedResource.GetPath());
	EXPECT_EQ(copiedResource1->a, resourceToCopy->a);
	EXPECT_EQ(copiedResource1->b, resourceToCopy->b);
	EXPECT_EQ(copiedResource1->c, resourceToCopy->c);

	resourceSystem->RemoveAll<TestResource>();
	
	// Check memory budget
	resourceSystem->SetMemoryBudget<TestResource>(100);
	EXPECT_EQ(resourceSystem->GetMemoryBudget<TestResource>(), 100);
	EXPECT_EQ(resourceSystem->GetMemoryUsage<TestResource>(), 0);

	PrCore::Resources::ResourceSystem::Terminate();
}
