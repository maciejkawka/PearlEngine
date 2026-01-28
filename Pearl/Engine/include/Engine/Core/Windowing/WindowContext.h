#pragma once

namespace PrCore {

	struct WindowContext {

		int dontCare = -1;

		int versionMajor = 3;

		int versionMinor = 2;

		bool forwardCompatibility = false;

		bool debugMode = false;

		int multiSampling = dontCare;
	};

}