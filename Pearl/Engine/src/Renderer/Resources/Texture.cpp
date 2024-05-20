#include"Core/Common/pearl_pch.h"

#include"Renderer/Resources/Texture.h"

using namespace PrRenderer::Resources;

void Texture::LoadCorruptedResource()
{
	Core::Color color;
	color.r = 255;
	color.g = 20;
	color.b = 147;
	color.a = 255;

	LoadUnitTexture(color);
}