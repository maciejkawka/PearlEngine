#vertex

#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aUV;

out vec2 UV;


void main()
{
    gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor 
    UV = aUV;
}


#fragment

#version 330 core
out vec4 FragColor;
 
uniform sampler2D u_tex;

in vec2 UV;

void main()
{
    FragColor = texture(u_tex, UV);
} 