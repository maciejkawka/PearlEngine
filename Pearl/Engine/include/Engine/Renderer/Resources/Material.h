#pragma once

#include"Renderer/Resources/Shader.h"
#include"Renderer/Resources/Cubemap.h"

#include "Core/Utils/ISerializable.h"
#include "Core/Utils/Logger.h"
#include "Core/Utils/Assert.h"

#include<vector>
#include<map>
#include<optional>

namespace PrRenderer {

	enum class RenderType {
		Opaque,
		Transparent
	};

	class Material : public PrCore::IResourceData, public PrCore::Utils::ISerializable {
	public:
		Material();
		Material(ShaderHandle p_shader);
		Material(const Material& p_material);

		void               SetColor(const Core::Color& p_color);
		const Core::Color& GetColor() const;

		inline void SetShader(ShaderPtr p_shader)
		{
			m_shader = p_shader;
			m_uniforms = m_shader->GetAllUniforms();
		}

		inline ShaderPtr GetShader() { return m_shader.GetData(); }
		inline bool      HasShader() const { return m_shader.IsValid(); }

		inline void   SetRenderOrder(size_t p_renderOrder) { m_renderOrder = p_renderOrder; }
		inline size_t GetRenderOrder() const { return m_renderOrder; }

		inline void       SetRenderType(RenderType p_renderType) { m_renderType = p_renderType; }
		inline RenderType GetRenderType() const { return m_renderType; }

		void Bind();
		void Unbind();

		void       SetTexture(const std::string& p_name, TextureHandle p_texture);
		void       SetCubemap(const std::string& p_name, CubemapHandle p_cubemap);
		TexturePtr GetTexture(const std::string& p_name);

		void SetTexScale(const PrCore::Math::vec2& p_value);
		void SetTexOffset(const PrCore::Math::vec2& p_value);

		PrCore::Math::vec2 GetTexScale() const;
		PrCore::Math::vec2 GetTexOffset() const;

		void CopyPropertiesFrom(const Material& p_material);

		bool HasProperty(const std::string& p_name) const;

		template<typename T>
		void SetProperty(const std::string& p_name, const T& p_value);

		template<typename T>
		void SetPropertyArray(const std::string& p_name, const T* p_value, unsigned int p_count);

		template<typename T>
		const T& GetProperty(const std::string& p_name) const;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override;
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override;

		size_t GetByteSize() const override;

		struct SerGuide
		{
			struct Texture
			{
				static constexpr const char* name = "name";
				static constexpr const char* path = "path";
			};

			struct Uniform
			{
				static constexpr const char* name = "name";
				static constexpr const char* value = "value";
				static constexpr const char* typeId = "typeId";
				static constexpr const char* type = "type";
			};

			static constexpr const char* name = "name";
			static constexpr const char* version = "version";
			static constexpr const char* shader = "shaderPath";
			static constexpr const char* renderOrder = "renderOrder";
			static constexpr const char* renderType = "renderType";

			static constexpr const char* uniforms = "uniforms";
			static constexpr const char* textures = "textures";
			static constexpr const char* cubemaps = "cubemaps";
		};

	protected:
		ShaderHandle                         m_shader;
		std::map<std::string, TextureHandle> m_textures;
		std::map<std::string, CubemapHandle> m_cubemaps;
		std::map<std::string, Uniform>       m_uniforms;
		RenderType                           m_renderType;
		size_t                               m_renderOrder;


		inline static TexturePtr blackTexture = nullptr;
		inline static TexturePtr blackCubemap = nullptr;

		static constexpr int     m_materialVersion = 1;
	};

	REGISTRER_RESOURCE_HANDLE(Material);
	typedef std::shared_ptr<Material> MaterialPtr;

	template<typename T>
	void Material::SetProperty(const std::string& p_name, const T& p_value)
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
	inline void Material::SetPropertyArray(const std::string& p_name, const T* p_value, unsigned int p_count)
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
	inline const T& Material::GetProperty(const std::string& p_name) const
	{

		auto find = m_uniforms.find(p_name);
		if (find != m_uniforms.end())
		{
			try
			{
				return std::any_cast<const T&>(find->second.value);;
			}
			catch (const std::bad_any_cast& e)
			{
				PRLOG_ERROR("Renderer: Material {0} {1}", p_name, e.what());
			}
		}

		PR_ASSERT(false, "Renderer: Material no such property. Returning invalid value!");
		static T invalid{};
		return invalid;
	}
}