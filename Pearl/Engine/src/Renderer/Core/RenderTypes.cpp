#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/RenderTypes.h"

using namespace PrRenderer::Core;

SortingHash::SortingHash(const RenderObject& p_renderObject)
{
	size_t materialHash = std::hash<Resources::MaterialPtr>{}(p_renderObject.material);
	size_t renderOrder = p_renderObject.material->GetRenderOrder();

	m_hash = materialHash << 32 | renderOrder << 24;
}