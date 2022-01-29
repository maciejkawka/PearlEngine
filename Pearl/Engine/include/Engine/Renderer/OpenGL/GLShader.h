#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Shader.h"
#include<string>

namespace PrRenderer::OpenGL {

	class GLShader : public Resources::Shader {
	public:
		GLShader(const std::string& p_name, PrCore::Resources::ResourceManager* p_creator, PrCore::Resources::ResourceID p_ID);

		~GLShader();

		void Bind() override;
		void Unbind() override;

		void SetUniformFloat(const std::string& p_name, float p_value) override;
		void SetUniformInt(const std::string& p_name, int p_value) override;
		void SetUniformMat4(const std::string& p_name, const PrCore::Math::mat4& p_value) override;
		void SetUniformMat3(const std::string& p_name, const PrCore::Math::mat3 p_value) override;
		void SetUniformVec4(const std::string& p_name, const PrCore::Math::vec4& p_value) override;
		void SetUniformVec3(const std::string& p_name, const PrCore::Math::vec3& p_value) override;
		void SetUniformVec2(const std::string& p_name, const PrCore::Math::vec2& p_value) override;

		void GetUniformFloat(const std::string& p_name) override;
		void GetUniformInt(const std::string& p_name) override;
		void GetUniformMat4(const std::string& p_name) override;
		void GetUniformMat3(const std::string& p_name) override;
		void GetUniformVec4(const std::string& p_name) override;
		void GetUniformVec3(const std::string& p_name) override;
		void GetUniformVec2(const std::string& p_name) override;
		
		

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

		std::string m_vertexShader;
		std::string m_fragmentShader;
	};
}
