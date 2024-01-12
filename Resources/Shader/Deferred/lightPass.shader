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

out vec4 FragColor;
in vec2 uv0;

//General consts
const float      PI = 3.14159265359;


//Main Uniforms
uniform vec3     camPos;


//gBuff textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;
uniform sampler2D aoMap;


//Lighting
const int         poissonDiskSize = 16;
const vec2        poissonDisk[16] = {
    vec2(0.588388f, 0.500000f),
    vec2(0.387110f, 0.603409f),
    vec2(0.517294f, 0.303116f),
    vec2(0.642265f, 0.685602f),
    vec2(0.238895f, 0.453775f),
    vec2(0.747377f, 0.342703f),
    vec2(0.417199f, 0.807744f),
    vec2(0.342298f, 0.196162f),
    vec2(0.842284f, 0.625116f),
    vec2(0.143823f, 0.646886f),
    vec2(0.671812f, 0.133199f),
    vec2(0.626701f, 0.904517f),
    vec2(0.117725f, 0.278238f),
    vec2(0.948614f, 0.401599f),
    vec2(0.226045f, 0.889245f),
    vec2(0.437026f, 0.011920f)
};
const int         maxDirLightNum = 8;
const int         maxPointLightNum = 16;
const int         maxSpotLightNum = 16;

uniform vec3      ambientColor; //To be add in future


// Shadows
const int         SHDW_cascadesCount = 4;

uniform float     SHDW_borders[4];
uniform float     SHDW_RadiusRatio[4];
uniform float     SHDW_BorderBlend = 5.0f;

uniform sampler2D SHDW_MainDirLightMap;
uniform int       SHDW_MainDirLightMapSize;
uniform int       SHDW_MainDirLightCombineMapSize;
uniform mat4      SHDW_MainDirLightViewMat[4];
uniform mat4      SHDW_MainDirLightMat;
uniform bool      SHDW_HasMainDirLight = false;
uniform float     SHDW_MainDirLightSize = 3.0f;
uniform float     SHDW_MainDirLightBias = 0.005f;

uniform sampler2D SHDW_PointLightMap;
uniform int       SHDW_PointLightMapSize;
uniform int       SHDW_PointCombineLightMapSize;
uniform mat4      SHDW_PointLightMat[maxPointLightNum];
uniform int       SHDW_PointLightID[maxPointLightNum];
uniform int       SHDW_PointLightNumber;
uniform float     SHDW_PointLightSize = 5.0f;
uniform float     SHDW_PointLightBias = 0.2f;

uniform sampler2D SHDW_SpotLightMap;
uniform int       SHDW_SpotLightMapSize;
uniform int       SHDW_SpotLightCombineMapSize;
uniform mat4      SHDW_SpotLightViewMat[maxSpotLightNum];
uniform mat4      SHDW_SpotLightMat[maxSpotLightNum];
uniform int       SHDW_SpotLightID[maxSpotLightNum];
uniform int       SHDW_SpotLightNumber;
uniform float     SHDW_SpotLightSize = 1.0f;
uniform float     SHDW_SpotLightBias = 0.005f;

uniform sampler2D SHDW_DirLightMap;
uniform int       SHDW_DirLightMapSize;
uniform int       SHDW_DirLightCombineShadowMapSize;
uniform mat4      SHDW_DirLightViewMat[maxDirLightNum * SHDW_cascadesCount];
uniform mat4      SHDW_DirLightMat[maxDirLightNum];
uniform int       SHDW_DirLightID[maxDirLightNum];
uniform int       SHDW_DirLightNumber;
uniform float     SHDW_DirLightSize = 3.0f;
uniform float     SHDW_DirLightBias = 0.005f;


// IBL
uniform samplerCube PBR_irradianceMap;
uniform samplerCube PBR_prefilterMap;
uniform sampler2D   PBR_brdfLUT;


// pseudorandom number generator
float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(vec4 co){
	float dot_product = dot(co, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

//--------------------Cascade Shadow Mapping-------------------------
vec3 CSM_DebugCascades(int cascadeIndex)
{
    vec3 CascadeIndicator = vec3(0.0f);
    if (cascadeIndex == 0)
        CascadeIndicator = vec3(1.0, 0.0, 0.0);
    else if (cascadeIndex == 1)
        CascadeIndicator = vec3(0.0, 1.0, 0.0);
    else if (cascadeIndex == 2)
        CascadeIndicator = vec3(0.0, 0.0, 1.0);
    else if (cascadeIndex == 3)
        CascadeIndicator = vec3(1.0, 1.0, 1.0);
    return CascadeIndicator;
}

//--------------------PBR Functions----------------------------------
float PBR_DistributionGGX(vec3 N, vec3 H, float roughness)
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

float PBR_GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float PBR_GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = PBR_GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = PBR_GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 PBR_FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 PBR_FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

struct PBR_Data
{
    //Material Values
    vec3 albedo;
    float roughness;
    float metallic;

    vec3 pos;
    vec3 V; //Light - eye dir
    vec3 R; //Reflection
    vec3 N; //Normal
    vec3 F0; //Reflectnace

};

vec3 PBR_Light(mat4 light, PBR_Data data)
{
    //Light variables
    int lightType = int(light[0].w);
    vec3 lightPos = light[0].xyz;
    vec3 lightColor = vec3(light[2].xyz);
    vec3 lightDir = normalize(-light[1].xyz);
    float lightRange = light[3].w;

    vec3 L = vec3(0);
    float attenuation = 0;

    //Unpack data
    vec3 albedo = data.albedo;
    float roughness = data.roughness;
    float metallic = data.metallic;
    vec3 pos = data.pos;
    vec3 V = data.V;
    vec3 R = data.R;
    vec3 N = data.N;
    vec3 F0 = data.F0;

    //Check light type and calculate 
    if(lightType == 0) //Directional Light
    {
        L = lightDir;
        attenuation = 1.0;
    }
    if(lightType == 1) //Point Light
    {
        L = normalize(lightPos - pos);
        
        float distance = length(lightPos - pos);
        attenuation = 1.0 / (distance * distance);

        if(distance >= lightRange)
            return vec3(0.0f);
    }
    if(lightType == 2) //Spot Light
    {
        L = normalize(lightPos - pos);
        float distance = length(lightPos - pos);

        float theta = dot(L, lightDir);
        float epsilon = light[1].w - light[2].w;
        float intensity = clamp((theta - light[2].w) / epsilon, 0.0, 1.0);

        if (intensity == 0)
            return vec3(0.0f);

        attenuation =  1.0 / (distance * distance);
        attenuation = attenuation * intensity;
    }     

    vec3 radiance = lightColor * attenuation;
    vec3 H = normalize(V + L);

    float NDF = PBR_DistributionGGX(N, H, roughness);
    float G = PBR_GeometrySmith(N, V, L, roughness);
    vec3 F = PBR_FresnelSchlick(max(dot(H, V), 0.0), F0); 

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot( N, L),0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    return Lo;
}
//-------------------------------------------------------------------

//--------------------Shadow Calculations----------------------------
vec2 SHDW_CalculateLightUVs(int id, int subTexSize, int comboTexSize, vec2 UVs)
{
    // clamp UVs not to exceed the [0,1] boundary
    UVs = clamp(UVs, 0.0f, 1.0f);

    int subTexPerRow = int(comboTexSize / subTexSize);
    int subTexRow = int(id / subTexPerRow);
    int subTexCol = id - subTexRow * subTexPerRow;

    // Add offset to avoid black seam between cubemap faces
    float offset = 1.0f / comboTexSize;
    vec2 subTexUVsStart = vec2(subTexCol, subTexRow) / subTexPerRow;
    subTexUVsStart += offset;
    vec2 subTexUVsEnd = vec2(subTexCol + 1, subTexRow + 1) / subTexPerRow;
    subTexUVsEnd -= offset;
    vec2 returnUVs =  mix(subTexUVsStart, subTexUVsEnd, UVs);
    
    returnUVs = floor(returnUVs * comboTexSize) / comboTexSize;
    return returnUVs;
}

float SHDW_CalculateBias(vec3 N, vec3 lightDir, float bias) {
    return max(bias * (1.0f - dot(N, lightDir)), bias);
}

float SHDW_PenumbraSize(float zReciever, float zBlocker)
{
    return (zReciever - zBlocker) / zBlocker;
}

float SHDW_FindBlockers(int lightId, vec2 UVs, float lightFragDist, sampler2D map, int mapSize, int mapCombineSize)
{
    float blockerSum = 0.0f;
    float numBlockers = 0.0f;
    float theta = rand(vec4(UVs, gl_FragCoord.xy));
    mat2 rotation = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
    for (int i = 0; i < poissonDiskSize; i++) {
        vec2 coord = UVs + rotation * poissonDisk[i] / 800.0f;
        float smap = texture(map, SHDW_CalculateLightUVs(lightId, mapSize, mapCombineSize, coord)).x;
        if (smap < lightFragDist) {
            blockerSum += smap;
            numBlockers++;
        }
    }
    return blockerSum / numBlockers;
}

float SHDW_PCF(int lightId, vec2 UVs, float lightFragDist, sampler2D map, int mapSize, int mapComboSize, float filterRadius, float bias)
{
    float sum = 0;
    float theta = rand(vec4(UVs, gl_FragCoord.xy));
    mat2 rotation = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
    for (int i = 0; i < poissonDiskSize; i++) {
        vec2 offset = rotation * poissonDisk[i] / 900.0f * filterRadius;
        vec2 texOffset = UVs + offset;

        vec2 UVCoords = SHDW_CalculateLightUVs(lightId, mapSize, mapComboSize, texOffset);
        float depth = texture(map, UVCoords).x;

        if (depth + bias < lightFragDist)
            sum += 0.0f;
        else
            sum += 1.0f;
    }

    return sum / poissonDiskSize;
}

float SHDW_PCSS(int lightId, vec2 UVs, float lightFragDist, sampler2D map, int mapSize, int mapComboSize, float bias, float lightSize)
{
    //PCSS not used, PCF calculated directly
    return SHDW_PCF(lightId, UVs, lightFragDist, map, mapSize, mapComboSize, lightSize, bias);
}

vec3 SHDW_PointMapFaceDebug(mat4 light, vec3 fragPos, int lightIndex)
{
    vec3 lightPos = light[0].xyz;
    vec3 fragToLight = fragPos - lightPos;
    vec3 fragToLightDir = normalize(fragToLight);

    vec3 absLightDirection = abs(fragToLightDir);
    int faceIndex = -1;
    if (absLightDirection.x > absLightDirection.y && absLightDirection.x > absLightDirection.z)
        faceIndex = fragToLightDir.x > 0.0 ? 0 : 1;
    else if (absLightDirection.y > absLightDirection.z)
        faceIndex = fragToLightDir.y > 0.0 ? 2 : 3;
    else
        faceIndex = fragToLightDir.z > 0.0 ? 4 : 5;

    if (faceIndex == 0)
        return vec3(1, 0, 0);
    else if (faceIndex == 1)
        return vec3(0.2f, 0, 0);
    else if (faceIndex == 2)
        return vec3(0, 1, 0);
    else if (faceIndex == 3)
        return vec3(0, 0.2f, 0);
    else if (faceIndex == 4)
        return vec3(0, 0, 1);
    else
        return vec3(0, 0, 0.2f);
}

float SHDW_CalculatePointFactor(mat4 light, vec3 fragPos, int lightIndex)
{
    vec3 lightPos = light[0].xyz;
    float lightRange = light[3].w;
    vec3 fragToLight = fragPos - lightPos;
    vec3 fragToLightDir = normalize(fragToLight);
    int  lightIDs = SHDW_PointLightID[lightIndex];

    // Check if frag is out of range
    if (length(fragToLight) >= lightRange)
        return 1.0f;

    // Determine which face to sample based on the maximum component of the direction vector
    vec3 absLightDirection = abs(fragToLightDir);
    int faceIndex = -1;
    if (absLightDirection.x > absLightDirection.y && absLightDirection.x > absLightDirection.z)
        faceIndex = fragToLightDir.x > 0.0 ? 0 : 1;
    else if (absLightDirection.y > absLightDirection.z)
        faceIndex = fragToLightDir.y > 0.0 ? 2 : 3;
    else
        faceIndex = fragToLightDir.z > 0.0 ? 4 : 5;

    // Sample correct cubemap face
    vec2 UVs = vec2(0.0f);
    if (faceIndex == 0)
        UVs = vec2(-fragToLightDir.z, -fragToLightDir.y) * 0.5 / absLightDirection.x + 0.5;
    else if (faceIndex == 1)
        UVs = vec2(fragToLightDir.z, -fragToLightDir.y) * 0.5 / absLightDirection.x + 0.5;
    else if (faceIndex == 2)
        UVs = vec2(fragToLightDir.x, fragToLightDir.z) * 0.5 / absLightDirection.y + 0.5;
    else if (faceIndex == 3)
        UVs = vec2(fragToLightDir.x, -fragToLightDir.z) * 0.5 / absLightDirection.y + 0.5;
    else if (faceIndex == 4)
        UVs = vec2(fragToLightDir.x, -fragToLightDir.y) * 0.5 / absLightDirection.z + 0.5;
    else
        UVs = vec2(-fragToLightDir.x, -fragToLightDir.y) * 0.5 / absLightDirection.z + 0.5;

    return SHDW_PCSS(lightIDs + faceIndex, UVs, length(fragToLight), SHDW_PointLightMap, SHDW_PointLightMapSize, SHDW_PointCombineLightMapSize, SHDW_PointLightBias, SHDW_PointLightSize);
}

float SHDW_CalculateSpotLightFactor(vec3 fragPos, int lightIndex)
{
    vec4 lightSpacePos = SHDW_SpotLightViewMat[lightIndex] * vec4(fragPos, 1.0f);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    // Convert [-1, 1] to [0, 1]
    projCoords = 0.5 * projCoords + 0.5;
    float fragToLight = projCoords.z;

    return SHDW_PCSS(SHDW_SpotLightID[lightIndex], projCoords.xy, fragToLight, SHDW_SpotLightMap, SHDW_SpotLightMapSize, SHDW_SpotLightCombineMapSize, SHDW_SpotLightBias, SHDW_SpotLightSize);
}

float SHDW_CalculateDirectLightFactor(float depth, vec3 fragPos, int lightIndex, vec3 normal, vec3 lightDir)
{
    float shadowFactor = 1.0f;
    for (int i = 0; i < SHDW_cascadesCount; i++)
    {
        //Blending between two layers 
        if (abs(depth - SHDW_borders[i]) < SHDW_BorderBlend)
        {
           vec4 lightSpacePosA = SHDW_DirLightViewMat[lightIndex * SHDW_cascadesCount + i] * vec4(fragPos, 1.0f);
           vec3 projCoordsA = lightSpacePosA.xyz / lightSpacePosA.w;
           projCoordsA = 0.5 * projCoordsA + 0.5;

           vec4 lightSpacePosB = SHDW_DirLightViewMat[lightIndex * SHDW_cascadesCount + i + 1] * vec4(fragPos, 1.0f);
           vec3 projCoordsB = lightSpacePosB.xyz / lightSpacePosB.w;
           projCoordsB = 0.5 * projCoordsB + 0.5;

           float distToTransition = depth - SHDW_borders[i];
           float blendValue = (distToTransition / SHDW_BorderBlend) * 0.5f + 0.5f;

           float blendA = SHDW_PCSS(SHDW_DirLightID[lightIndex] * SHDW_cascadesCount + i, projCoordsA.xy, projCoordsA.z, SHDW_DirLightMap, SHDW_DirLightMapSize, SHDW_DirLightCombineShadowMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i], SHDW_DirLightSize * SHDW_RadiusRatio[i]);
           float blendB = SHDW_PCSS(SHDW_DirLightID[lightIndex] * SHDW_cascadesCount + i + 1, projCoordsB.xy, projCoordsB.z, SHDW_DirLightMap, SHDW_DirLightMapSize, SHDW_DirLightCombineShadowMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i + 1], SHDW_DirLightSize * SHDW_RadiusRatio[i + 1]);

           shadowFactor = mix(blendA, blendB, blendValue);
            break;
        }
        //Normal Shadow
        else if (depth < SHDW_borders[i])
        {
            vec4 lightSpacePos = SHDW_DirLightViewMat[lightIndex * SHDW_cascadesCount + i] * vec4(fragPos, 1.0f);
            vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
            // Convert [-1, 1] to [0, 1]
            projCoords = 0.5 * projCoords + 0.5;
            float fragToLight = projCoords.z;
            shadowFactor = SHDW_PCSS(SHDW_DirLightID[lightIndex] * SHDW_cascadesCount + i, projCoords.xy, fragToLight, SHDW_DirLightMap, SHDW_DirLightMapSize, SHDW_DirLightCombineShadowMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i], SHDW_DirLightSize * SHDW_RadiusRatio[i]);
            //Lo = CSM_DebugCascades(i) * shadowFactor;
            break;
        }
    }
    return shadowFactor;
}

float SHDW_CalculateMainDirectLightFactor(float depth, vec3 fragPos)
{
    float shadowFactor = 1.0f;
    for (int i = 0; i < SHDW_cascadesCount; i++)
    {
        //Blending between two layers 
        if (abs(depth - SHDW_borders[i]) < SHDW_BorderBlend )
        {
            vec4 lightSpacePosA = SHDW_MainDirLightViewMat[i] * vec4(fragPos, 1.0f);
            vec3 projCoordsA = lightSpacePosA.xyz / lightSpacePosA.w;
            projCoordsA = 0.5 * projCoordsA + 0.5;

            vec4 lightSpacePosB = SHDW_MainDirLightViewMat[i + 1] * vec4(fragPos, 1.0f);
            vec3 projCoordsB = lightSpacePosB.xyz / lightSpacePosB.w;
            projCoordsB = 0.5 * projCoordsB + 0.5;

            float distToTransition = depth - SHDW_borders[i];
            float blendValue = (distToTransition / SHDW_BorderBlend) * 0.5f + 0.5f;

            float blendA = SHDW_PCSS(i, projCoordsA.xy, projCoordsA.z, SHDW_MainDirLightMap, SHDW_MainDirLightMapSize, SHDW_MainDirLightCombineMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i], SHDW_DirLightSize * SHDW_RadiusRatio[i]);
            float blendB = SHDW_PCSS(i + 1, projCoordsB.xy, projCoordsB.z, SHDW_MainDirLightMap, SHDW_MainDirLightMapSize, SHDW_MainDirLightCombineMapSize, SHDW_MainDirLightBias * SHDW_RadiusRatio[i + 1], SHDW_MainDirLightSize * SHDW_RadiusRatio[i + 1]);

            shadowFactor = mix(blendA, blendB, blendValue);
            break;
        }
        //Normal Shadow
        else if (depth < SHDW_borders[i])
        {
            vec4 lightSpacePos = SHDW_MainDirLightViewMat[i] * vec4(fragPos, 1.0f);
            vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
            // Convert [-1, 1] to [0, 1]
            projCoords = 0.5 * projCoords + 0.5;
            float fragToLight = projCoords.z;

            shadowFactor = SHDW_PCSS(i, projCoords.xy, fragToLight, SHDW_MainDirLightMap, SHDW_MainDirLightMapSize, SHDW_MainDirLightCombineMapSize, SHDW_MainDirLightBias * SHDW_RadiusRatio[i], SHDW_MainDirLightSize * SHDW_RadiusRatio[i]);
            //Lo = CSM_DebugCascades(i) * shadowFactor;
            break;
        }
    }
    return shadowFactor;
}
//-------------------------------------------------------------------


//-----------------------------MAIN----------------------------------
void main()
{
    //Sample Textures
    vec3 albedo = texture(albedoMap, uv0).rgb;
    float roughness = texture(albedoMap, uv0).a;
    //albedoTex = pow(albedoTex.rgb, vec3(2.2));
    float ao = texture(aoMap, uv0).a;
    vec3 emission = texture(aoMap, uv0).rgb;
    vec3 N = texture(normalMap, uv0).rgb;
    float metallic = texture(normalMap, uv0).a;
    vec3 pos = texture(positionMap, uv0).rgb;
    float depth = texture(positionMap, uv0).a;

    //If depth is 0 discard
    if(depth == 0)
        discard;
    // If N is vec3(0) it means we do not want to make a lighing on that fragment and copy albedo to the buffer
    if(N == vec3(0.0f))
      {
        FragColor = vec4(albedo, 1.0f);
        return;
      }
      
    N = normalize(N);
    vec3 V = normalize(camPos - pos);
    vec3 R = reflect(-V,N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    PBR_Data data;
    data.albedo = albedo;
    data.roughness = roughness;
    data.metallic = metallic;
    data.pos = pos;
    data.V = V;
    data.R = R;
    data.N = N;
    data.F0 = F0;

    vec3 Lo = vec3(0.0);

    //Main CSM Directional Light
    if(SHDW_HasMainDirLight == true)
    { 
        float shadowFactor = SHDW_CalculateMainDirectLightFactor(depth, pos);
        if (shadowFactor > 0.0f)
        {
            Lo += PBR_Light(SHDW_MainDirLightMat, data) * shadowFactor;
        }
    }

    //Directional Light
    for (int i = 0; i < SHDW_DirLightNumber; i++)
    {
        vec3 lightDir = normalize(-SHDW_DirLightMat[i][1].xyz);
        float shadowFactor = SHDW_CalculateDirectLightFactor(depth, pos, i, N, lightDir);
        if (shadowFactor > 0.0f)
        {
            vec3 lightAmmount = PBR_Light(SHDW_DirLightMat[i], data) * shadowFactor;
            Lo += lightAmmount;
        }
    }
    
    //Point Light
    for (int i = 0; i < SHDW_PointLightNumber; i++)
    {
        float shadowFactor = SHDW_CalculatePointFactor(SHDW_PointLightMat[i], pos, i);
        if (shadowFactor > 0.0f)
        {
            vec3 lightAmmount = PBR_Light(SHDW_PointLightMat[i], data) * shadowFactor;
            Lo += lightAmmount;
        }
    }

    //Spot Light
    for (int i = 0; i < SHDW_SpotLightNumber; i++)
    {
        float shadowFactor = SHDW_CalculateSpotLightFactor(pos, i);
        if (shadowFactor > 0.0f)
        {
            vec3 lightAmmount = PBR_Light(SHDW_SpotLightMat[i], data) * shadowFactor;
            Lo += lightAmmount;
        }
    }

    //IR diffuse 
    vec3 F = PBR_FresnelSchlickRoughness(max(dot(N,V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0-kS;
    kD *= 1.0-metallic;
    vec3 irradiance = texture(PBR_irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    //Indirect lighting specular
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(PBR_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(PBR_brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo + emission;

    //tone maping    
    color = color/(color + vec3(1.0));
    
    //gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0f);
} 