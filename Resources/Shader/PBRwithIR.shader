#vertex

#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec4 aTangents;
layout (location = 3) in vec2 aUV0;

uniform mat4 VPMatrix;
uniform mat4 modelMatrix;

uniform vec2 mainTex_scale = vec2(1,1);
uniform vec2 mainTex_offset = vec2(0, 0);

out Vertex{
    vec3 pos;
    vec3 normals;
    vec2 uv0;
} OUT;


void main()
{
    OUT.pos = vec3(modelMatrix * vec4(aPos,1.0));
    OUT.normals = mat3(modelMatrix) * aNormals;
    OUT.uv0 = aUV0 * mainTex_scale + mainTex_offset;

    gl_Position = VPMatrix * vec4(OUT.pos,1.0);
}

#fragment

#version 450 core

out vec4 FragColor;

in Vertex{
    vec3 pos;
    vec3 normals;
    vec2 uv0;
} IN;

#define MAX_LIGHT_NUM 4
const float PI = 3.14159265359;

// material textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform samplerCube irradianceMap;

// lights
uniform mat4 lightMat[4];
uniform int lightNumber = 0;
uniform vec3 ambientColor;

uniform vec3 camPos;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, IN.uv0).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(IN.pos);
    vec3 Q2  = dFdy(IN.pos);
    vec2 st1 = dFdx(IN.uv0);
    vec2 st2 = dFdy(IN.uv0);

    vec3 N   = normalize(IN.normals);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main()
{
    vec3 albedo = texture(albedoMap, IN.uv0).rgb;
    float metallic = texture(metallicMap, IN.uv0).r;
    float roughness = texture(roughnessMap, IN.uv0).r;
    float ao = texture(aoMap, IN.uv0).r;

    //  vec3 albedo = vec3(1.0,0.0,0.0);    
    //  float metallic = 0.4;
    //  float roughness = 0.1;
    //  float ao = 1.0;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - IN.pos);
    vec3 R = reflect(-V,N);


    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    int clampedLightNumber = clamp(lightNumber, 0, 4);
    for (int i = 0; i < clampedLightNumber; i++)
    {
        mat4 light = lightMat[i];
        vec3 lightPos = light[0].xyz;
        vec3 lightColor = vec3(light[2].xyz);

        vec3 L = normalize(lightPos - IN.pos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos - IN.pos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColor * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot( N, L),0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }


    //IR ambient
    vec3 kS = fresnelSchlick(max(dot(N,V), 0.0), F0);
    vec3 kD = 1.0-kS;
    kD *= 1.0-metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (kD * diffuse) * ao;
    
    vec3 color = ambient + Lo;

    //gamma correction
    color = color/(color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color,1.0);
} 