#pragma once

#include "Core/Resources/IResourceDataLoader.h"
#include "Renderer/Resources/Mesh.h"

namespace PrRenderer {

	class MeshOBJLoader : public PrCore::IResourceDataLoader {
	public:
		PrCore::IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(PrCore::IResourceDataPtr p_resourceData) override;

		bool SaveResourceOnDisc(PrCore::IResourceDataPtr p_resourceData, const std::string& p_path) override;
	};
}
