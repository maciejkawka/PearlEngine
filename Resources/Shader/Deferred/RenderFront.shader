#vertex
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec4 aTangents;
layout (location = 3) in vec2 aUV0;

out vec2 TexCoords;

void main()
{
    TexCoords = aUV0;
    gl_Position = vec4(aPos, 1.0);
}


#fragment
#version 450 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D lightTex;

void main()
{
    vec3 color = texture(lightTex, TexCoords).rgb;

    FragColor = vec4(color, 1.0f);
}
