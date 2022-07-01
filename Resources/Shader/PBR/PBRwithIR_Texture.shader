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
    mat3 TBN;
    vec2 uv0;
} OUT;


void main()
{
    //Normal moved to tangent space
    mat3 normalMat = transpose(inverse(mat3(modelMatrix)));
    vec3 normals = normalize(normalMat * normalize(aNormals)); 
    vec3 tangents = normalize(normalMat * normalize(aTangents.xyz));
    vec3 bitangents = normalize(cross(tangents, normals) * aTangents.w);
    OUT.TBN = mat3(tangents, bitangents, normals);

    //UVs and pos calculations
    OUT.uv0 = aUV0 * mainTex_scale + mainTex_offset;
    
    vec4 worldPos = (modelMatrix * vec4(aPos, 1.0));
    OUT.pos = worldPos.xyz; 

    gl_Position = VPMatrix * worldPos;
}

#fragment

#version 450 core

out vec4 FragColor;

in Vertex{
    vec3 pos;
    vec3 normals;
    mat3 TBN;
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

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform mat4 lightMat[4];
uniform int lightNumber = 0;
uniform vec3 ambientColor;

uniform vec3 camPos;

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

void main()
{
    vec3 albedo = texture(albedoMap, IN.uv0).rgb;
    albedo = pow(albedo.rgb, vec3(2.2));
    float metallic = texture(metallicMap, IN.uv0).r;
    float roughness = texture(roughnessMap, IN.uv0).r;
    float ao = texture(aoMap, IN.uv0).r;

    //Normal Mapping 
    vec3 N = texture(normalMap, IN.uv0).rgb;
    N = normalize(IN.TBN * normalize(N * 2.0 - 1.0));

    //Basic Light Calculations
    vec3 V = normalize(camPos - IN.pos);
    vec3 R = reflect(-V,N);


    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    //Calculate All Lights
    vec3 Lo = vec3(0.0);
    int clampedLightNumber = clamp(lightNumber, 0, MAX_LIGHT_NUM);
    for (int i = 0; i < clampedLightNumber; i++)
    {
        //Light variables
        mat4 light = lightMat[i];
        int lightType = int(light[0].w);
        vec3 lightPos = light[0].xyz;
        vec3 lightColor = vec3(light[2].xyz);
        vec3 lightDir = normalize(-light[1].xyz);
 
        vec3 L = vec3(0);
        float attenuation = 0;

        //Check light type and calculate 
        if(lightType == 0) //Directional Light
        {
            L = lightDir;
            attenuation = 1.0;
        }
        if(lightType == 1) //Point Light
        {
            L = normalize(lightPos - IN.pos);
        
            float distance = length(lightPos - IN.pos);
            attenuation = 1.0 / (distance * distance);
        }
        if(lightType == 2) //Spot Light
        {
            L = normalize(lightPos - IN.pos);
            float distance = length(lightPos - IN.pos);

            float theta = dot(L, lightDir);
            float epsilon = light[1].w - light[2].w;
            float intensity = clamp((theta - light[2].w) / epsilon, 0.0, 1.0);

            attenuation =  1.0 / (distance * distance);
            attenuation = attenuation * intensity;
        }     
    
        vec3 H = normalize(V + L);
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


    //IR diffuse 
    vec3 F = fresnelSchlickRoughness(max(dot(N,V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0-kS;
    kD *= 1.0-metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    //Indirect lighting specular
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

    //gamma correction
    color = color/(color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color,1.0);
} 