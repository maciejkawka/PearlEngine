#vertex

#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec4 aTangents;
layout (location = 3) in vec2 aUV0;

uniform mat4 VPMatrix;
uniform mat4 modelMatrix;
uniform mat4 modelMatrixArray[200];
uniform int instancedCount = 0;

uniform vec2 albedoMap_scale = vec2(1,1);
uniform vec2 albedoMap_offset = vec2(0, 0);

out Vertex{
    vec3 pos;
    vec3 normals;
    mat3 TBN;
    vec2 uv0;
} OUT;


void main()
{
    mat4 modelMat;
    if(instancedCount != 0)
    {
        //Grab instanced modelMatrix
        modelMat = modelMatrixArray[gl_InstanceID];
    }
    else
    {
        modelMat = modelMatrix;
    }

    //Normal moved to tangent space
    mat3 normalMat = transpose(inverse(mat3(modelMat)));
    vec3 normals = normalize(normalMat * normalize(aNormals)); 
    vec3 tangents = normalize(normalMat * normalize(aTangents.xyz));
    vec3 bitangents = normalize(cross(tangents, normals) * aTangents.w);

    OUT.TBN = mat3(tangents, bitangents, normals);
    OUT.normals = normals;
    
    //UVs calculations
    OUT.uv0 = aUV0 * albedoMap_scale + albedoMap_offset;
    
    //Set vertex to world space
    vec4 worldPos = (modelMat * vec4(aPos, 1.0));
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

//material textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D transparencyMap;

// material textures
uniform vec4 albedoValue;
uniform float metallicValue;
uniform float roughnessValue;
uniform float aoValue = 1.0;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights diabled for transparent objects
//uniform mat4 lightMat[4];
//uniform int lightNumber = 0;
uniform vec3 ambientColor; //To be add in future

//Additional
uniform vec3 camPos;
uniform bool normalMapping;

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
    //Sample Textures
    vec4 albedoTex = texture(albedoMap, IN.uv0);
    vec3 albedoTexPow = pow(albedoTex.rgb, vec3(2.2));
    float metallicTex = texture(metallicMap, IN.uv0).r;
    float roughnessTex = texture(roughnessMap, IN.uv0).r;
    float aoTex = texture(aoMap, IN.uv0).r;

    //Get textures and values combined
    vec3 albedo = albedoTexPow + albedoValue.rgb;
    float transparency = albedoTex.a + albedoValue.a;
    float metallic = metallicTex + metallicValue;
    float roughness = roughnessTex + roughnessValue;
    float ao = aoTex + aoValue;

    //normals calculations
    vec3 N;
    if(normalMapping)
    {
        N = texture(normalMap, IN.uv0).rgb;
        N = normalize(IN.TBN * normalize(N * 2.0 - 1.0)); 
    }
    else
    {
        N = normalize(IN.normals);
    }

    vec3 V = normalize(camPos - IN.pos);
    vec3 R = reflect(-V,N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    //Lighting disabled for transparent objects
    vec3 Lo = vec3(0.0);
    // int clampedLightNumber = clamp(lightNumber, 0, MAX_LIGHT_NUM);
    // for (int i = 0; i < clampedLightNumber; i++)
    // {
    //     //Light variables
    //     mat4 light = lightMat[i];
    //     int lightType = int(light[0].w);
    //     vec3 lightPos = light[0].xyz;
    //     vec3 lightColor = vec3(light[2].xyz);
    //     vec3 lightDir = normalize(-light[1].xyz);
 
    //     vec3 L = vec3(0);
    //     float attenuation = 0;

    //     //Check light type and calculate 
    //     if(lightType == 0) //Directional Light
    //     {
    //         L = lightDir;
    //         attenuation = 1.0;
    //     }
    //     if(lightType == 1) //Point Light
    //     {
    //         L = normalize(lightPos - IN.pos);
        
    //         float distance = length(lightPos - IN.pos);
    //         attenuation = 1.0 / (distance * distance);
    //     }
    //     if(lightType == 2) //Spot Light
    //     {
    //         L = normalize(lightPos - IN.pos);
    //         float distance = length(lightPos - IN.pos);

    //         float theta = dot(L, lightDir);
    //         float epsilon = light[1].w - light[2].w;
    //         float intensity = clamp((theta - light[2].w) / epsilon, 0.0, 1.0);

    //         attenuation =  1.0 / (distance * distance);
    //         attenuation = attenuation * intensity;
    //     }     

    //     vec3 radiance = lightColor * attenuation;
    //     vec3 H = normalize(V + L);

    //     float NDF = DistributionGGX(N, H, roughness);
    //     float G = GeometrySmith(N, V, L, roughness);
    //     vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0); 

    //     vec3 numerator = NDF * G * F;
    //     float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot( N, L),0.0) + 0.0001;
    //     vec3 specular = numerator / denominator;

    //     vec3 kS = F;
    //     vec3 kD = vec3(1.0) - kS;
    //     kD *= 1.0 - metallic;

    //     float NdotL = max(dot(N, L), 0.0);
    //     Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    // }


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
    vec3 ambient = (kD * diffuse * transparency + specular) * ao;
    
    vec3 color = ambient + Lo * transparency;

    //tone maping    
    color = color/(color + vec3(1.0));
    
    //gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color,transparency);
} 