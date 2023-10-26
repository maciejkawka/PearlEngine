#include"Core/Common/pearl_pch.h"

#include"Core/Entry/AppContext.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"
#include"Core/Filesystem/FileSystem.h"
#include"Core/Filesystem/ConfigFile.h"
#include"Core/Resources/ResourceLoader.h"
#include"Core/ECS/SceneManager.h"
#include"Renderer/OpenGL/GLContext.h"
#include"Renderer/Core/DeferredRendererFrontend.h"
#include"Renderer/Core/DefRendererBackend.h"


PrCore::Entry::AppContext::AppContext()
{
	Utils::Logger::Init();
	PRLOG_INFO("Building AppContext");

	//Init Engine Subsystems
	Filesystem::FileSystem::Init();
	Events::EventManager::Init();

	Resources::ResourceLoader::Init();

	Filesystem::ConfigFile contexConfig("graphic.cfg");	
	Windowing::WindowContext context;
	if (contexConfig.isValid())
	{
		context.debugMode = contexConfig.GetSetting<bool>("debugMode");
		context.forwardCompatibility = contexConfig.GetSetting<bool>("forwardCompatibility");
		context.multiSampling = contexConfig.GetSetting("multiSampling");
		context.versionMinor = contexConfig.GetSetting("versionMinor");
		context.versionMajor = contexConfig.GetSetting("versionMajor");
	}

	Windowing::GLWindow::InitDevice(context);
	
	Windowing::WindowSettings windowSettings;
	if (contexConfig.isValid())
	{
		windowSettings.title = contexConfig.GetSetting<std::string>("title");
		windowSettings.height = contexConfig.GetSetting("height");
		windowSettings.width = contexConfig.GetSetting("width");
		windowSettings.fullScreen = contexConfig.GetSetting<bool>("fullScreen");
		windowSettings.resizable = contexConfig.GetSetting<bool>("resizable");
		windowSettings.decorated = contexConfig.GetSetting<bool>("decorated");
		windowSettings.vSync = contexConfig.GetSetting<bool>("vSync");
		windowSettings.iconPath = contexConfig.GetSetting<std::string>("iconPath");
	}

	m_window = new Windowing::GLWindow(windowSettings);

	m_rendererContext = new PrRenderer::OpenGL::GLContext();
	m_rendererContext->Init();

	PrRenderer::Core::DefferedRendererFrontend::Init(PrRenderer::Core::RendererSettings());
	m_rendererBackend = PrRenderer::Core::DefferedRendererFrontend::GetInstance().GetRendererBackend();

	Input::InputManager::Init();

	ECS::SceneManager::Init();
}

PrCore::Entry::AppContext::~AppContext()
{
	PRLOG_INFO("Deleting AppContext");

	ECS::SceneManager::Terminate();
	Input::InputManager::Terminate();
	PrRenderer::Core::DefferedRendererFrontend::Terminate();
	delete m_rendererContext;
	delete m_window;
	Windowing::GLWindow::TerminateDevice();
	Resources::ResourceLoader::Terminate();
	Events::EventManager::Terminate();
	Filesystem::FileSystem::Terminate();
}
