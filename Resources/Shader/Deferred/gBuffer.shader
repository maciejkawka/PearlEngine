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

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gAO;

in Vertex{
    vec3 pos;
    vec3 normals;
    mat3 TBN;
    vec2 uv0;
} IN;

//PBR Textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// PBR values if no texture
uniform vec4 albedoValue;
uniform float metallicValue;
uniform float roughnessValue;
uniform float aoValue = 1.0;

uniform vec3 camPos;
uniform bool normalMapping;

uniform float nearPlane = 1.0f;
uniform float farPlane = 1000.0f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
    //Sample Textures
    vec3 albedoTex = texture(albedoMap, IN.uv0).rgb;
    albedoTex = pow(albedoTex.rgb, vec3(2.2));
    float metallicTex = texture(metallicMap, IN.uv0).r;
    float roughnessTex = texture(roughnessMap, IN.uv0).r;
    float aoTex = texture(aoMap, IN.uv0).r;

    //Get textures and values combined
    vec3 albedo = albedoTex + albedoValue.rgb;
    float metallic = metallicTex + metallicValue;
    float roughness = roughnessTex + roughnessValue;
    float ao = aoTex + aoValue;

    //normals calculations
    vec3 normals;
    if(normalMapping)
    {
        normals = texture(normalMap, IN.uv0).rgb;
        normals = normalize(IN.TBN * normalize(normals * 2.0 - 1.0)); 
    }
    else
    {
        normals = normalize(IN.normals);
    }

    gPosition = vec4(IN.pos, LinearizeDepth(gl_FragCoord.z));
    gAlbedo = vec4(albedo, roughness);
    gNormal = vec4(normals, metallic);
    gAO = vec4(vec3(0,0,0), ao);
}