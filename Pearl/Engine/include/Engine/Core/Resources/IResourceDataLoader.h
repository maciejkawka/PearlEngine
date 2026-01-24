#pragma once

#include "Core/Resources/IResource.h"


namespace PrCore {

	class IResourceDataLoader {
	public:
		IResourceDataLoader() = default;
		virtual ~IResourceDataLoader() = default;

		virtual IResourceDataPtr LoadResource(const std::string& p_path) = 0;
		virtual IResourceDataPtr LoadFromMemoryResource(const void* p_buffer, size_t p_size, int p_flags = 0) { return nullptr; }
		virtual void             UnloadResource(IResourceDataPtr p_resourceData) = 0;

		virtual bool SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path) = 0;

		// virtual ResourcePtr LoadResourceAsync(const std::string& p_path, size_t& p_dataSize) = 0;
		// virtual ResourcePtr UnloadResourceAsync(const std::string& p_path) = 0;
	};
}
