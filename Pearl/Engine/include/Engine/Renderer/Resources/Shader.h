#pragma once
#include"Core/Resources/Resource.h"
#include"Core/Math/Math.h"

#include"Renderer/Resources/Uniform.h"
#include"Renderer/Core/Defines.h"

#include<vector>
#include<map>
#include<unordered_map>
#include<string>
#include<memory>

namespace PrRenderer::Resources {

	class Shader : public PrCore::Resources::Resource {
	public:
		Shader() = delete;
		Shader(Shader&) = delete;
		Shader(Shader&&) = delete;

		//Constructor for managed resource 
		Shader(const std::string& p_name, PrCore::Resources::ResourceHandle p_ID) :
			Resource(p_name, p_ID),
			m_ID(0)
		{}

		virtual ~Shader() override = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		inline RendererID GetID() { return m_ID; }

		inline size_t GetUniformsCount() { return m_uniforms.size(); }
		inline const std::map<std::string, Uniform>& GetAllUniforms() { return m_uniforms; }

		virtual void SetUniformFloat(const std::string& p_name, float p_value) = 0;
		virtual void SetUniformInt(const std::string& p_name, int p_value) = 0;
		virtual void SetUniformBool(const std::string& p_name, bool p_value) = 0;
		virtual void SetUniformMat4(const std::string& p_name, const PrCore::Math::mat4& p_value) = 0;
		virtual void SetUniformMat3(const std::string& p_name, const PrCore::Math::mat3 p_value) = 0;
		virtual void SetUniformVec4(const std::string& p_name, const PrCore::Math::vec4& p_value) = 0;
		virtual void SetUniformVec3(const std::string& p_name, const PrCore::Math::vec3& p_value) = 0;
		virtual void SetUniformVec2(const std::string& p_name, const PrCore::Math::vec2& p_value) = 0;

		virtual void SetUniformFloatArray(const std::string& p_name, float* p_value, unsigned int p_count) = 0;
		virtual void SetUniformIntArray(const std::string& p_name, int* p_value, unsigned int p_count) = 0;
		virtual void SetUniformMat4Array(const std::string& p_name, const PrCore::Math::mat4* p_value, unsigned int p_count) = 0;
		virtual void SetUniformMat3Array(const std::string& p_name, const PrCore::Math::mat3* p_value, unsigned int p_count) = 0;
		virtual void SetUniformVec4Array(const std::string& p_name, const PrCore::Math::vec4* p_value, unsigned int p_count) = 0;
		virtual void SetUniformVec3Array(const std::string& p_name, const PrCore::Math::vec3* p_value, unsigned int p_count) = 0;
		virtual void SetUniformVec2Array(const std::string& p_name, const PrCore::Math::vec2* p_value, unsigned int p_count) = 0;

		virtual float GetUniformFloat(const std::string& p_name) = 0;
		virtual int GetUniformInt(const std::string& p_name) = 0;
		virtual bool GetUniformBool(const std::string& p_name) = 0;
		virtual PrCore::Math::mat4 GetUniformMat4(const std::string& p_name) = 0;
		virtual PrCore::Math::mat3 GetUniformMat3(const std::string& p_name) = 0;
		virtual PrCore::Math::vec4 GetUniformVec4(const std::string& p_name) = 0;
		virtual PrCore::Math::vec3 GetUniformVec3(const std::string& p_name) = 0;
		virtual PrCore::Math::vec2 GetUniformVec2(const std::string& p_name) = 0;

	protected:
		RendererID m_ID;

		std::map<std::string, Uniform> m_uniforms;
		std::unordered_map<std::string, int> m_uniformLocation;
	};

	typedef std::shared_ptr<Shader> ShaderPtr;
}