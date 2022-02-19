#pragma once
#include"Core/Resources/Resource.h"
#include"Core/Math/Math.h"

#include"Renderer/Core/Defines.h"
#include"Renderer/Core/Color.h"
#include"Renderer/Resources/Uniform.h"
#include"Renderer/Resources/Shader.h"
#include"Renderer/Resources/Texture.h"

#include"Core/Utils/JSONParser.h"

#include<vector>

namespace PrRenderer::Resources {

	enum class RenderType {
		Opaque,
		Transparent
	};

	struct TextureData {
		std::string name;
		TexturePtr resource;
		PrCore::Math::vec2 offset;
		PrCore::Math::vec3 scale;
	};

	class Material : public PrCore::Resources::Resources {
	public:
		Material(const std::string& p_name, PrCore::Resources::ResourceManager* p_creator, PrCore::Resources::ResourceID p_ID);
		//Material(const Material& p_material) {}
		//Material(ShaderPtr p_shader) {}

		~Material() {}

		inline void SetColor(const Core::Color& p_color) { m_color = p_color; }
		inline const Core::Color& GetColor() { return m_color; }

		inline void SetShader(ShaderPtr p_shader) { m_shader = p_shader; }
		inline ShaderPtr GetShader() { return m_shader; }
		inline bool HasShader() { return m_shader != nullptr; }

		inline void SetRenderOrder(size_t p_renderOrder) { m_renderOrder = p_renderOrder; }
		inline size_t GetRenderOrder() { return m_renderOrder; }

		inline void SetRenderType(RenderType p_renderType) { m_renderType = p_renderType; }
		inline RenderType GetRenderType() { return m_renderType; }

		//void Bind();
		//void Unbind();

		//void SetTexture(const std::string& p_name, TexturePtr p_texture);
		//TexturePtr GetTexture(const std::string& p_name);

		//void SetTexScale(const std::string& p_name, const PrCore::Math::vec2& p_value);
		//void SetTexOffset(const std::string& p_name, const PrCore::Math::vec2& p_value);

		//PrCore::Math::vec2 GetTexScale(const std::string& p_name);
		//PrCore::Math::vec2 GetTexOffset(const std::string& p_name);

		//bool HasProperty(const std::string& p_name);

		//Templated
		//template<typename T>
		//void SetProperty(const std::string& m_name, const T& p_value)
		//{

		//}

		////Templated
		//template<typename T>
		//const T& GetProperty(const std::string& p_name)
		//{

		//}

	protected:

		virtual void PreLoadImpl() override;
		virtual bool LoadImpl() override;
		virtual void PostLoadImpl() override;

		virtual void PreUnloadImpl() override;
		virtual bool UnloadImpl() override;
		virtual void PostUnloadImpl() override;

		virtual void CalculateSize() override;

		bool PopulateMaterial(PrCore::Utils::JSON::json& p_json);
		PrCore::Utils::JSON::json ReadFile();

		ShaderPtr m_shader;
		std::vector<TextureData> m_textures;
		std::vector<Uniform> m_unforms;
		Core::Color m_color;
		RenderType m_renderType;
		size_t m_renderOrder;

		const float m_materialVersion = 0.1f;
	};

	typedef std::shared_ptr<Material> MaterialPtr;
}