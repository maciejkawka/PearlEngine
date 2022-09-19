#include"Core/Common/pearl_pch.h"

#include"Core/Entry/AppContext.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"
#include"Core/Filesystem/FileSystem.h"
#include"Core/Filesystem/ConfigFile.h"
#include"Core/Resources/ResourceLoader.h"
#include"Core/ECS/SceneManager.h"
#include"Renderer/OpenGL/GLContext.h"


PrCore::Entry::AppContext::AppContext()
{
	PrCore::Utils::Logger::Init();
	PRLOG_INFO("Building AppContext");

	//Init Engine Subsystems
	PrCore::Filesystem::FileSystem::Init();
	PrCore::Events::EventManager::Init();

	PrCore::Resources::ResourceLoader::GetInstance();

	PrCore::Filesystem::ConfigFile contexConfig("graphic.cfg");	
	PrCore::Windowing::WindowContext context;
	if (contexConfig.isValid())
	{
		context.debugMode = contexConfig.GetSetting<bool>("debugMode");
		context.forwardCompatibility = contexConfig.GetSetting<bool>("forwardCompatibility");
		context.multiSampling = contexConfig.GetSetting("multiSampling");
		context.versionMinor = contexConfig.GetSetting("versionMinor");
		context.versionMajor = contexConfig.GetSetting("versionMajor");
	}

	PrCore::Windowing::GLWindow::InitDevice(context);
	
	PrCore::Windowing::WindowSettings windowSettings;
	if (contexConfig.isValid())
	{
		windowSettings.title = contexConfig.GetSetting<std::string>("title");
		windowSettings.height = contexConfig.GetSetting("height");
		windowSettings.width = contexConfig.GetSetting("width");
		windowSettings.fullScreen = contexConfig.GetSetting<bool>("fullScreen");
		windowSettings.resizable = contexConfig.GetSetting<bool>("resizable");
		windowSettings.decorated = contexConfig.GetSetting<bool>("decorated");
		windowSettings.vSync = contexConfig.GetSetting<bool>("vSync");
	}

	m_window = new PrCore::Windowing::GLWindow(windowSettings);

	m_rendererContext = new PrRenderer::OpenGL::GLContext();
	m_rendererContext->Init();

	m_renderer3D = new PrRenderer::Core::Renderer3D();

	m_input = new PrCore::Input::InputManager();

	ECS::SceneManager::Init();
}

PrCore::Entry::AppContext::~AppContext()
{
	PRLOG_INFO("Deleting AppContext");

	ECS::SceneManager::Terminate();
	delete m_input;
	delete m_renderer3D;
	delete m_rendererContext;
	delete m_window;
	PrCore::Windowing::GLWindow::TerminateDevice();
	PrCore::Resources::ResourceLoader::Terminate();
	PrCore::Events::EventManager::Terminate();
	PrCore::Filesystem::FileSystem::Terminate();
}
