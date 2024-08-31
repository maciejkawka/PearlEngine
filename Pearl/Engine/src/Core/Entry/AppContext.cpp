#include"Core/Common/pearl_pch.h"

#include"Core/Entry/AppContext.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"
#include"Core/Utils/Clock.h"
#include"Core/Filesystem/FileSystem.h"
#include"Core/Filesystem/ConfigFile.h"
#include"Core/Resources/ResourceLoader.h"
#include"Core/ECS/SceneManager.h"

#include"Renderer/OpenGL/GLContext.h"
#include"Renderer/Core/DeferRenderFrontend.h"
#include"Renderer/Core/RenderSystem.h"

const std::string_view GraphicConfig{ "graphic.cfg" };
const std::string_view RendererConfig{ "renderer.cfg" };

PrCore::Entry::AppContext::AppContext()
{
	Utils::Logger::Init();
	Utils::Clock::Init();
	PRLOG_INFO("Building AppContext");

	//Init Engine Subsystems
	Filesystem::FileSystem::Init();
	Events::EventManager::Init();

	Resources::ResourceLoader::Init();

	Filesystem::ConfigFile contexConfig(GraphicConfig.data());
	Windowing::WindowContext context;
	if (contexConfig.IsValid())
	{
		context.debugMode = contexConfig.GetSetting<bool>("debugMode");
		context.forwardCompatibility = contexConfig.GetSetting<bool>("forwardCompatibility");
		context.multiSampling = contexConfig.GetSetting("multiSampling");
		context.versionMinor = contexConfig.GetSetting("versionMinor");
		context.versionMajor = contexConfig.GetSetting("versionMajor");
	}

	Windowing::GLWindow::InitDevice(context);
	
	Windowing::WindowSettings windowSettings;
	if (contexConfig.IsValid())
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

	PrRenderer::Core::RendererSettings rendererSettings;
	Filesystem::ConfigFile rendererConfig(RendererConfig.data());
	if(rendererConfig.IsValid())
	{
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, dirLightMaxShadows);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, dirLightShadowsMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, dirLightCombineMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, dirLightShadowBias);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, dirLightCascadeExtend);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, dirLightSize);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, pointLightMaxShadows);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, pointLightShadowMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, pointLightCombineShadowMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, pointLightShadowBias);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, pointLightSize);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, spotLightMaxShadows);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, spotLightShadowMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, spotLightCombineShadowMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, spotLightShadowBias);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, spotLightSize);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, mainLightShadowMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, mainLightShadowCombineMapSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, mainLightShadowBias);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, mainLightSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, mainLightBlendDist);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, mainLightCascadeExtend);

		rendererSettings.cascadeShadowBorders[0] = rendererConfig.GetSetting<float>("CSMBorder1");
		rendererSettings.cascadeShadowBorders[1] = rendererConfig.GetSetting<float>("CSMBorder2");
		rendererSettings.cascadeShadowBorders[2] = rendererConfig.GetSetting<float>("CSMBorder3");
		rendererSettings.cascadeShadowBorders[3] = rendererConfig.GetSetting<float>("CSMBorder4");

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, ambientColor);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, ambientIntensity);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, skyColor);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, enableFog);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, fogColor);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, fogDencity);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, fogMaxDistance);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, enableFXAAA);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, FXAAThreasholdMax);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, FXAAThreasholdMin);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, FXAAEdgeIterations);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, FXAASubpixelQuiality);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, enableSSAO);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, SSAOKenrelSize);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, SSAORadius);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, SSAObias);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, SSAOMagnitude);
		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, SSAOBlureSize);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, enableInstancing);

		rendererConfig.GET_CONFIG_SETTING_NAME(rendererSettings, toneMappingExposure);
	}

	PrRenderer::Core::renderSystem = std::make_unique<PrRenderer::Core::DeferRenderFrontend>(rendererSettings);

	Input::InputManager::Init();

	ECS::SceneManager::Init();
}

PrCore::Entry::AppContext::~AppContext()
{
	PRLOG_INFO("Deleting AppContext");

	ECS::SceneManager::Terminate();
	Input::InputManager::Terminate();
	PrRenderer::Core::renderSystem.reset();
	delete m_rendererContext;
	delete m_window;
	Windowing::GLWindow::TerminateDevice();
	Resources::ResourceLoader::Terminate();
	Events::EventManager::Terminate();
	Filesystem::FileSystem::Terminate();
	Utils::Clock::Terminate();
}
