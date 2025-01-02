#include"Core/Common/pearl_pch.h"

#include"Core/Entry/AppContext.h"
#include"Core/Events/EventManager.h"
#include"Core/Utils/Logger.h"
#include"Core/Utils/Clock.h"
#include"Core/Utils/PathUtils.h"
#include "Core/File/ConfigFile.h"
#include "Core/File/FileSystem.h"
#include "Core/ECS/SceneManager.h"

#include"Renderer/Core/DeferRenderFrontend.h"
#include"Renderer/Core/RenderSystem.h"
#include"Renderer/OpenGL/GLContext.h"

#include "Core/Resources/ResourceDatabase.h"

#include "Core/Resources/ResourceSystem.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Cubemap.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Texture.h"

#include "Renderer/Resources/BasicCubemapLoader.h"
#include "Renderer/Resources/HdrCubemapLoader.h"
#include "Renderer/Resources/MaterialLoader.h"
#include "Renderer/Resources/MeshOBJLoader.h"
#include "Renderer/Resources/ShaderLoader.h"
#include "Renderer/Resources/Texture2DLoader.h"
//

const std::string_view GraphicConfig{ "config/graphic.cfg" };
const std::string_view RendererConfig{ "config/renderer.cfg" };

PrCore::Entry::AppContext::AppContext()
{
	m_window = nullptr;
	m_rendererContext = nullptr;

	Utils::Logger::Init();
	Utils::Clock::Init();
	PRLOG_INFO("Building AppContext");

	//Init Engine Subsystems

	//-----------------------
	// Init File System
	std::string_view engineAssetsPath = "EngineAssets";
	std::string_view gameAssetPath = "GameAssets";

	File::FileSystem::Init();
	auto filePtr = File::FileSystem::GetInstancePtr();

	auto binView = filePtr->GetExecutablePath();
	auto sanitized = PrCore::PathUtils::Sanitize(binView.data());

	auto rootEngine = PrCore::PathUtils::RemoveSubFolderInPlace(sanitized, 2);
	auto engineAssets = PrCore::PathUtils::MakePath(rootEngine, engineAssetsPath);
	filePtr->SetEngineRoot(rootEngine);
	filePtr->SetEngineAssetsPath(engineAssets);
	filePtr->MountDir(engineAssets);

	// This is going to be moved to PrGame system
	auto gameAssets = PrCore::PathUtils::MakePath(rootEngine, gameAssetPath);
	filePtr->SetGameAssetsPath(gameAssets);
	filePtr->MountDir(gameAssets);
	filePtr->SetWriteDir(gameAssets);
	//-----------------------

	Events::EventManager::Init();
	Resources::ResourceSystem::Init();

	//-----------------------
	// Init Resource System
	{
		using namespace PrRenderer::Resources;
		auto textureDatabase = std::make_unique<ResourceDatabase>();
		textureDatabase->RegisterLoader(".png", std::make_unique<Texture2DLoader>());
		textureDatabase->RegisterLoader(".jpg", std::make_unique<Texture2DLoader>());
		textureDatabase->RegisterLoader(".tga", std::make_unique<Texture2DLoader>());
		textureDatabase->RegisterLoader(".hdr", std::make_unique<Texture2DLoader>());
		ResourceSystem::GetInstance().RegisterDatabase<Texture>(std::move(textureDatabase));

		auto materialLoader = std::make_unique<ResourceDatabase>();
		materialLoader->RegisterLoader(".mat", std::make_unique<MaterialLoader>());
		ResourceSystem::GetInstance().RegisterDatabase<Material>(std::move(materialLoader));

		auto shaderLoader = std::make_unique<ResourceDatabase>();
		shaderLoader->RegisterLoader(".shader", std::make_unique<ShaderLoader>());
		ResourceSystem::GetInstance().RegisterDatabase<Shader>(std::move(shaderLoader));

		auto meshLoader = std::make_unique<ResourceDatabase>();
		meshLoader->RegisterLoader(".obj", std::make_unique<MeshOBJLoader>());
		ResourceSystem::GetInstance().RegisterDatabase<Mesh>(std::move(meshLoader));

		auto cubemapDatabase = std::make_unique<ResourceDatabase>();
		cubemapDatabase->RegisterLoader(".cubemap", std::make_unique<BasicCubemapLoader>());
		cubemapDatabase->RegisterLoader(".hdr", std::make_unique<HdrCubemapLoader>());
		ResourceSystem::GetInstance().RegisterDatabase<Cubemap>(std::move(cubemapDatabase));
	}
	//-----------------------

	File::ConfigFile contexConfig;
	if (contexConfig.OpenFromFile(GraphicConfig))
	{
		Windowing::WindowContext context;
		context.debugMode = contexConfig.GetSetting<bool>("debugMode");
		context.forwardCompatibility = contexConfig.GetSetting<bool>("forwardCompatibility");
		context.multiSampling = contexConfig.GetSetting("multiSampling");
		context.versionMinor = contexConfig.GetSetting("versionMinor");
		context.versionMajor = contexConfig.GetSetting("versionMajor");
		Windowing::GLWindow::InitDevice(context);


		Windowing::WindowSettings windowSettings;
		windowSettings.title = contexConfig.GetSetting<std::string>("title");
		windowSettings.height = contexConfig.GetSetting("height");
		windowSettings.width = contexConfig.GetSetting("width");
		windowSettings.fullScreen = contexConfig.GetSetting<bool>("fullScreen");
		windowSettings.resizable = contexConfig.GetSetting<bool>("resizable");
		windowSettings.decorated = contexConfig.GetSetting<bool>("decorated");
		windowSettings.vSync = contexConfig.GetSetting<bool>("vSync");
		windowSettings.iconPath = contexConfig.GetSetting<std::string>("iconPath");
		m_window = new Windowing::GLWindow(windowSettings);
	}

	m_rendererContext = new PrRenderer::OpenGL::GLContext();
	m_rendererContext->Init();

	File::ConfigFile rendererConfig;
	if (rendererConfig.OpenFromFile(RendererConfig))
	{
		PrRenderer::Core::RendererSettings rendererSettings;
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

		PrRenderer::Core::renderSystem = std::make_unique<PrRenderer::Core::DeferRenderFrontend>(rendererSettings);
	}

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
	Resources::ResourceSystem::Terminate();
	Events::EventManager::Terminate();
	File::FileSystem::Terminate();
	Utils::Clock::Terminate();
}
