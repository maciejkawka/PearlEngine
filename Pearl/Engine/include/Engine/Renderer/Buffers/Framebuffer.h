#pragma once

#include <any>
#include <vector>
#include "Renderer/Core/Defines.h"
#include "Renderer/Resources/Texture2D.h"

namespace PrRenderer {

	class Framebufffer;
	typedef std::shared_ptr<Framebufffer> FramebuffferPtr;

	enum class FramebufferMask
	{
		ColorBufferBit,
		DepthBufferBit,
		StencilBufferBit,
		None
	};

	struct FramebufferTexture {

		FramebufferTexture() = default;
		FramebufferTexture(
			TextureFormat p_format,
			TextureFiltering p_filteringMin = TextureFiltering::Linear,
			TextureFiltering p_filteringMag = TextureFiltering::Linear,
			TextureWrapMode p_wrapModeU = TextureWrapMode::Clamp,
			TextureWrapMode p_wrapModeV = TextureWrapMode::Clamp,
			size_t p_with = 0,
			size_t p_height = 0,
			bool p_cubeTexture = false,
			TextureWrapMode p_wrapModeR = TextureWrapMode::Clamp) :
			width(p_with),
			height(p_height),
			format(p_format),
			filteringMin(p_filteringMin),
			filteringMag(p_filteringMag),
			wrapModeU(p_wrapModeU),
			wrapModeV(p_wrapModeV),
			wrapModeR(p_wrapModeR),
			cubeTexture(p_cubeTexture)
		{}

		size_t width = 0;
		size_t height = 0;
		TextureFormat format = TextureFormat::RGBA32;
		TextureFiltering filteringMin = TextureFiltering::Linear;
		TextureFiltering filteringMag = TextureFiltering::Linear;
		TextureWrapMode wrapModeU = TextureWrapMode::Clamp;
		TextureWrapMode wrapModeV = TextureWrapMode::Clamp;
		TextureWrapMode wrapModeR = TextureWrapMode::Clamp;
		bool cubeTexture = false;
	};

	struct FramebufferTexAttachments {

		FramebufferTexAttachments() = default;
		FramebufferTexAttachments(FramebufferTexture p_texture)
		{
			textures.push_back(p_texture);
		}

		FramebufferTexAttachments(std::initializer_list<FramebufferTexture> p_textures) :
			textures(p_textures)
		{}

		std::vector<FramebufferTexture> textures;
	};

	struct FramebufferSettings {
		size_t globalWidth = 0;
		size_t globalHeight = 0;
		bool   mipMaped = false;
		int    samples = 1;

		FramebufferTexAttachments    colorTextureAttachments;
		FramebufferTexture           depthStencilAttachment = TextureFormat::None;
	};

	class Framebufffer {
	public:
		Framebufffer() = default;
		virtual ~Framebufffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void SetAttachmentDetails(int p_attachment, int p_textureTarget, int p_mipLevel = 0) = 0;
		virtual void Resize(size_t width, size_t height) = 0;
		virtual void ClearAttachmentColor(unsigned int p_attachemntIndex, const Color& p_color) = 0;
		
		virtual TexturePtr GetTexturePtr(unsigned int p_index = 0) = 0;
		virtual TexturePtr GetDepthTexturePtr() = 0;

		inline const FramebufferSettings& GetSettings() const { return m_settings; }
		inline RendererID                 GetID() const { return m_ID; }

		static FramebuffferPtr Create(const FramebufferSettings& p_settings);

	protected:
		RendererID               m_ID;
		FramebufferSettings      m_settings;
		TexturePtr               m_depthTexture;
		std::vector<TexturePtr>  m_colorTextures;
	};

	typedef std::shared_ptr<Framebufffer> FramebuffferPtr;
}