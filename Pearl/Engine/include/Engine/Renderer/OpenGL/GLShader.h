#pragma once
#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Shader.h"
#include<string>

namespace PrRenderer::OpenGL {

	class GLShader : public Resources::Shader {
	public:
		GLShader(const std::string& p_name, PrCore::Resources::ResourceID p_ID);

		~GLShader();

		void Bind() override;
		void Unbind() override;

		void SetUniformFloat(const std::string& p_name, float p_value) override;
		void SetUniformInt(const std::string& p_name, int p_value) override;
		void SetUniformBool(const std::string& p_name, bool p_value) override;
		void SetUniformMat4(const std::string& p_name, const PrCore::Math::mat4& p_value) override;
		void SetUniformMat3(const std::string& p_name, const PrCore::Math::mat3 p_value) override;
		void SetUniformVec4(const std::string& p_name, const PrCore::Math::vec4& p_value) override;
		void SetUniformVec3(const std::string& p_name, const PrCore::Math::vec3& p_value) override;
		void SetUniformVec2(const std::string& p_name, const PrCore::Math::vec2& p_value) override;

		void SetUniformFloatArray(const std::string& p_name, float* p_value, unsigned int p_count) override;
		void SetUniformIntArray(const std::string& p_name, int* p_value, unsigned int p_count) override;
		void SetUniformMat4Array(const std::string& p_name, const PrCore::Math::mat4* p_value, unsigned int p_count) override;
		void SetUniformMat3Array(const std::string& p_name, const PrCore::Math::mat3* p_value, unsigned int p_count) override;
		void SetUniformVec4Array(const std::string& p_name, const PrCore::Math::vec4* p_value, unsigned int p_count) override;
		void SetUniformVec3Array(const std::string& p_name, const PrCore::Math::vec3* p_value, unsigned int p_count) override;
		void SetUniformVec2Array(const std::string& p_name, const PrCore::Math::vec2* p_value, unsigned int p_count) override;

		float GetUniformFloat(const std::string& p_name) override;
		int GetUniformInt(const std::string& p_name) override;
		bool GetUniformBool(const std::string& p_name) override;
		PrCore::Math::mat4 GetUniformMat4(const std::string& p_name) override;
		PrCore::Math::mat3 GetUniformMat3(const std::string& p_name) override;
		PrCore::Math::vec4 GetUniformVec4(const std::string& p_name) override;
		PrCore::Math::vec3 GetUniformVec3(const std::string& p_name) override;
		PrCore::Math::vec2 GetUniformVec2(const std::string& p_name) override;
		
		

	private:
		void PreLoadImpl() override;
		bool LoadImpl() override;
		void PostLoadImpl() override;

		void PreUnloadImpl() override;
		bool UnloadImpl() override;
		void PostUnloadImpl() override;

		void CalculateSize() override;

		bool Compile();
		void ScanUniforms();
		int GetUniformLocation(const std::string& p_name);

		std::string m_vertexShader;
		std::string m_fragmentShader;
	};
}
