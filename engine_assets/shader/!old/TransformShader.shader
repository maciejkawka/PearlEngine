#vertex

#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aColour;
out vec4 vertexColor; // specify a color output to the fragment shader

uniform float test = 0.1;
uniform float test1 = 0.4;
uniform float test2 = 0.5;
uniform mat4 u_modelMat;

void main()
{
    gl_Position = u_modelMat * vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    vertexColor = vec4(aColour, test + test1 + test2); 
}

#fragment

#version 330 core
out vec4 FragColor;
  
in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

void main()
{
    FragColor = vertexColor;
} 