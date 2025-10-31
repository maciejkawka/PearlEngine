#pragma once

#include "Core/ECS/Scene.h"

namespace PrEditor::Assets
{
	class SceneExporter {
	public:

		// Saves all resources created in memory to files. 
		void SaveMemoryResourcesToFile(std::string_view exportRoot);

		// Runtime Exporter maybe later I really want to export to optimized files 
		// Prmat, Prtex, Prmesh - Loaded directly into memory without the parsing overhead.
		// But that later
		// void RuntimeExport(std::string_view exportRoot);
	};
}