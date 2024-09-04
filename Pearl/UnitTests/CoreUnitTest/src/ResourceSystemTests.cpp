#include <CommonUnitTest/Common/common.h>

#include "Core/Resources/IResourceDataLoader.h"
#include "Core/Resources/IResource.h"
#include "Core/Resources/ResourceDatabase.h"
		 
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
};

class TestResource : public IResource<TestResourceData>
{
public:
	TestResource(const ResourceDescPtr p_resourceDesc) :
		IResource(p_resourceDesc)
	{}
};

class TestLoader : public IResourceDataLoader
{

public:
	IResourceDataPtr LoadResource(const std::string& p_path, size_t& p_dataSize) override
	{
		auto data = std::make_shared<TestResourceData>();
		data->a = 1;
		data->b = 2;
		data->c = 3;

		auto elo = sizeof(TestResourceData);

		p_dataSize = elo;

		return data;
	}


	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}


	bool SaveResourceOnDisc(IResourceDataPtr p_resource, const std::string& p_path) override
	{
		return false;
	}

};

class CustomTestLoader : public IResourceDataLoader
{

public:
	IResourceDataPtr LoadResource(const std::string& p_path, size_t& p_dataSize) override
	{
		auto data = std::make_shared<TestResourceData>();
		data->a = 10;
		data->b = 10;
		data->c = 10;

		auto elo = sizeof(TestResourceData);

		p_dataSize = elo;

		return data;
	}


	void UnloadResource(IResourceDataPtr p_resource) override
	{
		p_resource.reset();
	}


	bool SaveResourceOnDisc(IResourceDataPtr p_resource, const std::string& p_path) override
	{
		return false;
	}

};

TEST(ResourceSystem, Test)
{
	ResourceDatabase dataBase;

	dataBase.SetMemoryBudget(48);

	auto loader = new TestLoader();
	auto customLoader = new CustomTestLoader();
	dataBase.RegisterLoader(".test", loader);

	auto resourceDesc = dataBase.Load("Test.test");
	dataBase.Load("Test1.test");
	dataBase.Load("Test2.test");
	dataBase.Load("Test3.test");
	dataBase.Load("Test4.test", customLoader);


	TestResource resource(resourceDesc);

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
		ids.push_back(p_ptr->m_state);
	};

	dataBase.ForEachResource(visitor);

	auto memoryData = std::make_shared<TestResourceData>();
	memoryData->a = 100;
	memoryData->b = 200;
	memoryData->c = 300;

	auto resource3 = dataBase.Register(memoryData, sizeof(TestResourceData));



	auto dupson = sizeof(ResourceDesc);

	auto resourceMemory = dataBase.Get(resource3->m_ID);
	dataBase.Remove(resource3->m_ID);

	memoryData = nullptr;

	auto metadata1 = dataBase.GetMetadata(resource.GetID());


	dataBase.RemoveAll();
}
