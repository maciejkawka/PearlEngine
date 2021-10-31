#include"Core/Common/pearl_pch.h"

#include"Core/Filesystem/ConfigFile.h"
#include"Core/Filesystem/FileSystem.h"

using namespace PrCore::Filesystem;

ConfigFile::ConfigFile(const std::string& p_fileName)
{
	Open(p_fileName);
	m_fileName = p_fileName;
}

void ConfigFile::Open(const std::string& p_fileName)
{
	FileStreamPtr file = FileSystem::GetInstance().OpenFileStream("Config/" + p_fileName);
	if (file == nullptr)
		return;

	m_fileName = p_fileName;
	char* data = new char[file->GetSize()];
	file->Read(data);

	std::vector<uint8_t> dataVector;

	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));

	m_jsonFile = json::parse(dataVector);
}

void ConfigFile::Create(const std::string& p_fileName)
{
	FileStreamPtr file = FileSystem::GetInstance().OpenFileStream("Config/" + p_fileName, DataAccess::Write);
	m_fileName = p_fileName;
	std::string dumpJson = m_jsonFile.dump(4);
	int lenght = dumpJson.length();
	file->Write(dumpJson.c_str(), lenght);
}

void ConfigFile::Override()
{
	if (m_fileName.empty())
		return;

	Create(m_fileName);
}
