#pragma once
#include"Renderer/Resources/Texture.h"

#include<vector>

namespace PrRenderer::Resources{

	class Cubemap: public Texture{
	public:
		Cubemap() = default;

		//Constructor for managed resource
		Cubemap(std::string p_name, PrCore::Resources::ResourceHandle p_handle):
			Texture(p_name, p_handle),
		m_rawDataArray(nullptr)
		{
			m_wrapU = Resources::TextureWrapMode::Clamp;
			m_wrapV = Resources::TextureWrapMode::Clamp;
			m_wrapR = Resources::TextureWrapMode::Clamp;
			m_magFiltering = Resources::TextureFiltering::Linear;
			m_minFiltering = Resources::TextureFiltering::Linear;
		}

		inline void SetFace(int p_index, const std::string& p_name) { m_facesNames[p_index] = p_name; }
		inline void Faces(std::vector<std::string> p_names) { m_facesNames = p_names; }

		inline const std::vector<std::string>& GetFaces() { return m_facesNames; }
		inline const std::string& GetFace(int p_index) { return m_facesNames[p_index]; }

		virtual void SetWrapModeR(Resources::TextureWrapMode p_wrapR) = 0;
		TextureWrapMode GetWrapModeR() { return m_wrapR; }

		//To implement in future
		//void SetPixel(unsigned int p_u, unsigned int p_v);
		//Core::Color GetPixel(unsigned int p_u, unsigned int p_v) const;

		//void SetPixels(const Core::Color p_colorArray[]);
		//const Core::Color* GetPixels();

	protected:
		unsigned char** m_rawDataArray;
		std::vector<std::string> m_facesNames;
		TextureWrapMode m_wrapR;
	};

	typedef std::shared_ptr<Cubemap> CubemapPtr;
}