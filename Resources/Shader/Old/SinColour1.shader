#vertex

#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aColour;
out vec4 vertexColor; // specify a color output to the fragment shader

uniform float sinColR = 0.0;
uniform float sinColG = 0.0;
uniform float sinColB = 0.0;


void main()
{
    gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    vertexColor = vec4(sinColR, sinColG , sinColB , 1.0);  
}

#fragment

#version 330 core
out vec4 FragColor;
  
in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

void main()
{
    FragColor = vertexColor;
} 