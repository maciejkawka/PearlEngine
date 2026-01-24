#pragma once

#include "Core/Resources/IResourceDataLoader.h"

namespace PrPhysics {

	class ConvexMeshLoader : public PrCore::IResourceDataLoader {
	public:
		PrCore::IResourceDataPtr LoadResource(const std::string& p_path) override;

		void UnloadResource(PrCore::IResourceDataPtr p_resourceData) override;

		bool SaveResourceOnDisc(PrCore::IResourceDataPtr p_resourceData, const std::string& p_path) override;
	};
}