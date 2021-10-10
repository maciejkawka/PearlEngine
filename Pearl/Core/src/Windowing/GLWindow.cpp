#include"Core/Common/pearl_pch.h"

#include"Core/Windowing/GLWindow.h"
#include"GLFW/glfw3.h"
#include"Core/Utils/Logger.h"

using namespace PrCore::Windowing;

WindowContext GLWindow::s_context;
bool GLWindow::s_init = false;
int GLWindow::s_windowsCount = 0;

void GLWindow::InitDevice(const WindowContext& p_context)
{
	if (s_init)
	{
		PRLOG_WARN("Window Context already initiated!");
		return;
	}

	s_init = glfwInit();
	if (!s_init)
	{
		PRLOG_ERROR("Filed to initaite GLFW");
		glfwTerminate();
		return;
	}

	PRLOG_INFO("Init GLFW!");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, p_context.versionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, p_context.varsionMinor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, p_context.forwardCompatibility);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, p_context.debugMode);
	glfwWindowHint(GLFW_SAMPLES, p_context.multiSampling);

	s_context = p_context;
}

void GLWindow::TerminateDevice()
{
	if (s_windowsCount > 0)
		PRLOG_WARN("Not all windows were closed!");
	glfwTerminate();
	PRLOG_INFO("Terminate GLFW!");
}

void GLWindow::PollEvents()
{
	glfwPollEvents();
}

void PrCore::Windowing::GLWindow::SwapBuffers()
{
	glfwSwapBuffers(m_window);
}

GLWindow::GLWindow(const WindowSettings& p_settings) :
	m_window(nullptr)
{
	if (!s_init)
	{
		PRLOG_ERROR("Cannot create a window, GLFW not init!");
		PRLOG_ERRORF("Cannot create a window, GLFW not init!");
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
		PRLOG_ERRORF("Cannot create window!");
		return;
	}

	PRLOG_INFO("Window created Title: \"{0}\", Width: {1}, Height: {2}", m_settings.title, m_settings.width, m_settings.height);
	if (m_settings.vSync)
		glfwSwapInterval(1);
	else
		glfwSwapBuffers(0);

	s_windowsCount++;
	//BindCallbacks();
}

GLWindow::~GLWindow()
{
	glfwDestroyWindow(m_window);
	s_windowsCount--;

	PRLOG_INFO("Deleting Window!");
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
	//TODO
	//GLFWimage images;
	//images[0] = load_icon("my_icon.png");

	//glfwSetWindowIcon(window, 2, images);
}
