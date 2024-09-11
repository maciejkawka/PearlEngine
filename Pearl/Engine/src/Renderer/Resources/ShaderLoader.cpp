#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/ShaderLoader.h"
#include "Renderer/Resources/Shaderv2.h"

#include"Core/Filesystem/FileSystem.h"
#include"Core/Utils/StringUtils.h"
#include"Core/Utils/PathUtils.h"

using namespace PrRenderer::Resources;

PrCore::Resources::IResourceDataPtr ShaderLoader::LoadResource(const std::string& p_path)
{
	//Load from file
	std::string resourcePath = PrCore::PathUtils::MakePath(SHADER_DIR, p_path);
	PrCore::Filesystem::FileStreamPtr file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(resourcePath.c_str());
	if (file == nullptr)
		return nullptr;

	std::string shader;
	shader.resize(file->GetSize());
	file->Read(&shader[0]);

	std::string vertexShaderKeyword = "#vertex";
	std::string fragmentShaderKeyword = "#fragment";
	std::string geometryShaderKeyword = "#geometry";

	// check if geometry shader is available and create shader
	Shaderv2Ptr shaderPtr = nullptr;
	const auto fragmentShaderPos = shader.find(fragmentShaderKeyword);
	const auto geometryShaderPos = shader.find(geometryShaderKeyword);
	if (geometryShaderPos != std::string::npos)
	{
		auto vertexShader = shader.substr(vertexShaderKeyword.length(), geometryShaderPos - vertexShaderKeyword.length());
		auto geometryShader = shader.substr(geometryShaderPos + geometryShaderKeyword.length(), fragmentShaderPos - geometryShaderPos - geometryShaderKeyword.length());
		auto fragmentShader = shader.substr(fragmentShaderPos + fragmentShaderKeyword.length());
		
		shaderPtr = Shaderv2::Create(vertexShader, fragmentShader, geometryShader);
	}
	else
	{
		auto vertexShader = shader.substr(vertexShaderKeyword.length(), fragmentShaderPos - vertexShaderKeyword.length());
		auto fragmentShader = shader.substr(fragmentShaderPos + fragmentShaderKeyword.length());

		shaderPtr = Shaderv2::Create(vertexShader, fragmentShader);
	}

	if (!shaderPtr->Compile())
		return nullptr;

	return shaderPtr;
}

void ShaderLoader::UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool ShaderLoader::SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
