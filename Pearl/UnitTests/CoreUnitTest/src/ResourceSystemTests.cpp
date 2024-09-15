#include <CommonUnitTest/Common/common.h>

#include "Core/Resources/IResourceDataLoader.h"
#include "Core/Resources/IResource.h"
#include "Core/Resources/ResourceDatabase.h"
#include "Core/Resources/ResourceSystem.h"

#include "Core/Filesystem/ConfigFile.h"
#include "Core/Filesystem/FileSystem.h"

#include "Core/Utils/Logger.h"
		 
using namespace PrCore::Resources;

class TestResourceData : public IResourceData
{
public:
	TestResourceData() = default;
	
	~TestResourceData()
	{
			
	}

	int a;
	int b;
	int c;

	size_t GetByteSize() const override
	{
		constexpr size_t size = sizeof(TestResourceData);
		return size;
	}

};
REGISTRER_RESOURCE_HANDLE(TestResourceData);

class TestLoader : public IResourceDataLoader
{

public:
	IResourceDataPtr LoadResource(const std::string& p_path) override
	{
		auto data = std::make_shared<TestResourceData>();

		data->a = 1;
		data->b = 2;
		data->c = 3;

		return data;
	}


	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}


	bool SaveResourceOnDisc(IResourceDataPtr p_resource, const std::string& p_path) override
	{
		PrCore::Filesystem::ConfigFile file(p_path);
		
		if (!file.IsValid())
			return false;
		auto testResource = std::static_pointer_cast<TestResourceData>(p_resource);
		file.SetSetting("a", testResource->a);
		file.SetSetting("b", testResource->b);
		file.SetSetting("c", testResource->c);

		file.Override();
		return true;
	}
};

class CustomTestLoader : public IResourceDataLoader
{

public:
	IResourceDataPtr LoadResource(const std::string& p_path) override
	{
		auto data = std::make_shared<TestResourceData>();
		data->a = 10;
		data->b = 10;
		data->c = 10;

		return data;
	}


	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}


	bool SaveResourceOnDisc(IResourceDataPtr p_resource, const std::string& p_path) override
	{
		PrCore::Filesystem::ConfigFile file(p_path);

		if (!file.IsValid())
			return false;
		auto testResource = std::static_pointer_cast<TestResourceData>(p_resource);
		file.SetSetting("a", testResource->a);
		file.SetSetting("b", testResource->b);
		file.SetSetting("c", testResource->c);

		file.Override();
		return true;
	}

};

class CustomTestLoader1 : public IResourceDataLoader
{
public:
	IResourceDataPtr LoadResource(const std::string& p_path) override
	{
		auto data = std::make_shared<TestResourceData>();
		data->a = 60;
		data->b = 60;
		data->c = 60;

		return data;
	}


	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}


	bool SaveResourceOnDisc(IResourceDataPtr p_resource, const std::string& p_path) override
	{
		PrCore::Filesystem::ConfigFile file(p_path);

		if (!file.IsValid())
			return false;
		auto testResource = std::static_pointer_cast<TestResourceData>(p_resource);
		file.SetSetting("a", testResource->a);
		file.SetSetting("b", testResource->b);
		file.SetSetting("c", testResource->c);

		file.Override();
		return true;
	}

};

TEST(ResourceSystem, Test)
{
	PrCore::Utils::Logger::Init();
	PrCore::Filesystem::FileSystem::Init();

	ResourceDatabase dataBase;

	dataBase.SetMemoryBudget(48);

	auto loader = std::make_unique<TestLoader>();
	auto customLoader = new CustomTestLoader();
	dataBase.RegisterLoader(".test", std::move(loader));

	auto resourceDesc = dataBase.Load("Test.test");
	dataBase.Load("Test1.test");
	dataBase.Load("Test2.test");
	dataBase.Load("Test3.test");

	auto dupkon = std::make_shared<CustomTestLoader>();
	auto dupkon1 = std::make_shared<CustomTestLoader1>();

	dataBase.Load("Test4.test", dupkon);
	dataBase.Load("Test4.test", dupkon1);

	dataBase.Unload("Test4.test");
	dataBase.Load("Test4.test", dupkon1);
	dataBase.Unload("Test4.test");
	dataBase.Load("Test4.test");

	TestResourceDataHandle resource(resourceDesc);

	resource.GetData()->a;
	resource.GetData()->b;
	resource.GetData()->c;

	dataBase.Unload(resource.GetID());
	dataBase.UnloadAll();

	auto test1 = dataBase.Get(resource.GetID());

	auto test2 = dataBase.Get(resource.GetPath());

	std::vector<ResourceState> ids;
	auto visitor = [&](ResourceDescConstPtr p_ptr)
	{
		ids.push_back(p_ptr->state);
	};

	dataBase.ForEachResource(visitor);

	auto memoryData = std::make_shared<TestResourceData>();
	memoryData->a = 100;
	memoryData->b = 200;
	memoryData->c = 300;

	auto resource3 = dataBase.Register(memoryData);

	auto sucess = dataBase.SaveToFile(resource3->id, "resource.test");
	auto resourceSaved = dataBase.SaveToFileAndLoad(resource3->id, "resource.test");

	auto dupson = sizeof(ResourceDesc);

	auto resourceMemory = dataBase.Get(resource3->id);
	dataBase.Remove(resource3->id);

	memoryData = nullptr;

	auto metadata1 = dataBase.GetMetadata(resource.GetID());

	ResourceSystem s;

	auto databasePtr = std::make_unique<ResourceDatabase>();
	databasePtr->RegisterLoader(".test", std::make_unique<CustomTestLoader>());


	s.RegisterDatabase<TestResourceData>(std::move(databasePtr));
	auto resourceDescELO = s.Load<TestResourceData>("Test10.test");
	s.Load<TestResourceData>("Test10.test");
	s.Unload<TestResourceData>(resourceDescELO.GetID());

	auto copy = s.Copy<TestResourceData>(resourceDescELO.GetID());
	copy->a = -20;

	TestLoader elo{};
	elo.SaveResourceOnDisc(copy, "dupson.TXT");

	auto registered = s.Register<TestResourceData>(copy);
	TestResourceDataHandle testrestouesss = s.Get<TestResourceData>(registered.GetID());
	
	copy = nullptr;
	std::shared_ptr<TestResourceData> data = registered.GetData();
	s.SaveToFile<TestResourceData>(registered.GetID(), "Test20.test");
	s.Remove<TestResourceData>(registered.GetID());
	data = nullptr;
	auto resourceKalafior = resourceDescELO.GetData();
	
	dataBase.RemoveAll();
	dataBase.UnregisterAllLoaders();

	//s.UnregisterDatabase<TestResourceData>();
	s.UnregisterDatabasesAll();

	auto siezeDupsko =sizeof(TestResourceDataHandle);
	siezeDupsko =sizeof(ResourceDescPtr);
}
