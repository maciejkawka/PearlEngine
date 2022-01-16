#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer.h"
#include"Renderer/OpenGL/GLVertexBuffer.h"
#include"Renderer/OpenGL/GLIndexBuffer.h"
#include"Renderer/OpenGL/GLVertexArray.h"

#include"glad/glad.h"

using namespace PrRenderer::Core;

void Renderer::Test()
{
	float vertecies[] = {
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,// bottom left
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f  // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	vertexArray = std::make_shared<OpenGL::GLVertexArray>();

	IndexBufferPtr indexBuffer = std::make_shared<OpenGL::GLIndexBuffer>(indices, 6);
	vertexArray->SetIndexBuffer(indexBuffer);


	Buffers::BufferLayout layout = {
		{ "Vertex", Buffers::ShaderDataType::Float3},
		{ "Colour", Buffers::ShaderDataType::Float3}
	};

	VertexBufferPtr vertexBuffer = std::make_shared<OpenGL::GLVertexBuffer>(vertecies, sizeof(vertecies));
	vertexBuffer->SetBufferLayout(layout);

	vertexArray->SetVertexBuffer(vertexBuffer);
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aCol;\n"
		"out vec3 vertexCol;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"   vertexCol = aCol;\n"
		"}\0";

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in  vec3 vertexCol;\n"
		"void main()\n"
		"{\n"
		"FragColor = vec4(vertexCol,1.0);\n"
		"}\0";

	
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Renderer::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glUseProgram(shaderProgram);
	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
