#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLDefines.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

unsigned int PrRenderer::OpenGL::TextureFormatToGL(Resources::TextureFormat p_format)
{
	switch (p_format)
	{
	case Resources::TextureFormat::R8:
		return GL_RED;
		break;
	case Resources::TextureFormat::RG16:
		return GL_RG;
		break;
	case Resources::TextureFormat::RGB24:
		return GL_RGB;
		break;
	case Resources::TextureFormat::RGBA32:
		return GL_RGBA;
		break;
	case Resources::TextureFormat::Depth16:
	case Resources::TextureFormat::Depth24:
	case Resources::TextureFormat::Depth32:
		return GL_DEPTH_COMPONENT;
		break;
	case Resources::TextureFormat::Depth24Stencil8:
		return GL_DEPTH24_STENCIL8;
		break;
	case Resources::TextureFormat::Depth32Stencil8:
		return GL_DEPTH32F_STENCIL8;
		break;
	case Resources::TextureFormat::Stencil8:
		return GL_STENCIL_INDEX8;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureFormatToInternalGL(Resources::TextureFormat p_format)
{
	switch (p_format)
	{
	case Resources::TextureFormat::R8:
		return GL_R8;
		break;
	case Resources::TextureFormat::RG16:
		return GL_RG8;
		break;
	case Resources::TextureFormat::RGB24:
		return GL_RGB8;
		break;
	case Resources::TextureFormat::RGBA32:
		return GL_RGBA8;
		break;
	case Resources::TextureFormat::Depth16:
		return GL_DEPTH_COMPONENT16;
		break;
	case Resources::TextureFormat::Depth24:
		return GL_DEPTH_COMPONENT24;
		break;
	case Resources::TextureFormat::Depth32:
		return GL_DEPTH_COMPONENT32F;
		break;
	case Resources::TextureFormat::Depth24Stencil8:
		return GL_DEPTH24_STENCIL8;
		break;
	case Resources::TextureFormat::Depth32Stencil8:
		return GL_DEPTH32F_STENCIL8;
		break;
	case Resources::TextureFormat::Stencil8:
		return GL_STENCIL_INDEX8;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureWrapToGL(PrRenderer::Resources::TextureWrapMode p_wrap)
{
	switch (p_wrap)
	{
	case Resources::TextureWrapMode::Clamp:
		return GL_CLAMP_TO_EDGE;
		break;
	case Resources::TextureWrapMode::Repeat:
		return GL_REPEAT;
		break;
	case Resources::TextureWrapMode::Mirror:
		return GL_MIRRORED_REPEAT;
		break;
	default:
		return GL_REPEAT; //If not recognise return default
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureFilterToGL(PrRenderer::Resources::TextureFiltering p_filter)
{
	switch (p_filter)
	{
	case Resources::TextureFiltering::Linear:
		return GL_LINEAR;
		break;
	case Resources::TextureFiltering::Nearest:
		return GL_NEAREST;
		break;
	case Resources::TextureFiltering::LinearMipMapLinear:
		return GL_LINEAR_MIPMAP_LINEAR;
		break;
	case Resources::TextureFiltering::LinearMipmapNearest:
		return GL_LINEAR_MIPMAP_NEAREST;
		break;
	case Resources::TextureFiltering::NearestMipmapLinear:
		return GL_NEAREST_MIPMAP_LINEAR;
		break;
	case Resources::TextureFiltering::NearestMipmapNearest:
		return GL_NEAREST_MIPMAP_NEAREST;
		break;
	default:
		return GL_LINEAR; //If not recognise return default
		break;
	}
}