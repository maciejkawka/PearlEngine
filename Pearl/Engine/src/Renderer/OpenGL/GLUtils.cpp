#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLUtils.h"

#include"glad/glad.h"

using namespace PrRenderer::OpenGL;

unsigned int PrRenderer::OpenGL::TextureFormatToGL(TextureFormat p_format)
{
	switch (p_format)
	{
	case TextureFormat::R8:
		return GL_RED;
		break;
	case TextureFormat::RG16:
		return GL_RG;
		break;
	case TextureFormat::RGB24:
	case TextureFormat::RGB16F:
		return GL_RGB;
		break;
	case TextureFormat::RGBA32:
	case TextureFormat::RGBA16F:
		return GL_RGBA;
		break;
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
		return GL_DEPTH_COMPONENT;
		break;
	case TextureFormat::Depth24Stencil8:
		return GL_DEPTH24_STENCIL8;
		break;
	case TextureFormat::Depth32Stencil8:
		return GL_DEPTH32F_STENCIL8;
		break;
	case TextureFormat::Stencil8:
		return GL_STENCIL_INDEX8;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureFormatToInternalGL(TextureFormat p_format)
{
	switch (p_format)
	{
	case TextureFormat::R8:
		return GL_R8;
		break;
	case TextureFormat::RG16:
		return GL_RG8;
		break;
	case TextureFormat::RGB24:
		return GL_RGB8;
		break;
	case TextureFormat::RGBA32:
		return GL_RGBA8;
		break;
	case TextureFormat::RGB16F:
		return GL_RGB16F;
		break;
	case TextureFormat::RGBA16F:
		return GL_RGBA16F;
		break;
	case TextureFormat::Depth16:
		return GL_DEPTH_COMPONENT16;
		break;
	case TextureFormat::Depth24:
		return GL_DEPTH_COMPONENT24;
		break;
	case TextureFormat::Depth32:
		return GL_DEPTH_COMPONENT32F;
		break;
	case TextureFormat::Depth24Stencil8:
		return GL_DEPTH24_STENCIL8;
		break;
	case TextureFormat::Depth32Stencil8:
		return GL_DEPTH32F_STENCIL8;
		break;
	case TextureFormat::Stencil8:
		return GL_STENCIL_INDEX8;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureWrapToGL(TextureWrapMode p_wrap)
{
	switch (p_wrap)
	{
	case TextureWrapMode::Clamp:
		return GL_CLAMP_TO_EDGE;
		break;
	case TextureWrapMode::Repeat:
		return GL_REPEAT;
		break;
	case TextureWrapMode::Mirror:
		return GL_MIRRORED_REPEAT;
		break;
	default:
		return GL_REPEAT; //If not recognise return default
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureFilterToGL(TextureFiltering p_filter)
{
	switch (p_filter)
	{
	case TextureFiltering::Linear:
		return GL_LINEAR;
		break;
	case TextureFiltering::Nearest:
		return GL_NEAREST;
		break;
	case TextureFiltering::LinearMipMapLinear:
		return GL_LINEAR_MIPMAP_LINEAR;
		break;
	case TextureFiltering::LinearMipmapNearest:
		return GL_LINEAR_MIPMAP_NEAREST;
		break;
	case TextureFiltering::NearestMipmapLinear:
		return GL_NEAREST_MIPMAP_LINEAR;
		break;
	case TextureFiltering::NearestMipmapNearest:
		return GL_NEAREST_MIPMAP_NEAREST;
		break;
	default:
		return GL_LINEAR; //If not recognise return default
		break;
	}
}

unsigned int PrRenderer::OpenGL::TextureFormatToDataTypeGL(TextureFormat p_format)
{
	switch (p_format)
	{
	case TextureFormat::R8:
	case TextureFormat::RG16:
	case TextureFormat::RGB24:
	case TextureFormat::RGBA32:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
	case TextureFormat::Depth24Stencil8:
	case TextureFormat::Depth32Stencil8:
	case TextureFormat::Stencil8:
		return GL_UNSIGNED_BYTE;
		break;
	case TextureFormat::RGB16F:
	case TextureFormat::RGBA16F:
		return GL_FLOAT;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::FramebufferMaskToGL(FramebufferMask p_mask)
{
	switch(p_mask)
	{
	case FramebufferMask::ColorBufferBit:
		return GL_COLOR_BUFFER_BIT;
	case FramebufferMask::DepthBufferBit:
		return GL_DEPTH_BUFFER_BIT;
	case FramebufferMask::StencilBufferBit:
		return GL_STENCIL_BUFFER_BIT;
	default: 
		return 0;
	}
}

unsigned int PrRenderer::OpenGL::PrimitiveToGL(Primitives p_primitives)
{
	switch (p_primitives)
	{
	case Primitives::Points:
		return GL_POINTS;
		break;
	case Primitives::LineStrip:
		return GL_LINE_STRIP;
		break;
	case Primitives::TriangleStrip:
		return GL_TRIANGLE_STRIP;
		break;
	case Primitives::Triangles:
		return GL_TRIANGLES;
		break;
	default:
		break;
	}
}

unsigned int PrRenderer::OpenGL::ComparaisonToGL(ComparaisonAlgorithm p_algorythm)
{
	switch (p_algorythm)
	{
	case ComparaisonAlgorithm::Never:
		return GL_NEVER;
		break;
	case ComparaisonAlgorithm::Less:
		return GL_LESS;
		break;
	case ComparaisonAlgorithm::Equal:
		return GL_EQUAL;
		break;
	case ComparaisonAlgorithm::LessEqual:
		return GL_LEQUAL;
		break;
	case ComparaisonAlgorithm::Greater:
		return GL_GREATER;
		break;
	case ComparaisonAlgorithm::NotEqual:
		return GL_NOTEQUAL;
		break;
	case ComparaisonAlgorithm::GreaterEqual:
		return GL_GEQUAL;
		break;
	case ComparaisonAlgorithm::Always:
		return GL_ALWAYS;
		break;
	default:
		break;
	}
}

unsigned int PrRenderer::OpenGL::BlendingToGL(BlendingAlgorithm p_algorythm)
{
	switch(p_algorythm)
	{
	case BlendingAlgorithm::Zero: 
		return GL_ZERO;
	case BlendingAlgorithm::One:
		return GL_ONE;
	case BlendingAlgorithm::SourceColor:
		return GL_SRC_COLOR;
	case BlendingAlgorithm::OneMinusSorceColor:
		return GL_ONE_MINUS_SRC_COLOR;
	case BlendingAlgorithm::DestinationColor:
		return GL_DST_COLOR;
	case BlendingAlgorithm::OneMinusDestinationColor:
		return GL_ONE_MINUS_DST_COLOR;
	case BlendingAlgorithm::SourceAlpha:
		return GL_SRC_ALPHA;
	case BlendingAlgorithm::OneMinusSourceAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case BlendingAlgorithm::DestinationAlpha:
		return GL_DST_ALPHA;
	case BlendingAlgorithm::OneMinusDestinationAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	default:
		return -1;
	}
}

unsigned int PrRenderer::OpenGL::CullFaceToGL(CullFaceMode p_mode)
{
	switch (p_mode)
	{
	case CullFaceMode::Back:
		return GL_BACK;
	case CullFaceMode::Front:
		return GL_FRONT;
	case CullFaceMode::BackAndFront:
		return GL_FRONT_AND_BACK;
	default:
		return -1;
	}
}

unsigned int PrRenderer::OpenGL::BoolToGL(bool p_bool)
{
	if (p_bool)
		return GL_TRUE;
	else
		return GL_FALSE;
}