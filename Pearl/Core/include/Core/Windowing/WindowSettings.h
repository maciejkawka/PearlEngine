#include<string>

namespace PrCore::Windowing {
	
	struct WindowSettings {

		std::string title = "Pearl Window";

		int width = 1280;

		int height = 720;

		bool fullScreen = false;

		bool resizable = false;

		//CoursorMode

		bool vSync = true;

		bool decorated = true;

	};
}