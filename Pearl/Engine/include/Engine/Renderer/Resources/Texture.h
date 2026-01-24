#pragma once

#include "Renderer/Core/Defines.h"
#include "Core/Resources/IResource.h"

#include<memory>

namespace PrRenderer {
	class Color;
}

namespace PrRenderer {

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

	class Texture : public PrCore::IResourceData {
	public:
		Texture() :
			m_ID(0U),
			m_height(0),
			m_width(0),
			m_readable(false),
			m_mipmap(true),
			m_format(TextureFormat::None),
			m_minFiltering(TextureFiltering::LinearMipMapLinear),
			m_magFiltering(TextureFiltering::Linear),
			m_wrapU(TextureWrapMode::Repeat),
			m_wrapV(TextureWrapMode::Repeat),
			m_rawData(nullptr)
		{}
		virtual ~Texture() = default;

		virtual void Bind(unsigned int p_slot = 0) = 0;
		virtual void Unbind(unsigned int p_slot = 0) = 0;

		virtual void  GenerateMipMaps() = 0;
		virtual void* FetchGPUData(int p_level = 0) = 0;
		virtual void  Apply() = 0;

		virtual void ClearWithColor(const Color& p_color) = 0;

		virtual void SetMinFiltering(TextureFiltering p_minfiltering) { m_minFiltering = p_minfiltering; }
		virtual void SetMagFiltering(TextureFiltering p_magfiltering) { m_magFiltering = p_magfiltering; }
		virtual void SetWrapModeU(TextureWrapMode p_wrapU) { m_wrapU = p_wrapU; }
		virtual void SetWrapModeV(TextureWrapMode p_wrapV) { m_wrapV = p_wrapV; }
		virtual void SetHeight(size_t p_height) { m_height = p_height; }
		virtual void SetWidth(size_t p_width) { m_width = p_width; }
		virtual void SetFormat(TextureFormat p_format) { m_format = p_format; }
		virtual void SetData(void* p_data) { m_rawData = p_data; }
		virtual void SetMipMap(bool p_mipmap) { m_mipmap = p_mipmap; }
		virtual void SetReadable(bool p_readable) { m_readable = p_readable; }

		RendererID       GetRendererID() const { return m_ID; }
		size_t           GetHeight() const { return m_height; }
		size_t           GetWidth() const { return m_width; }
		bool             GetReadable() const { return m_readable; }
		bool             IsMipMap() const { return m_mipmap; }
		TextureFiltering GetMinFiltering() const { return m_minFiltering; }
		TextureFiltering GetMagFiltering() const { return m_magFiltering; }
		TextureWrapMode  GetWrapModeU() const { return m_wrapU; }
		TextureWrapMode  GetWrapModeV() const { return m_wrapV; }
		TextureFormat    GetFormat() const { return m_format; }
		void*            GetData() { return m_rawData; }


	protected:
		virtual void CalculateSize() = 0;

		RendererID m_ID;
		size_t m_height;
		size_t m_width;
		bool m_readable;
		bool m_mipmap;
		size_t m_size;

		TextureFormat m_format;
		TextureFiltering m_minFiltering;
		TextureFiltering m_magFiltering;
		TextureWrapMode m_wrapU;
		TextureWrapMode m_wrapV;

		void* m_rawData;
	};

	REGISTRER_RESOURCE_HANDLE(Texture);
	typedef std::shared_ptr<Texture> TexturePtr;
}