namespace PrCore::Windowing{

	struct WindowContext {

		int dontCare = -1;

		int versionMajor = 3;

		int varsionMinor = 2;

		bool forwardCompatibility = false;

		bool debugMode = false;

		int multiSampling = dontCare;
	};

}