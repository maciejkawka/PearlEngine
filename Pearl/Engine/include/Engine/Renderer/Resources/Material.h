#pragma once

#include"Core/Resources/IResource.h"

#include"Renderer/Core/Defines.h"
#include"Renderer/Core/Color.h"
#include"Renderer/Resources/Uniform.h"
#include"Renderer/Resources/Shader.h"
#include"Renderer/Resources/Cubemap.h"
#include"Renderer/Resources/Texture.h"

// Remove later
#include"Renderer/Resources/Material.h"

#include"Core/Utils/JSONParser.h"
#include"Core/Math/Math.h"
#include"Core/Utils/Logger.h"

#include<vector>
#include<map>
#include<optional>

namespace PrRenderer::Resources {

	enum class RenderType {
		Opaque,
		Transparent
	};

	class Material : public PrCore::Resources::IResourceData {
	public:
		Material() = delete;
		Material(Shaderv2Ptr p_shader);
		Material(const Material& p_material);
		Material(PrCore::Utils::JSON::json& p_seralizedMat);

		void SetColor(const Core::Color& p_color);
		const Core::Color& GetColor() const;

		inline void SetShader(Shaderv2Ptr p_shader)
		{
			m_shader = p_shader;
			m_uniforms = m_shader->GetAllUniforms();
		}

		inline Shaderv2Ptr GetShader() { return m_shader; }
		inline bool HasShader() const { return m_shader != nullptr; }

		inline void SetRenderOrder(size_t p_renderOrder) { m_renderOrder = p_renderOrder; }
		inline size_t GetRenderOrder() const { return m_renderOrder; }

		inline void SetRenderType(RenderType p_renderType) { m_renderType = p_renderType; }
		inline RenderType GetRenderType() const { return m_renderType; }

		void Bind();
		void Unbind();

		void SetTexture(const std::string& p_name, Texturev2Ptr p_texture);
		Texturev2Ptr GetTexture(const std::string& p_name);

		void SetTexScale(const std::string& p_name, const PrCore::Math::vec2& p_value);
		void SetTexOffset(const std::string& p_name, const PrCore::Math::vec2& p_value);

		PrCore::Math::vec2 GetTexScale(const std::string& p_name) const;
		PrCore::Math::vec2 GetTexOffset(const std::string& p_name) const;

		bool HasProperty(const std::string& p_name) const;

		void CopyPropertiesFrom(const Material& p_material);

		// Templated
		template<typename T>
		void SetProperty(const std::string& p_name, const T& p_value);

		template<typename T>
		void SetPropertyArray(const std::string& p_name, const T* p_value, unsigned int p_count);

		template<typename T>
		const T& GetProperty(const std::string& p_name);


		size_t GetByteSize() const override;

	protected:
		bool PopulateBasedOnShader(PrCore::Utils::JSON::json& p_json);

		Shaderv2Ptr m_shader;
		std::map<std::string, Texturev2Ptr> m_textures;
		std::map<std::string, Uniform> m_uniforms;
		RenderType m_renderType;
		size_t m_renderOrder;

		const float m_materialVersion = 0.1f;
	};

	REGISTRER_RESOURCE_HANDLE(Material);
	typedef std::shared_ptr<Material> Materialv2Ptr;


	//Templated Implementation
	template<typename T>
	inline void Material::SetProperty(const std::string& p_name, const T& p_value)
	{
		auto find = m_uniforms.find(p_name);
		if (find != m_uniforms.end())
		{
			if (find->second.size != 1)
			{
				PRLOG_WARN("Renderer: Material {0} property {1} is an array", m_name, p_name);
				return;
			}

			find->second.value = std::make_any<T>(p_value);
		}
		else
		{
			PRLOG_WARN("Renderer: Material {0} does not have property {1}", m_name, p_name);
		}
	}

	template<typename T>
	inline void Material::SetPropertyArray(const std::string& p_name, const T const* p_value, unsigned int p_count)
	{
		auto find = m_uniforms.find(p_name);
		if (find != m_uniforms.end())
		{
			if (find->second.size == 1)
			{
				PRLOG_WARN("Renderer: Material {0} property {1} is not an array", m_name, p_name);
				return;
			}

			std::vector<T> tempVector(find->second.size);
			for (int i = 0; i < p_count; i++)
				tempVector[i] = p_value[i];
			find->second.value = std::make_any<std::vector<T>>(tempVector);
		}
		else
		{
			PRLOG_WARN("Renderer: Material {0} does not have property {1}", m_name, p_name);
		}
	}

	template<typename T>
	inline const T& Material::GetProperty(const std::string& p_name)
	{

		auto find = m_uniforms.find(p_name);
		if (find != m_uniforms.end())
		{
			try
			{
				T& returnValue = std::any_cast<T&>(find->second.value);
				return returnValue;
			}
			catch (const std::bad_any_cast& e)
			{
				PRLOG_ERROR("Renderer: Material {0} {1}", p_name, e.what());
			}
		}

		PRLOG_WARN("Renderer: Material {0} no such property \"{1}\"", m_name, p_name);
		return T();
	}
}