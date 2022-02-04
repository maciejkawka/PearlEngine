#pragma once
#include"Core/Resources/Resource.h"
#include"Renderer/Core/Defines.h"

namespace PrRenderer::Resources {

	enum class TextureFormat {
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
		virtual ~Texture() = default;

		virtual void Bind(unsigned int p_slot = 0) = 0;
		virtual void Unbind() = 0;

		inline void IsMipMapped(bool p_mipmap) { m_mipmap = p_mipmap; }
		inline void IsReadable(bool p_readable) { m_readable = p_readable; }

		inline void SetMinFiltering(TextureFiltering p_minfiltering) { m_minFiltering = p_minfiltering; }
		inline void SetMagFiltering(TextureFiltering p_magfiltering) { m_magFiltering = p_magfiltering; }
		inline void SetWrapModeU(TextureWrapMode p_wrapU) { m_wrapU = p_wrapU; }
		inline void SetWrapModeV(TextureWrapMode p_wrapV) { m_wrapV = p_wrapV; }

		inline size_t GetHeight() { return m_height; }
		inline size_t GetWidth() { return m_width; }
		inline bool  IsReadable() { return m_readable; }
		inline bool IsMipMap() { return m_mipmap; }
		inline TextureFiltering GetMinFiltering() { return m_minFiltering; }
		inline TextureFiltering GetMinFiltering() { return m_magFiltering; }
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
	};
}