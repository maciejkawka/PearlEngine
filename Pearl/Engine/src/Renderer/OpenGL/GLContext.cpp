#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/LowRenderer.h"
#include"Renderer/OpenGL/GLContext.h"
#include"Core/Utils/Logger.h"

#include"glad/glad.h"
#include"GLFW/glfw3.h"

using namespace PrRenderer::OpenGL;

GLContext::GLContext(const ContextSettings& p_contextSettings)
{
}

GLContext::~GLContext()
{
    LowRenderer::Terminate();
}

void GLContext::Init()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        PRLOG_ERROR("Renderer: Cannot Init OpenGL!");
        m_isActive = false;
        return;       
    }

    PRLOG_INFO("Renderer: GLContext Init!");
    PRLOG_INFO("OpenGL:\n Vendor: {0}\n Renderer: {1}\n Version: {2}", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
    
    m_isActive = true;
    LowRenderer::Init(GraphicsAPI::OpenGL);
}