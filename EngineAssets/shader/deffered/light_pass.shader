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
uniform vec3     PIPELINE_CAMPOS;


//gBuff textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;
uniform sampler2D aoMap;


//Lighting
const int         poissonDiskSize = 16;
const vec2        poissonDisk[16] = {
    vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790)
};
const int         maxDirLightNum = 8;
const int         maxPointLightNum = 40;
const int         maxSpotLightNum = 16;



// Shadows
const int         SHDW_cascadesCount = 4;

uniform float     SHDW_borders[4];
uniform float     SHDW_RadiusRatio[4];
uniform float     SHDW_BorderBlend = 5.0f;

uniform sampler2D SHDW_MainDirLightMap;
uniform int       SHDW_MainDirLightMapSize;
uniform int       SHDW_MainDirLightCombineMapSize;
uniform bool      SHDW_MainDirLightShadow;
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
uniform float       PBR_cubemapIntensity = 1.0f;
uniform vec3        PBR_ambientColor;

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
        attenuation = 1.0 / ( 1.0f + 1.0 * distance + 1.0 * distance * distance);

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

        if (intensity == 0 || distance >= lightRange)
            return vec3(0.0f);

        attenuation =  1.0 / ( 1.0f + 1.0 * distance + 1.0 * distance * distance);
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
    // Clamp to reasonable value to avoid bloom flickering
    specular = clamp(specular, 0.0f, 10.0f);
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

    int subTexPerRow = comboTexSize / subTexSize;
    int subTexRow = id / subTexPerRow;
    int subTexCol = id % subTexPerRow;

    // Add offset to avoid black seam between cubemap faces
    vec2 subTexUVsStart = vec2(subTexCol, subTexRow) / float(subTexPerRow);
    vec2 subTexUVsEnd = vec2(subTexCol + 1, subTexRow + 1) / float(subTexPerRow);
    
    float offset = 1.0f / float(comboTexSize);
    subTexUVsEnd -= offset;
    subTexUVsStart += offset;
    
    vec2 returnUVs = mix(subTexUVsStart, subTexUVsEnd, UVs);
    returnUVs = floor(returnUVs * comboTexSize) / floor(comboTexSize);
    return returnUVs;
}

float SHDW_CalculateBias(vec3 N, vec3 lightDir, float bias) {
    return max(bias * (1.0f - dot(N, lightDir)), bias);
}

float SHDW_PenumbraSize(float zReciever, float zBlocker)
{
    return (zReciever - zBlocker) / zBlocker;
}

float SHDW_PCF(int lightId, vec2 UVs, float lightFragDist, sampler2D map, int mapSize, int mapComboSize, float filterRadius, float bias)
{
    float sum = 0;
    float theta = rand(vec4(UVs, gl_FragCoord.xy));
    mat2 rotation = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
    for (int i = 0; i < poissonDiskSize; i++) {
        vec2 offset = rotation * poissonDisk[i] * (filterRadius / float(mapSize));
        vec2 texOffset = UVs + offset;

        vec2 UVCoords = SHDW_CalculateLightUVs(lightId, mapSize, mapComboSize, texOffset);
        float depth = texture(map, UVCoords).x;

        float depthDifference = lightFragDist - depth;
        sum += smoothstep(bias, 0.0, depthDifference);
    }

    return sum / float(poissonDiskSize);
}

float SHDW_CalculateShadowFactor(int lightId, vec2 UVs, float lightFragDist, sampler2D map, int mapSize, int mapComboSize, float bias, float lightSize)
{
    //Calculate PCF shadow factor
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

    return SHDW_CalculateShadowFactor(lightIDs + faceIndex, UVs, length(fragToLight), SHDW_PointLightMap, SHDW_PointLightMapSize, SHDW_PointCombineLightMapSize, SHDW_PointLightBias, SHDW_PointLightSize);
}

float SHDW_CalculateSpotLightFactor(vec3 fragPos, int lightIndex)
{
    vec4 lightSpacePos = SHDW_SpotLightViewMat[lightIndex] * vec4(fragPos, 1.0f);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    projCoords = 0.5 * projCoords + 0.5;
    float fragToLight = projCoords.z;

    return SHDW_CalculateShadowFactor(SHDW_SpotLightID[lightIndex], projCoords.xy, fragToLight, SHDW_SpotLightMap, SHDW_SpotLightMapSize, SHDW_SpotLightCombineMapSize, SHDW_SpotLightBias, SHDW_SpotLightSize);
}

float SHDW_CalculateDirectLightFactor(float depth, vec3 fragPos, int lightIndex)
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

           float blendA = SHDW_CalculateShadowFactor(SHDW_DirLightID[lightIndex] * SHDW_cascadesCount + i, projCoordsA.xy, projCoordsA.z, SHDW_DirLightMap, SHDW_DirLightMapSize, SHDW_DirLightCombineShadowMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i], SHDW_DirLightSize * SHDW_RadiusRatio[i]);
           float blendB = SHDW_CalculateShadowFactor(SHDW_DirLightID[lightIndex] * SHDW_cascadesCount + i + 1, projCoordsB.xy, projCoordsB.z, SHDW_DirLightMap, SHDW_DirLightMapSize, SHDW_DirLightCombineShadowMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i + 1], SHDW_DirLightSize * SHDW_RadiusRatio[i + 1]);

           shadowFactor = mix(blendA, blendB, blendValue);
           break;
        }
        //Normal Shadow
        else if (depth < SHDW_borders[i])
        {
            vec4 lightSpacePos = SHDW_DirLightViewMat[lightIndex * SHDW_cascadesCount + i] * vec4(fragPos, 1.0f);
            vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

            projCoords = 0.5 * projCoords + 0.5;
            float fragToLight = projCoords.z;
            shadowFactor = SHDW_CalculateShadowFactor(SHDW_DirLightID[lightIndex] * SHDW_cascadesCount + i, projCoords.xy, fragToLight, SHDW_DirLightMap, SHDW_DirLightMapSize, SHDW_DirLightCombineShadowMapSize, SHDW_DirLightBias * SHDW_RadiusRatio[i], SHDW_DirLightSize * SHDW_RadiusRatio[i]);
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

            float blendA = SHDW_CalculateShadowFactor(i, projCoordsA.xy, projCoordsA.z, SHDW_MainDirLightMap, SHDW_MainDirLightMapSize, SHDW_MainDirLightCombineMapSize, SHDW_MainDirLightBias * SHDW_RadiusRatio[i], SHDW_MainDirLightSize * SHDW_RadiusRatio[i]);
            float blendB = SHDW_CalculateShadowFactor(i + 1, projCoordsB.xy, projCoordsB.z, SHDW_MainDirLightMap, SHDW_MainDirLightMapSize, SHDW_MainDirLightCombineMapSize, SHDW_MainDirLightBias * SHDW_RadiusRatio[i + 1], SHDW_MainDirLightSize * SHDW_RadiusRatio[i + 1]);

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

            shadowFactor = SHDW_CalculateShadowFactor(i, projCoords.xy, fragToLight, SHDW_MainDirLightMap, SHDW_MainDirLightMapSize, SHDW_MainDirLightCombineMapSize, SHDW_MainDirLightBias * SHDW_RadiusRatio[i], SHDW_MainDirLightSize * SHDW_RadiusRatio[i]);
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
    vec4 albedoSampled = texture(albedoMap, uv0).rgba;
    vec3 albedo = albedoSampled.rgb;
    float roughness = albedoSampled.a;

    vec4 aoSampled = texture(aoMap, uv0).rgba;
    float ao = aoSampled.a;
    vec3 emission = aoSampled.rgb;

    vec4 normalSampled = texture(normalMap, uv0).rgba;
    vec3 N = normalSampled.rgb;
    float metallic = normalSampled.a;

    vec4 positionSampled = texture(positionMap, uv0).rgba;
    vec3 pos = positionSampled.rgb;
    float depth = positionSampled.a;

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
    vec3 V = normalize(PIPELINE_CAMPOS - pos);
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
        vec3 lightAmmount = PBR_Light(SHDW_MainDirLightMat, data);
        if(length(lightAmmount) > 0.001f && SHDW_MainDirLightShadow == true)
        {
            float shadowFactor = SHDW_CalculateMainDirectLightFactor(depth, pos);
            lightAmmount *= shadowFactor;
        }

        Lo += lightAmmount;
    }

    //Directional Light
    for (int i = 0; i < SHDW_DirLightNumber; i++)
    {
        vec3 lightAmmount = PBR_Light(SHDW_DirLightMat[i], data);
        if(length(lightAmmount) > 0.001f && SHDW_DirLightID[i] != -1)
        {
            float shadowFactor = SHDW_CalculateDirectLightFactor(depth, pos, i);
            lightAmmount *= shadowFactor;
        }

        Lo += lightAmmount;
    }
    
    //Point Light
    for (int i = 0; i < SHDW_PointLightNumber; i++)
    {
        vec3 lightAmmount = PBR_Light(SHDW_PointLightMat[i], data);
        if(length(lightAmmount) > 0.001f && SHDW_PointLightID[i] != -1)
        {
            float shadowFactor = SHDW_CalculatePointFactor(SHDW_PointLightMat[i], pos, i);
            lightAmmount *= shadowFactor;
        }

        Lo += lightAmmount;
    }

    //Spot Light
    for (int i = 0; i < SHDW_SpotLightNumber; i++)
    {
        vec3 lightAmmount = PBR_Light(SHDW_SpotLightMat[i], data);
        if(length(lightAmmount) > 0.001f && SHDW_SpotLightID[i] != -1)
        {
            float shadowFactor = SHDW_CalculateSpotLightFactor(pos, i);
            lightAmmount *= shadowFactor;
        }

        Lo += lightAmmount;
    }

    //IR diffuse 
    vec3 F = PBR_FresnelSchlickRoughness(max(dot(N,V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0-kS;
    kD *= 1.0-metallic;
    vec3 irradiance = texture(PBR_irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    if(diffuse == vec3(0.0f))
        diffuse = PBR_ambientColor * albedo;

    //Indirect lighting specular
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(PBR_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(PBR_brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse + specular) * ao * PBR_cubemapIntensity;
    
    vec3 color = ambient + Lo + emission;
    color = max(color, vec3(0.0f)); // Avoid negatve values
    FragColor = vec4(color, 1.0f);
} 