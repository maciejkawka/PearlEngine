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
		{}

		inline void SetFace(int p_index, const std::string& p_name) { m_facesNames[p_index] = p_name; }
		inline void Faces(std::vector<std::string> p_names) { m_facesNames = p_names; }

		inline const std::vector<std::string>& GetFaces() { return m_facesNames; }
		inline const std::string& GetFace(int p_index) { return m_facesNames[p_index]; }

		//To implement in future
		//void SetPixel(unsigned int p_u, unsigned int p_v);
		//Core::Color GetPixel(unsigned int p_u, unsigned int p_v) const;

		//void SetPixels(const Core::Color p_colorArray[]);
		//const Core::Color* GetPixels();

	protected:
		std::vector<std::string> m_facesNames;
		unsigned char** m_rawDataArray;
	};

	typedef std::shared_ptr<Cubemap> CubemapPtr;
}