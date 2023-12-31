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

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gAO;

uniform sampler2D ssaoInput;
uniform int kenrelSize = 4;
void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    int halfSize = kenrelSize / 2;
    for (int x = -halfSize; x < halfSize; ++x) 
    {
        for (int y = -halfSize; y < halfSize; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, uv0 + offset).r;
        }
    }

    float aoResult = result / (kenrelSize * kenrelSize);
    gAO = vec4(vec3(aoResult, 0, 0), aoResult);
}  