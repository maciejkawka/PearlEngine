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

uniform sampler2D backTex;
uniform sampler2D bloomTex;
uniform float exposure = 1.0f;
uniform bool enableBloom = true;

// Uncharted Tone Mapping
// https://64.github.io/tonemapping/
vec3 Uncharted2ToneMapping(vec3 color)
{
  float A = 0.15f;
  float B = 0.50f;
  float C = 0.10f;
  float D = 0.20f;
  float E = 0.02f;
  float F = 0.30f;
  color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
  return color;
}

// Tone Mapping Epic Games
// https://64.github.io/tonemapping/
vec3 tone_aces_approx(vec3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}

// Tone mapping taken from https://github.com/tgalaj/RapidGL/tree/master
// It makes very bright spots white regardless of the fragment color
// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat =
{
    {0.59719, 0.07600, 0.02840},
    {0.35458, 0.90834, 0.13383},
    {0.04823, 0.01566, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat =
{
    { 1.60475, -0.10208, -0.00327},
    {-0.53108,  1.10813, -0.07276},
    {-0.07367, -0.00605,  1.07602 }
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

void main()
{
    vec3 color = texture(backTex, TexCoords).rgb * exposure;

    if(enableBloom == true)
        color += texture(bloomTex, TexCoords).rgb * 0.5f;
    
    color = ACESInputMat * color;
    color = RRTAndODTFit(color);
    color = ACESOutputMat * color;

    // color = Uncharted2ToneMapping(color * exposure);
    // vec3 white = vec3(1.0f) / Uncharted2ToneMapping(vec3(11.2f));
    // color = color * white;

    color = pow(color, vec3(1.0f / 2.2f));
    color = clamp(color, 0.0, 1.0);
    FragColor = vec4(color, 1.0f);
}
