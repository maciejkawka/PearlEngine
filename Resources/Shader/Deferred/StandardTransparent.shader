#vertex

#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec4 aTangents;
layout (location = 3) in vec2 aUV0;

uniform vec2 albedoMap_scale = vec2(1,1);
uniform vec2 albedoMap_offset = vec2(0, 0);

// Universal uniform set by pipeline
uniform mat4 PIPELINE_VP_MAT;
uniform mat4 PIPELINE_MODEL_MAT;
uniform mat4 PIPELINE_MODEL_MAT_ARRAY[200];
uniform int PIPELINE_INTANCE_COUNT = 0;

out Vertex{
    vec3 pos;
    vec3 normals;
    mat3 TBN;
    vec2 uv0;
} OUT;


void main()
{
    mat4 modelMat;
    if(PIPELINE_INTANCE_COUNT != 0)
    {
        //Grab instanced modelMatrix
        modelMat = PIPELINE_MODEL_MAT_ARRAY[gl_InstanceID];
    }
    else
    {
        modelMat = PIPELINE_MODEL_MAT;
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

    gl_Position = PIPELINE_VP_MAT * worldPos;
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
uniform bool normalMapping;

// material textures
uniform vec4 albedoValue;
uniform float metallicValue;
uniform float roughnessValue;
uniform float aoValue = 1.0;

uniform vec3 ambientColor; //To be add in future

// Universal uniform set by pipeline
uniform vec3 PIPELINE_CAMPOS;

uniform samplerCube PIPELINE_IRRADIANCE_MAP;
uniform samplerCube PIPELINE_PREFILTER_MAP;
uniform sampler2D PIPELINE_BRDF_LUT;


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

    vec3 V = normalize(PIPELINE_CAMPOS - IN.pos);
    vec3 R = reflect(-V,N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    //IR diffuse 
    vec3 F = fresnelSchlickRoughness(max(dot(N,V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0-kS;
    kD *= 1.0-metallic;
    vec3 irradiance = texture(PIPELINE_IRRADIANCE_MAP, N).rgb;
    vec3 diffuse = irradiance * albedo;

    //Indirect lighting specular
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(PIPELINE_PREFILTER_MAP, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(PIPELINE_BRDF_LUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse * transparency + specular) * ao;
    
    vec3 color = ambient + Lo * transparency;

    //tone maping    
    color = color/(color + vec3(1.0));
    
    //gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color,transparency);
}