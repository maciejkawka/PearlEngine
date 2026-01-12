#include"Core/Common/pearl_pch.h"

#include"Core/File/FileSystem.h"
#include"Core/File/ConfigFile.h"

using namespace PrCore;

bool ConfigFile::OpenFromFile(std::string_view p_filePath)
{
	auto file = PrSystems::Get<FileSystem>()->OpenFileWrapper(p_filePath);
	if (file == nullptr)
		return false;

	m_filePath = p_filePath;
	char* data = new char[file->GetSize()];
	file->Read(data, file->GetSize());

	std::vector<uint8_t> dataVector;
	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	m_jsonFile = Utils::JSON::json::parse(dataVector);
	return true;
}

bool ConfigFile::SaveToFile(std::string_view p_filePath)
{
	auto file = PrSystems::Get<FileSystem>()->FileOpen(p_filePath, FileOpenMode::Write);
	if (file == nullptr)
		return false;

	m_jsonFile = p_filePath;
	std::string dumpJson = m_jsonFile.dump(4);
	int lenght = dumpJson.length();
	PrSystems::Get<FileSystem>()->FileWrite(file, dumpJson.c_str(), lenght);
	PrSystems::Get<FileSystem>()->FileClose(file);
	return true;
}
