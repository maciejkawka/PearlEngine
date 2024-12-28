#include"Core/Common/pearl_pch.h"

#include"Core/Filesystem/ConfigFile.h"
#include"Core/Filesystem/FileSystemNew.h"

using namespace PrCore::File;

bool ConfigFile::OpenFromFile(std::string_view p_filePath)
{
	auto file = File::FileSystemNew::GetInstance().OpenFileWrapper(p_filePath);
	if (file == nullptr)
		return false;

	m_filePath = p_filePath;
	char* data = new char[file->GetSize()];
	file->Read(data, file->GetSize());

	std::vector<uint8_t> dataVector;
	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	m_jsonFile = json::parse(dataVector);
	return true;
}

bool ConfigFile::SaveToFile(std::string_view p_filePath)
{
	auto file = File::FileSystemNew::GetInstance().FileOpen(p_filePath, File::OpenMode::Write);
	if (file == nullptr)
		return false;

	m_jsonFile = p_filePath;
	std::string dumpJson = m_jsonFile.dump(4);
	int lenght = dumpJson.length();
	File::FileSystemNew::GetInstance().FileWrite(file, dumpJson.c_str(), lenght);
	File::FileSystemNew::GetInstance().FileClose(file);
	return true;
}
