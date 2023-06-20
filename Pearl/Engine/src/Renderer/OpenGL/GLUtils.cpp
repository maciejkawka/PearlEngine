#include"Core/Common/pearl_pch.h"

#include"Renderer/OpenGL/GLUtils.h"

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
	case Resources::TextureFormat::RGB16F:
		return GL_RGB;
		break;
	case Resources::TextureFormat::RGBA32:
	case Resources::TextureFormat::RGBA16F:
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
	case Resources::TextureFormat::RGB16F:
		return GL_RGB16F;
		break;
	case Resources::TextureFormat::RGBA16F:
		return GL_RGBA16F;
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

unsigned int PrRenderer::OpenGL::TextureWrapToGL(Resources::TextureWrapMode p_wrap)
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

unsigned int PrRenderer::OpenGL::TextureFilterToGL(Resources::TextureFiltering p_filter)
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

unsigned int PrRenderer::OpenGL::TextureFormatToDataTypeGL(Resources::TextureFormat p_format)
{
	switch (p_format)
	{
	case Resources::TextureFormat::R8:
	case Resources::TextureFormat::RG16:
	case Resources::TextureFormat::RGB24:
	case Resources::TextureFormat::RGBA32:
	case Resources::TextureFormat::Depth16:
	case Resources::TextureFormat::Depth24:
	case Resources::TextureFormat::Depth32:
	case Resources::TextureFormat::Depth24Stencil8:
	case Resources::TextureFormat::Depth32Stencil8:
	case Resources::TextureFormat::Stencil8:
		return GL_UNSIGNED_BYTE;
		break;
	case Resources::TextureFormat::RGB16F:
	case Resources::TextureFormat::RGBA16F:
		return GL_FLOAT;
		break;
	default:
		return 0;
		break;
	}
}

unsigned int PrRenderer::OpenGL::PrimitiveToGL(Core::Primitives p_primitives)
{
	switch (p_primitives)
	{
	case Core::Primitives::Points:
		return GL_POINTS;
		break;
	case Core::Primitives::LineStrip:
		return GL_LINE_STRIP;
		break;
	case Core::Primitives::TriangleStrip:
		return GL_TRIANGLE_STRIP;
		break;
	case Core::Primitives::Triangles:
		return GL_TRIANGLES;
		break;
	default:
		break;
	}
}

unsigned int PrRenderer::OpenGL::ComparaisonToGL(Core::ComparaisonAlgorithm p_algorythm)
{
	switch (p_algorythm)
	{
	case Core::ComparaisonAlgorithm::Never:
		return GL_NEVER;
		break;
	case Core::ComparaisonAlgorithm::Less:
		return GL_LESS;
		break;
	case Core::ComparaisonAlgorithm::Equal:
		return GL_EQUAL;
		break;
	case Core::ComparaisonAlgorithm::LessEqual:
		return GL_LEQUAL;
		break;
	case Core::ComparaisonAlgorithm::Greater:
		return GL_GREATER;
		break;
	case Core::ComparaisonAlgorithm::NotEqual:
		return GL_NOTEQUAL;
		break;
	case Core::ComparaisonAlgorithm::GreaterEqual:
		return GL_GEQUAL;
		break;
	case Core::ComparaisonAlgorithm::Always:
		return GL_ALWAYS;
		break;
	default:
		break;
	}
}

unsigned int PrRenderer::OpenGL::BlendingToGL(Core::BlendingAlgorithm p_algorythm)
{
	switch(p_algorythm)
	{
	case Core::BlendingAlgorithm::Zero: 
		return GL_ZERO;
	case Core::BlendingAlgorithm::One:
		return GL_ONE;
	case Core::BlendingAlgorithm::SourceColor:
		return GL_SRC_COLOR;
	case Core::BlendingAlgorithm::OneMinusSorceColor:
		return GL_ONE_MINUS_SRC_COLOR;
	case Core::BlendingAlgorithm::DestinationColor:
		return GL_DST_COLOR;
	case Core::BlendingAlgorithm::OneMinusDestinationColor:
		return GL_ONE_MINUS_DST_COLOR;
	case Core::BlendingAlgorithm::SourceAlpha:
		return GL_SRC_ALPHA;
	case Core::BlendingAlgorithm::OneMinusSourceAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case Core::BlendingAlgorithm::DestinationAlpha:
		return GL_DST_ALPHA;
	case Core::BlendingAlgorithm::OneMinusDestinationAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	default:
		return -1;
	}
}

unsigned int PrRenderer::OpenGL::CullFaceToGL(Core::CullFaceMode p_mode)
{
	switch (p_mode)
	{
	case Core::CullFaceMode::Back:
		return GL_BACK;
	case Core::CullFaceMode::Front:
		return GL_FRONT;
	case Core::CullFaceMode::BackAndFront:
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
