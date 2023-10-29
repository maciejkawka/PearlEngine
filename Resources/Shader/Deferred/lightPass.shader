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


//Lighting and shadowing
const int     maxLightNum = 200;

uniform mat4  lightMat[4];
uniform int   lightNumber = 0;
uniform vec3  ambientColor; //To be add in future


// IBL
uniform samplerCube PBR_irradianceMap;
uniform samplerCube PBR_prefilterMap;
uniform sampler2D   PBR_brdfLUT;


//Cascade Shadow Mapping
const int          CSM_PCFFilterSize = 64;
const int          CSM_cascadesCount = 4;
const vec2         CDM_poissonDisk[64] = {
    vec2(-0.04117257f, -0.1597612f),
    vec2(0.06731031f, -0.4353096f),
    vec2(-0.206701f, -0.4089882f),
    vec2(0.1857469f, -0.2327659f),
    vec2(-0.2757695f, -0.159873f),
    vec2(-0.2301117f, 0.1232693f),
    vec2(0.05028719f, 0.1034883f),
    vec2(0.236303f, 0.03379251f),
    vec2(0.1467563f, 0.364028f),
    vec2(0.516759f, 0.2052845f),
    vec2(0.2962668f, 0.2430771f),
    vec2(0.3650614f, -0.1689287f),
    vec2(0.5764466f, -0.07092822f),
    vec2(-0.5563748f, -0.4662297f),
    vec2(-0.3765517f, -0.5552908f),
    vec2(-0.4642121f, -0.157941f),
    vec2(-0.2322291f, -0.7013807f),
    vec2(-0.05415121f, -0.6379291f),
    vec2(-0.7140947f, -0.6341782f),
    vec2(-0.4819134f, -0.7250231f),
    vec2(-0.7627537f, -0.3445934f),
    vec2(-0.7032605f, -0.13733f),
    vec2(0.8593938f, 0.3171682f),
    vec2(0.5223953f, 0.5575764f),
    vec2(0.7710021f, 0.1543127f),
    vec2(0.6919019f, 0.4536686f),
    vec2(0.3192437f, 0.4512939f),
    vec2(0.1861187f, 0.595188f),
    vec2(0.6516209f, -0.3997115f),
    vec2(0.8065675f, -0.1330092f),
    vec2(0.3163648f, 0.7357415f),
    vec2(0.5485036f, 0.8288581f),
    vec2(-0.2023022f, -0.9551743f),
    vec2(0.165668f, -0.6428169f),
    vec2(0.2866438f, -0.5012833f),
    vec2(-0.5582264f, 0.2904861f),
    vec2(-0.2522391f, 0.401359f),
    vec2(-0.428396f, 0.1072979f),
    vec2(-0.06261792f, 0.3012581f),
    vec2(0.08908027f, -0.8632499f),
    vec2(0.9636437f, 0.05915006f),
    vec2(0.8639213f, -0.309005f),
    vec2(-0.03422072f, 0.6843638f),
    vec2(-0.3734946f, -0.8823979f),
    vec2(-0.3939881f, 0.6955767f),
    vec2(-0.4499089f, 0.4563405f),
    vec2(0.07500362f, 0.9114207f),
    vec2(-0.9658601f, -0.1423837f),
    vec2(-0.7199838f, 0.4981934f),
    vec2(-0.8982374f, 0.2422346f),
    vec2(-0.8048639f, 0.01885651f),
    vec2(-0.8975322f, 0.4377489f),
    vec2(-0.7135055f, 0.1895568f),
    vec2(0.4507209f, -0.3764598f),
    vec2(-0.395958f, -0.3309633f),
    vec2(-0.6084799f, 0.02532744f),
    vec2(-0.2037191f, 0.5817568f),
    vec2(0.4493394f, -0.6441184f),
    vec2(0.3147424f, -0.7852007f),
    vec2(-0.5738106f, 0.6372389f),
    vec2(0.5161195f, -0.8321754f),
    vec2(0.6553722f, -0.6201068f),
    vec2(-0.2554315f, 0.8326268f),
    vec2(-0.5080366f, 0.8539945f)
};

uniform mat4       CSM_mainLight;
uniform bool       CSM_hasMainLight = false;
uniform mat4       CSM_viewMats[4];
uniform float      CSM_borders[4];
uniform int        CSM_mapSize;
uniform sampler2D  CSM_map;


// pseudorandom number generator
float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(vec4 co){
	float dot_product = dot(co, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}


//--------------------Cascade Shadow Mapping---------------------------
vec2 CSM_CalculateSubTexUV(int index, vec2 UVs)
{
    float u_min = 0.0f;
    float v_min = 0.0f;
    float u_max = 0.0f;
    float v_max = 0.0f;

    //Select UVs
    if (index == 0) 
    {
        u_min = 0.0;
        v_min = 0.0;
        u_max = 0.5;
        v_max = 0.5;
    } 
    else if(index == 1) 
    {
        u_min = 0.5;
        v_min = 0.0;
        u_max = 1.0;
        v_max = 0.5;
    } 
    else if(index == 2) 
    {
        u_min = 0.0;
        v_min = 0.5;
        u_max = 0.5;
        v_max = 1.0;
    } 
    else if(index == 3) 
    {
        u_min = 0.5;
        v_min = 0.5;
        u_max = 1.0;
        v_max = 1.0;
    }

    //Lerp UVs
    return  mix(vec2(u_min, v_min), vec2(u_max, v_max), UVs);
}

vec3 CSM_DebugCascades(int cascadeIndex)
{
    vec3 CascadeIndicator = vec3(0.0f);
    if (cascadeIndex == 0) 
        CascadeIndicator = vec3(1.0, 0.0, 0.0);
    else if (cascadeIndex == 1)
        CascadeIndicator = vec3(0.0, 1.0, 0.0);
    else if (cascadeIndex == 2)
        CascadeIndicator = vec3(0.0, 0.0, 1.0);
    else if (cascadeIndex ==3)
        CascadeIndicator = vec3(1.0, 1.0, 1.0);
    return CascadeIndicator;
}

float CSM_CalculateShadowFactor(int cascadeIndex, vec4 LightSpacePos)
{
   vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
 
    // Convert [-1, 1] to [0, 1]
    ProjCoords = 0.5 * ProjCoords + 0.5;
    float z = ProjCoords.z;

    float sum = 0;
    float texelSize = 1.0f / float(CSM_mapSize * 4.0f);
	float theta = rand(vec4(ProjCoords.xy, gl_FragCoord.xy));
	mat2 rotation = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
	for (int i = 0; i < CSM_PCFFilterSize; i++) {
		vec2 offset = rotation * CDM_poissonDisk[i] * 20.0f * texelSize / (cascadeIndex + 1.0f);
		vec2 texOffset = ProjCoords.xy + offset;

        vec2 UVCoords = CSM_CalculateSubTexUV(cascadeIndex, clamp(texOffset, 0.0f, 1.0f));
		float Depth = texture(CSM_map, UVCoords).x;

        if(Depth + 0.0003f< z)
            sum += 0.0f;
        else
            sum += 1.0f;
	}

	return sum / CSM_PCFFilterSize;
}
//--------------------------------------------------------------------


//--------------------PBR Functions-----------------------------------
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
    }
    if(lightType == 2) //Spot Light
    {
        L = normalize(lightPos - pos);
        float distance = length(lightPos - pos);

        float theta = dot(L, lightDir);
        float epsilon = light[1].w - light[2].w;
        float intensity = clamp((theta - light[2].w) / epsilon, 0.0, 1.0);

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
//--------------------------------------------------------------------


//-----------------------------MAIN-----------------------------------
void main()
{
    //Sample Textures
    vec3 albedo = texture(albedoMap, uv0).rgb;
    float roughness = texture(albedoMap, uv0).a;
    //albedoTex = pow(albedoTex.rgb, vec3(2.2));
    float ao = texture(aoMap, uv0).a;
    vec3 N = normalize(texture(normalMap, uv0).rgb);
    float metallic = texture(normalMap, uv0).a;
    vec3 pos = texture(positionMap, uv0).rgb;
    float depth = texture(positionMap, uv0).a;

    //If depth is 0 discard
    if(depth == 0)
        discard;

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
    data.V = V;
    data.R = R;
    data.N = N;
    data.F0 = F0;

    vec3 Lo = vec3(0.0);

    //Main CSM Directional Light
    if(CSM_hasMainLight == true)
    { 
        float shadowFactor = 0.0f;       
        for(int i = 0; i < CSM_cascadesCount ; i++)
        {
            if(depth < CSM_borders[i])
            {
                shadowFactor = CSM_CalculateShadowFactor(i, CSM_viewMats[i] * vec4(pos, 1.0f));
                //Lo = DebugCDM(i) * shadowFactor;
                break;
            }
        }

        Lo += PBR_Light(CSM_mainLight, data) * shadowFactor;
    }

    //Other Lights
    int clampedLightNumber = clamp(lightNumber, 0, maxLightNum);
    for (int i = 0; i < clampedLightNumber; i++)
    {
        //Shadow Calculations in future

        vec3 lightAmmount = PBR_Light(lightMat[i], data);
        Lo += lightAmmount;
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
    
    vec3 color = ambient + Lo;

    //tone maping    
    color = color/(color + vec3(1.0));
    
    //gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0f);
} 