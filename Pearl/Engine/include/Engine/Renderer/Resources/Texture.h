#pragma once
#include"Core/Resources/Resource.h"
#include"Renderer/Core/Defines.h"

namespace PrRenderer::Resources {

	enum class TextureFormat {
		R8,
		RG16,
		RGB24,
		RGBA32,
		RGB16F,
		RGBA16F,

		//Stencil and Depth
		Depth16,
		Depth24,
		Depth32,
		Depth32Stencil8,
		Depth24Stencil8,
		Stencil8,
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

	class Texture : public PrCore::Resources::Resource{
	public:
		Texture() :
			Resource("Texture"),
			m_ID(0),
			m_height(0),
			m_width(0),
			m_readable(false),
			m_mipmap(true),
			m_format(TextureFormat::None),
			m_minFiltering(TextureFiltering::Linear),
			m_magFiltering(TextureFiltering::Linear),
			m_wrapU(TextureWrapMode::Repeat),
			m_wrapV(TextureWrapMode::Repeat),
			m_rawData(nullptr)
		{}

		//Constructor for managed resource
		Texture(const std::string& p_name, PrCore::Resources::ResourceHandle p_handle) :
			Resource(p_name, p_handle),
			m_ID(0),
			m_height(0),
			m_width(0),
			m_readable(false),
			m_mipmap(true),
			m_format(TextureFormat::None),
			m_minFiltering(TextureFiltering::Nearest),
			m_magFiltering(TextureFiltering::Nearest),
			m_wrapU(TextureWrapMode::Repeat),
			m_wrapV(TextureWrapMode::Repeat),
			m_rawData(nullptr)
		{}

		virtual void Bind(unsigned int p_slot = 0) = 0;
		virtual void Unbind(unsigned int p_slot = 0) = 0;

		virtual void GenerateMipMaps() = 0;
		virtual void Apply() = 0;

		inline virtual void IsMipMapped(bool p_mipmap) { m_mipmap = p_mipmap; }
		inline virtual void IsReadable(bool p_readable) { m_readable = p_readable; }

		inline virtual void SetMinFiltering(TextureFiltering p_minfiltering) { m_minFiltering = p_minfiltering; }
		inline virtual void SetMagFiltering(TextureFiltering p_magfiltering) { m_magFiltering = p_magfiltering; }
		inline virtual void SetWrapModeU(TextureWrapMode p_wrapU) { m_wrapU = p_wrapU; }
		inline virtual void SetWrapModeV(TextureWrapMode p_wrapV) { m_wrapV = p_wrapV; }
		inline void SetHeight(size_t p_height) { m_height = p_height; }
		inline void SetWidth(size_t p_width) { m_width = p_width; }
		inline void SetFormat(TextureFormat p_format) { m_format = p_format; }

		inline RendererID GetRendererID() const { return m_ID; }
		inline size_t GetHeight() const { return m_height; }
		inline size_t GetWidth() const { return m_width; }
		inline bool  IsReadable() const { return m_readable; }
		inline bool IsMipMap() const { return m_mipmap; }
		inline TextureFiltering GetMinFiltering() const { return m_minFiltering; }
		inline TextureFiltering GetMagFiltering() const { return m_magFiltering; }
		inline TextureWrapMode GetWrapModeU() const { return m_wrapU; }
		inline TextureWrapMode GetWrapModeV() const { return m_wrapV; }
		inline TextureFormat GetFormat() const { return m_format; }

	protected:
		virtual void LoadUnitTexture(Core::Color p_unitColor) = 0;
		void LoadCorruptedResource() override;

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

		void* m_rawData;
	};

	typedef std::shared_ptr<Texture> TexturePtr;
}