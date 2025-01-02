#vertex
#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec4 aTangents;
layout (location = 3) in vec2 aUV0;

out vec2 uv0;

void main()
{
    //UVs calculations
    uv0 = aUV0;

    //Pass vertex to fs
    gl_Position = vec4(aPos, 1.0);
}

//---------------------------------------------------
#fragment
#version 450 core

in vec2 uv0;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D positionMap;
uniform float densityFactor;
uniform float maxDistance;
uniform vec3  fogColor = vec3(0.5f, 0.5f, 0.5f);

void main()
{
    vec4 mapSampled = texture(positionMap, uv0);
    vec3 pos = mapSampled.xyz;
    float linearDepth = mapSampled.w;

    float distanceRatio = (linearDepth)/ maxDistance;
    float fogFactor = exp(-distanceRatio * distanceRatio * densityFactor * densityFactor);

    vec3 color = texture(screenTexture, uv0).rgb;
    FragColor = vec4(mix(fogColor, color, fogFactor), 1.0f);
}