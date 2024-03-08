#pragma once
#include"FileStream.h"
#include"StandardFileStream.h"
#include"Core/Utils/Singleton.h"

#include<vector>

namespace PrCore::Filesystem {
	
//#define ROOT_DIR "C:/Users/Maciej/Current Projects/GitHub/PearlEngine/PearlEngine/Resources"
#define RESOURCES_DIR "Resources"
#define CONFIG_DIR "Config"
#define SHADER_DIR "Shader"
#define TEXTURE_DIR "Texture"
#define MATERIAL_DIR "Material"
#define MESH_DIR "Mesh"
#define SCENE_DIR "Scene"

	//TODO Implement Commented Functions
	class FileSystem: public Utils::Singleton<FileSystem> {
	public:
		PrFile* OpenFile(const std::string& p_name, DataAccess p_access = DataAccess::Read);
		//PrFile* OpenFileDirect(const std::string& p_name, DataAccess p_access = DataAccess::Read);
		FileStreamPtr OpenFileStream(const std::string& p_name, DataAccess p_access = DataAccess::Read);
		//FileStreamPtr OpenFileStreamDirect(const std::string& p_name, DataAccess p_access = DataAccess::Read);

		void CloseFile(PrFile* p_file);

		//StandardFileStreamPtr OpenFileStandard(const std::string& p_name, DataAccess p_access = DataAccess::Read);

		//StandardFileStreamPtr ToStandardFileStream(const FileStreamPtr& p_fileStream);
		//FileStreamPtr ToFileStream(const StandardFileStreamPtr& p_fileStream);

		void DeleteDir(const std::string& p_name);
		void DeleteFile(const std::string& p_name);

		void CreateDir(const std::string& p_name);

		unsigned int GetFileCount(const std::string& p_dir);
		std::vector<std::string> GetFileList(const std::string& p_dir);

		std::string GetResourcesPath() const { return m_resourcesPath; }
		void SetResourcesPath(const std::string& p_root) { m_resourcesPath = p_root; }

	private:
		FileSystem();
		~FileSystem();

		void InitDir();
		void DeduceResourcesPath();

		std::string m_resourcesPath;

		friend Singleton<FileSystem>;
	};
}