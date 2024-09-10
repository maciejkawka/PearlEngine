#pragma once

#include "Renderer/Core/Defines.h"
#include "Core/Resources/IResource.h"

//Delete later
#include "Renderer/Resources/Texture.h"
//

#include<memory>

namespace PrRenderer::Core {
	class Color;
}

namespace PrRenderer::Resources {

	//enum class TextureFormat {
	//	R8,
	//	RG16,
	//	RGB24,
	//	RGBA32,
	//	RGB16F,
	//	RGBA16F,

	//	//Stencil and Depth
	//	Depth16,
	//	Depth24,
	//	Depth32,
	//	Depth32Stencil8,
	//	Depth24Stencil8,
	//	Stencil8,
	//	None
	//};

	//enum class TextureFiltering {
	//	Nearest,
	//	Linear,
	//	NearestMipmapNearest,
	//	LinearMipmapNearest,
	//	NearestMipmapLinear,
	//	LinearMipMapLinear,
	//	None
	//};

	//enum class TextureWrapMode {
	//	Repeat,
	//	Clamp,
	//	Mirror,
	//	None
	//};

	class Texturev2: public PrCore::Resources::IResourceData {
	public:
		Texturev2() :
			m_ID(0U),
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
		virtual ~Texturev2() = default;

		virtual void Bind(unsigned int p_slot = 0) = 0;
		virtual void Unbind(unsigned int p_slot = 0) = 0;

		virtual void GenerateMipMaps() = 0;
		virtual void Apply() = 0;

		inline virtual void SetMipMap(bool p_mipmap) { m_mipmap = p_mipmap; }
		inline virtual void SetReadable(bool p_readable) { m_readable = p_readable; }

		inline virtual void SetMinFiltering(TextureFiltering p_minfiltering) { m_minFiltering = p_minfiltering; }
		inline virtual void SetMagFiltering(TextureFiltering p_magfiltering) { m_magFiltering = p_magfiltering; }
		inline virtual void SetWrapModeU(TextureWrapMode p_wrapU) { m_wrapU = p_wrapU; }
		inline virtual void SetWrapModeV(TextureWrapMode p_wrapV) { m_wrapV = p_wrapV; }
		inline virtual void SetHeight(size_t p_height) { m_height = p_height; }
		inline virtual void SetWidth(size_t p_width) { m_width = p_width; }
		inline virtual void SetFormat(TextureFormat p_format) { m_format = p_format; }
		inline virtual void SetData(void* p_data) { m_rawData = p_data; }

		inline RendererID GetRendererID() const { return m_ID; }
		inline size_t GetHeight() const { return m_height; }
		inline size_t GetWidth() const { return m_width; }
		inline bool  SetReadable() const { return m_readable; }
		inline bool IsMipMap() const { return m_mipmap; }
		inline TextureFiltering GetMinFiltering() const { return m_minFiltering; }
		inline TextureFiltering GetMagFiltering() const { return m_magFiltering; }
		inline TextureWrapMode GetWrapModeU() const { return m_wrapU; }
		inline TextureWrapMode GetWrapModeV() const { return m_wrapV; }
		inline TextureFormat GetFormat() const { return m_format; }
		inline void* GetData(void* p_data) { return m_rawData; }


		size_t GetByteSize();

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

	typedef std::shared_ptr<Texturev2> Texturev2Ptr;
}