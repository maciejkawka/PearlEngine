#pragma once
#include"Core/Resources/Resource.h"
#include"Renderer/Core/Defines.h"

namespace PrRenderer::Resources {

	enum class TextureFormat {
		Gray8,
		GrayAlpha8,
		R8,
		RG8,
		RGB8,
		RGBA8,
		Alpha8,
		None
	};

	enum class TextureFiltering {
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipMapLinear,
		None
	};

	enum class TextureWrapMode {
		Repeat,
		Clamp,
		Mirror,
		None
	};

	class Texture : public PrCore::Resources::Resources{
	public:
		Texture() = default;
		Texture(const std::string& p_name, PrCore::Resources::ResourceManager* p_creator, PrCore::Resources::ResourceID p_ID) :
			Resources(p_name, p_creator, p_ID),
			m_ID(0),
			m_height(0),
			m_width(0),
			m_readable(false),
			m_mipmap(false),
			m_format(TextureFormat::None),
			m_minFiltering(TextureFiltering::Linear),
			m_magFiltering(TextureFiltering::Linear),
			m_wrapU(TextureWrapMode::Repeat),
			m_wrapV(TextureWrapMode::Repeat),
			m_rawData(nullptr)
		{}

		virtual void Bind(unsigned int p_slot = 0) = 0;
		virtual void Unbind() = 0;

		inline virtual void IsMipMapped(bool p_mipmap) { m_mipmap = p_mipmap; }
		inline virtual void IsReadable(bool p_readable) { m_readable = p_readable; }

		inline virtual void SetMinFiltering(TextureFiltering p_minfiltering) { m_minFiltering = p_minfiltering; }
		inline virtual void SetMagFiltering(TextureFiltering p_magfiltering) { m_magFiltering = p_magfiltering; }
		inline virtual void SetWrapModeU(TextureWrapMode p_wrapU) { m_wrapU = p_wrapU; }
		inline virtual void SetWrapModeV(TextureWrapMode p_wrapV) { m_wrapV = p_wrapV; }

		inline size_t GetHeight() { return m_height; }
		inline size_t GetWidth() { return m_width; }
		inline bool  IsReadable() { return m_readable; }
		inline bool IsMipMap() { return m_mipmap; }
		inline TextureFiltering GetMinFiltering() { return m_minFiltering; }
		inline TextureFiltering GetMagFiltering() { return m_magFiltering; }
		inline TextureWrapMode GetWrapModeU() { return m_wrapU; }
		inline TextureWrapMode GetWrapModeV() { return m_wrapV; }
		inline TextureFormat GetFormat() { return m_format; }

	protected:
		RendererID m_ID;
		size_t m_height;
		size_t m_width;
		bool  m_readable;
		bool m_mipmap;

		TextureFormat m_format;
		TextureFiltering m_minFiltering;
		TextureFiltering m_magFiltering;
		TextureWrapMode m_wrapU;
		TextureWrapMode m_wrapV;

		unsigned char* m_rawData;
	};

	typedef std::shared_ptr<Texture> TexturePtr;
}