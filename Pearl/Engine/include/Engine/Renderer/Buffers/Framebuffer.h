#pragma once
#include<any>
#include<vector>
#include"Renderer/Core/Defines.h"
#include"Renderer/Resources/Texture2D.h"

namespace PrRenderer::Buffers {

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
			Resources::TextureFormat p_format,
			Resources::TextureFiltering p_filteringMin = Resources::TextureFiltering::Linear,
			Resources::TextureFiltering p_filteringMag = Resources::TextureFiltering::Linear,
			Resources::TextureWrapMode p_wrapModeU = Resources::TextureWrapMode::Clamp,
			Resources::TextureWrapMode p_wrapModeV = Resources::TextureWrapMode::Clamp,
			size_t p_with = 0,
			size_t p_height = 0,
			bool p_cubeTexture = false,
			Resources::TextureWrapMode p_wrapModeR = Resources::TextureWrapMode::Clamp) :
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
		Resources::TextureFormat format = Resources::TextureFormat::RGBA32;
		Resources::TextureFiltering filteringMin = Resources::TextureFiltering::Linear;
		Resources::TextureFiltering filteringMag = Resources::TextureFiltering::Linear;
		Resources::TextureWrapMode wrapModeU = Resources::TextureWrapMode::Clamp;
		Resources::TextureWrapMode wrapModeV = Resources::TextureWrapMode::Clamp;
		Resources::TextureWrapMode wrapModeR = Resources::TextureWrapMode::Clamp;
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
		bool mipMaped = false;
		int samples = 1;
		FramebufferTexAttachments colorTextureAttachments;
		FramebufferTexture depthStencilAttachment = Resources::TextureFormat::None;
	};

	class Framebufffer {
	public:
		Framebufffer() = default;
		virtual ~Framebufffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void SetAttachmentDetails(int p_attachment, int p_textureTarget, int p_mipLevel = 0) = 0;
		virtual void Resize(size_t width, size_t height) = 0;
		virtual void ClearAttachmentColor(unsigned int p_attachment, Core::Color p_color) = 0;
		
		virtual Resources::TexturePtr GetTexturePtr(unsigned int p_index = 0) = 0;
		virtual Resources::TexturePtr GetDepthTexturePtr() = 0;
		virtual RendererID GetTextureID(unsigned int p_index = 0) = 0;

		inline const FramebufferSettings& GetSettings() const { return m_settings; }
		inline RendererID GetID() const { return m_ID; }

		static FramebuffferPtr Create(const FramebufferSettings& p_settings);

	protected:
		RendererID m_ID;
		std::vector<RendererID> m_colorTextureIDs;
		RendererID m_depthTextureID;

		FramebufferSettings m_settings;
	};

	typedef std::shared_ptr<Framebufffer> FramebuffferPtr;
}