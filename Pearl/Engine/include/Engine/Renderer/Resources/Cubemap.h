#pragma once
#include"Renderer/Resources/Texture.h"

#include<vector>

namespace PrRenderer::Resources{

	class Cubemap: public Texture{
	public:
		Cubemap():
			Texture(),
			m_wrapR(TextureWrapMode::Clamp),
			m_rawDataArray(nullptr)
		{}

		//Constructor for managed resource
		Cubemap(std::string p_name, PrCore::Resources::ResourceHandle p_handle):
			Texture(p_name, p_handle),
		m_rawDataArray(nullptr)
		{
			m_wrapU = TextureWrapMode::Clamp;
			m_wrapV = TextureWrapMode::Clamp;
			m_wrapR = TextureWrapMode::Clamp;
			m_magFiltering = TextureFiltering::Linear;
			m_minFiltering = TextureFiltering::Linear;
		}

		inline void SetFace(int p_index, const std::string& p_name) { m_facesNames[p_index] = p_name; }
		inline void Faces(std::vector<std::string> p_names) { m_facesNames = p_names; }

		inline const std::vector<std::string>& GetFaces() { return m_facesNames; }
		inline const std::string& GetFace(int p_index) { return m_facesNames[p_index]; }

		virtual void SetWrapModeR(TextureWrapMode p_wrapR) = 0;
		TextureWrapMode GetWrapModeR() { return m_wrapR; }

		static TexturePtr GenerateBlackTexture();
		static TexturePtr GenerateWhiteTexture();
		static TexturePtr GenerateRedTexture();

		//To implement in future
		//void SetPixel(unsigned int p_u, unsigned int p_v);
		//Core::Color GetPixel(unsigned int p_u, unsigned int p_v) const;

		//void SetPixels(const Core::Color p_colorArray[]);
		//const Core::Color* GetPixels();

	protected:
		unsigned char** m_rawDataArray;
		std::vector<std::string> m_facesNames;
		TextureWrapMode m_wrapR;

	private:
		static TexturePtr GenerateUnitTexture(Core::Color p_color);
	};

	typedef std::shared_ptr<Cubemap> CubemapPtr;
}