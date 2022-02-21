#vertex

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV0;
layout (location = 2) in vec2 aUV1;

uniform mat4 MVP;

uniform vec2 mainTex_scale;
uniform vec2 mainTex_offset;

uniform vec2 normalTex_scale;
uniform vec2 normalTex_offset;

out vec2 UV0;
out vec2 UV1;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
    UV0 = aUV0 * mainTex_scale + mainTex_offset;
    UV1 = aUV1 * normalTex_scale + normalTex_offset;
}

#fragment

#version 330 core
out vec4 FragColor;

in vec2 UV0;
in vec2 UV1;

uniform vec4 _color;
uniform sampler2D mainTex;
uniform sampler2D normalTex;

void main()
{
    vec4 tex0Color = texture(mainTex, UV0);
    vec4 tex1Color = texture(normalTex, UV1);
    FragColor = tex0Color * _color;
} 