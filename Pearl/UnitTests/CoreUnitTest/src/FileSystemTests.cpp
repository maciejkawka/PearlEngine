#include <CommonUnitTest/Common/common.h>

#include "Core/File/FileSystem.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Utils/PathUtils.h"

using namespace PrCore::File;

const char* g_unitTestPath = "UnitTests";
const char* g_subUnitTestPath = "UnitTests/SubFolder";

class FileSystemTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Utils::Logger::Init();
		PrCore::File::FileSystem::Init();

		// Create UnitTests files
		auto fileSystem = PrCore::File::FileSystem::GetInstancePtr();
		auto exePath = fileSystem->GetExecutablePath();
		fileSystem->SetWriteDir(exePath);
		fileSystem->MountDir(exePath);
		
		fileSystem->CreateDir(g_unitTestPath);
		fileSystem->CreateDir(g_subUnitTestPath);
		for (int i = 0; i < 10; i++)
		{
			{
				auto path = PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(i));
				FileHandle handle = fileSystem->FileOpen(path, OpenMode::Write);
				const char* text = "Test Text";
				fileSystem->FileWrite(handle, text, strlen(text));
				fileSystem->FileClose(handle);

				EXPECT_TRUE(fileSystem->FileExist(path));
			}

			{
				auto path = PrCore::PathUtils::MakePath(g_subUnitTestPath, PrCore::StringUtils::ToString(i * 10));
				FileHandle handle = fileSystem->FileOpen(path, OpenMode::Write);
				const char* text = "Test Text";
				fileSystem->FileWrite(handle, text, strlen(text));
				fileSystem->FileClose(handle);

				EXPECT_TRUE(fileSystem->FileExist(path));
			}
		}
	}

	static void TearDownTestSuite()
	{
		// Delete UnitTests files
		auto fileSystem = PrCore::File::FileSystem::GetInstancePtr();
		for (int i = 0; i < 10; i++)
		{
			{
				auto path = PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(i));
				fileSystem->FileDelete(path);
				
				EXPECT_FALSE(fileSystem->FileExist(path));
			}

			{
				auto path = PrCore::PathUtils::MakePath(g_subUnitTestPath, PrCore::StringUtils::ToString(i * 10));
				fileSystem->FileDelete(path);
				
				EXPECT_FALSE(fileSystem->FileExist(path));
			}
		}
		fileSystem->DeleteDir(g_subUnitTestPath);
		fileSystem->DeleteDir(g_unitTestPath);

		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::File::FileSystem::Terminate();
		PrCore::Utils::Logger::Terminate();
	}
};

TEST_F(FileSystemTest, FileRead)
{
	auto fileSystem = FileSystem::GetInstancePtr();

	auto handle = fileSystem->FileOpen(PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(0)), OpenMode::Read);
	EXPECT_TRUE(handle);

	int size = fileSystem->FileSize(handle);
	EXPECT_EQ(size, 9);

	bool EoF = fileSystem->FileEOF(handle);
	EXPECT_TRUE(EoF == false);

	char* buffer = new char[size];
	fileSystem->FileRead(handle, buffer, size);
	std::string compStr{ buffer, (size_t)size };
	EXPECT_TRUE(compStr == "Test Text");
	delete[] buffer;

	EoF = fileSystem->FileEOF(handle);
	EXPECT_TRUE(EoF);

	fileSystem->FileSeek(handle, 5);
	int pos = fileSystem->FileTell(handle);
	EXPECT_EQ(pos, 5);

	char symbol;
	fileSystem->FileRead(handle, &symbol, 1);
	EXPECT_EQ(symbol, 'T');

	fileSystem->FileSeek(handle, 20);
	pos = fileSystem->FileTell(handle);
	EXPECT_EQ(pos, 20);

	auto stat = fileSystem->GetStat(PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(0)));
	EXPECT_EQ(stat.fileSize, size);
	EXPECT_FALSE(stat.readOnly);
	EXPECT_EQ(stat.type, FileType::RegularFile);

	fileSystem->FileClose(handle);
}

TEST_F(FileSystemTest, FileWrapper)
{
	auto fileSystem = FileSystem::GetInstancePtr();

	auto file = fileSystem->OpenFileWrapper(PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(0)));
	EXPECT_TRUE(file);

	int size = file->GetSize();
	EXPECT_EQ(size, 9);

	bool EoF = file->IsEof();
	EXPECT_FALSE(EoF);

	char* buffer = new char[size];
	file->Read(buffer, size);
	std::string compStr{ buffer, (size_t)size };
	EXPECT_TRUE(compStr == "Test Text");
	delete[] buffer;

	EoF = file->IsEof();
	EXPECT_TRUE(EoF);

	file->Seek(5);
	int pos = file->Tell();
	EXPECT_EQ(pos, 5);

	char symbol;
	file->Read(&symbol, 1);
	EXPECT_EQ(symbol, 'T');

	file->Seek(20);
	pos = file->Tell();
	EXPECT_EQ(pos, 20);

	auto path = file->GetPath();
	EXPECT_STRCASEEQ(path.data(), PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(0)).data());

	auto stat = file->GetStat();
	EXPECT_EQ(stat.fileSize, size);
	EXPECT_FALSE(stat.readOnly);
	EXPECT_EQ(stat.type, FileType::RegularFile);
}

TEST_F(FileSystemTest, SystemFeatures)
{
	auto fileSystem = FileSystem::GetInstancePtr();

	// Check file
	auto filePath = PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(5));
	auto wrongFilePath = PrCore::PathUtils::MakePath(g_unitTestPath, PrCore::StringUtils::ToString(20));
	{
		bool fileExist = fileSystem->FileExist(filePath);
		EXPECT_TRUE(fileExist);
		fileExist = fileSystem->FileExist(wrongFilePath);
		EXPECT_FALSE(fileExist);

		bool isFile = fileSystem->IsFile(filePath);
		EXPECT_TRUE(isFile);
		bool isDir = fileSystem->IsDir(filePath);
		EXPECT_FALSE(isDir);
	}

	// Check directory
	{
		bool isDir = fileSystem->IsDir(g_unitTestPath);
		EXPECT_TRUE(isDir);
		isDir = fileSystem->IsDir(PrCore::PathUtils::MakePath(g_unitTestPath, ""));
		EXPECT_TRUE(isDir);
		bool isFile = fileSystem->IsFile(g_unitTestPath);
		EXPECT_FALSE(isFile);
	}

	// Enumerate files and directories
	{
		auto files = fileSystem->EnumerateFiles(g_unitTestPath);
		int i = 0;
		for(auto file : files)
		{
			if (fileSystem->IsFile(PrCore::PathUtils::MakePath(g_unitTestPath, file)))
			{
				auto expectedFileName = PrCore::StringUtils::ToString(i);
				EXPECT_TRUE(file == expectedFileName);
				i++;
			}
		}

		files = fileSystem->EnumerateFiles(g_subUnitTestPath);
		i = 0;
		for (auto file : files)
		{
			if (fileSystem->IsFile(PrCore::PathUtils::MakePath(g_subUnitTestPath, file)))
			{
				auto expectedFileName = PrCore::StringUtils::ToString(i * 10);
				EXPECT_TRUE(file == expectedFileName);
				i++;
			}
		}
	}
}

TEST_F(FileSystemTest, BufferedWrite)
{
	auto fileSystem = FileSystem::GetInstancePtr();

	auto filePath = PrCore::PathUtils::MakePath(g_unitTestPath, "BufferedWrite.txt");
	auto writeHandle = fileSystem->FileOpen(filePath, Write);
	auto readHandle = fileSystem->FileOpen(filePath, Read);

	// Write to file only on flush
	{
		fileSystem->FileSetBuffer(writeHandle, 50);
		fileSystem->FileWrite(writeHandle, "FirstLine\n", 10);
		fileSystem->FileWrite(writeHandle, "SecondLine\n", 11);

		char* buffer = new char[50];
		std::memset(buffer, 0, 50);

		fileSystem->FileRead(readHandle, buffer, 21);
		EXPECT_STRCASEEQ(buffer, "");

		fileSystem->FileFlush(writeHandle);

		fileSystem->FileRead(readHandle, buffer, 21);
		EXPECT_STRCASEEQ(buffer, "FirstLine\nSecondLine\n");

		// The second buffered write
		fileSystem->FileWrite(writeHandle, "ThirdLine\n", 10);
		fileSystem->FileWrite(writeHandle, "ForthLine\n", 10);

		fileSystem->FileRead(readHandle, buffer, 21);
		EXPECT_STRCASEEQ(buffer, "FirstLine\nSecondLine\n");

		fileSystem->FileFlush(writeHandle);
		fileSystem->FileSeek(readHandle, 0);
		fileSystem->FileRead(readHandle, buffer, 41);
		EXPECT_STRCASEEQ(buffer, "FirstLine\nSecondLine\nThirdLine\nForthLine\n");
	}

	fileSystem->FileClose(writeHandle);
	fileSystem->FileClose(readHandle);
	fileSystem->FileDelete(filePath);
}
TEST_F(FileSystemTest, MountPriority)
{
	auto fileSystem = FileSystem::GetInstancePtr();
	auto exePath = fileSystem->GetExecutablePath();
	fileSystem->UnmountDir(exePath);

	const char* fileName = "text.txt";
	const char* engine = "EnginePath";
	const char* engineText = "This is Engine Text";
	const char* game = "GamePath";
	const char* gameText = "This is Game Text";

	fileSystem->CreateDir(engine);
	fileSystem->CreateDir(game);

	// Prepare Files
	{
		auto handle = fileSystem->FileOpen(PrCore::PathUtils::MakePath(engine, fileName), Write);
		fileSystem->FileWrite(handle, engineText, 20);
		fileSystem->FileClose(handle);

		handle = fileSystem->FileOpen(PrCore::PathUtils::MakePath(game, fileName), Write);
		fileSystem->FileWrite(handle, gameText, 18);
		fileSystem->FileClose(handle);
	}

	// Mount Engine -> Game
	{
		fileSystem->MountDir(PrCore::PathUtils::MakePath(exePath, engine));
		fileSystem->MountDir(PrCore::PathUtils::MakePath(exePath, game));

		char* buffer = new char[20];
		auto file = fileSystem->OpenFileWrapper(fileName);
		file->Read(buffer, 20);
		file.reset();

		auto comapreStr = std::string{ buffer, 20 };
		EXPECT_STRCASEEQ(comapreStr.data(), engineText);

		fileSystem->UnmountDir(PrCore::PathUtils::MakePath(exePath, engine));
		fileSystem->UnmountDir(PrCore::PathUtils::MakePath(exePath, game));
		delete[] buffer;
	}

	// Mount Game -> Engine
	{
		fileSystem->MountDir(PrCore::PathUtils::MakePath(exePath, game));
		fileSystem->MountDir(PrCore::PathUtils::MakePath(exePath, engine));

		char* buffer = new char[20];
		auto file = fileSystem->OpenFileWrapper(fileName);
		file->Read(buffer, 20);
		file.reset();

		auto comapreStr = std::string{ buffer, 20 };
		EXPECT_STRCASEEQ(comapreStr.data(), gameText);

		fileSystem->UnmountDir(PrCore::PathUtils::MakePath(exePath, engine));
		fileSystem->UnmountDir(PrCore::PathUtils::MakePath(exePath, game));
		delete[] buffer;
	}

	// Mount Game Only
	{
		fileSystem->MountDir(PrCore::PathUtils::MakePath(exePath, game));

		char* buffer = new char[20];
		auto file = fileSystem->OpenFileWrapper(fileName);
		file->Read(buffer, 20);
		file.reset();

		auto comapreStr = std::string{ buffer, 20 };
		EXPECT_STRCASEEQ(comapreStr.data(), gameText);

		fileSystem->UnmountDir(PrCore::PathUtils::MakePath(exePath, game));
		delete[] buffer;
	}

	fileSystem->MountDir(exePath);

	fileSystem->FileDelete(PrCore::PathUtils::MakePath(engine, fileName));
	fileSystem->FileDelete(PrCore::PathUtils::MakePath(game, fileName));
	fileSystem->DeleteDir(engine);
	fileSystem->DeleteDir(game);
}


// For later when archive is supported
TEST_F(FileSystemTest, OpenArchive)
{
}