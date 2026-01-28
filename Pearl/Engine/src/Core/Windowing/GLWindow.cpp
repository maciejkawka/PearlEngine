#include "Core/Common/pearl_pch.h"
		 
#include "Core/Windowing/GLWindow.h"
#include "GLFW/glfw3.h"
#include "Core/Utils/Logger.h"
#include "Core/Events/EventManager.h"
#include "Core/Events/WindowEvents.h"
#include "Core/Events/InputEvents.h"
		 
#include "Core/File/FileSystem.h"
#include "stb/stb_image.h"

using namespace PrCore;

bool GLWindow::InitDevice(const WindowContext& p_context)
{
	const bool init = glfwInit();
	if (!init)
	{
		PRLOG_ERROR("Filed to initaite GLFW");
		glfwTerminate();
		return false;
	}

	PRLOG_INFO("Init GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, p_context.versionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, p_context.versionMinor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, p_context.forwardCompatibility);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, p_context.debugMode);
	glfwWindowHint(GLFW_SAMPLES, p_context.multiSampling);

	m_context = p_context;
	return true;
}

void GLWindow::TerminateDevice()
{
	glfwTerminate();
	PRLOG_INFO("Terminate GLFW");
}

void GLWindow::PollEvents()
{
	glfwPollEvents();
}

void GLWindow::SwapBuffers()
{
	glfwSwapBuffers(m_window);
}

GLWindow::GLWindow(const WindowContext& p_context, const WindowSettings& p_settings) :
	m_window(nullptr)
{
	if (!InitDevice(p_context))
	{
		PRLOG_ERROR("Cannot create a window, GLFW not init!");
		return;
	}

	m_settings = p_settings;
	//Create window
	glfwWindowHint(GLFW_RESIZABLE, m_settings.resizable);
	glfwWindowHint(GLFW_DECORATED, m_settings.decorated);

	GLFWmonitor* primaryMonitor = nullptr;
	if (m_settings.fullScreen)
		primaryMonitor = glfwGetPrimaryMonitor();

	m_window = glfwCreateWindow(m_settings.width, m_settings.height,
		m_settings.title.c_str(), primaryMonitor, nullptr);

	if (!m_window)
	{
		PRLOG_ERROR("Cannot create window!");
		return;
	}

	PRLOG_INFO("Window created Title: \"{0}\", Width: {1}, Height: {2}", m_settings.title, m_settings.width, m_settings.height);
	if (m_settings.vSync)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);

	glfwSetWindowUserPointer(m_window, &m_settings);
	BindCallbacks();
	glfwMakeContextCurrent(m_window);

	if (!m_settings.iconPath.empty())
		SetIcon(std::string{ PrSystems::Get<FileSystem>()->GetEngineAssetsPath() } + m_settings.iconPath);
}

GLWindow::~GLWindow()
{
	TerminateDevice();
	glfwDestroyWindow(m_window);

	PRLOG_INFO("Deleting Window");
}

bool GLWindow::ShouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void GLWindow::SetVSync(bool p_vsync)
{
	if (p_vsync)
		glfwSwapInterval(1);
	else
		glfwSwapBuffers(0);

	m_settings.vSync = p_vsync;
}

void GLWindow::SetIcon(std::string p_path)
{
	GLFWimage images[1];
	images[0].pixels = stbi_load(p_path.c_str(), &images[0].width, &images[0].height, 0, 4);

	if (!images[0].pixels)
	{
		PRLOG_ERROR("Window icon not found");
		return;
	}

	glfwSetWindowIcon(m_window, 1, images);
	stbi_image_free(images[0].pixels);
}

void GLWindow::BindCallbacks()
{
	//Close Window Callback
	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* p_window)
		{
			EventPtr event = std::make_shared<WindowCloseEvent>();
			PrSystems::Get<EventManager>()->QueueEvent(event);
		});

	//Resize Window Callback
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* p_window, int p_width, int p_height)
		{
			WindowSettings* windowSettings = (WindowSettings*)glfwGetWindowUserPointer(p_window);
			windowSettings->width = p_width;
			windowSettings->height = p_height;

			EventPtr event = std::make_shared<WindowResizeEvent>(p_width, p_height);
			PrSystems::Get<EventManager>()->QueueEvent(event);
		});

	//Minimalize Window Callback
	glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* p_window, int p_iconified)
		{

			EventPtr event = std::make_shared<WindowMinimalizeEvent>(p_iconified);
			PrSystems::Get<EventManager>()->QueueEvent(event);
		});


	//Key State Callback
	glfwSetKeyCallback(m_window, [](GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods)
		{
			if (p_action == GLFW_PRESS)
			{
				EventPtr event = std::make_shared<KeyPressedEvent>(p_key);
				PrSystems::Get<EventManager>()->QueueEvent(event);
			}
			else if (p_action == GLFW_RELEASE)
			{
				EventPtr event = std::make_shared<KeyReleasedEvent>(p_key);
				PrSystems::Get<EventManager>()->QueueEvent(event);
			}
		});

	//Mouse Button State Callback
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* p_window, int p_button, int p_action, int p_mods)
		{
			if (p_action == GLFW_PRESS)
			{
				EventPtr event = std::make_shared<MouseButtonPressedEvent>(p_button);
				PrSystems::Get<EventManager>()->QueueEvent(event);
			}
			else if (p_action == GLFW_RELEASE)
			{
				EventPtr event = std::make_shared<MouseButtonReleasedEvent>(p_button);
				PrSystems::Get<EventManager>()->QueueEvent(event);
			}
		});

	//Mouse Position Callback
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* p_window, double p_xPos, double p_yPos)
		{
			EventPtr event = std::make_shared<MouseMovedEvent>(p_xPos, p_yPos);
			PrSystems::Get<EventManager>()->QueueEvent(event);
		});

	glfwSetScrollCallback(m_window, [](GLFWwindow* p_window, double p_xOffset, double p_yOffset)
		{
			EventPtr event = std::make_shared<MouseScrollEvent>(p_xOffset, p_yOffset);
			PrSystems::Get<EventManager>()->QueueEvent(event);
		});
}
