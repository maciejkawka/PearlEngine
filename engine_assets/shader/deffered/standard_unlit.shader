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

//material textures
uniform sampler2D albedoMap;

// material textures
uniform vec4 albedoValue;

//Universal set by pipeline uniform
uniform float PIPELINE_NEAR = 1.0f;
uniform float PIPELINE_FAR = 1000.0f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * PIPELINE_NEAR * PIPELINE_FAR) / (PIPELINE_FAR + PIPELINE_NEAR - z * (PIPELINE_FAR - PIPELINE_NEAR));
}

void main()
{
    //Sample Textures
    vec3 albedoTex = texture(albedoMap, IN.uv0).rgb;

    //Get textures and values combined
    vec3 albedo = albedoTex + albedoValue.rgb;

    //IMPORTANT all vectors are in world space
    gPosition = vec4(IN.pos, LinearizeDepth(gl_FragCoord.z));
    gAlbedo = vec4(albedo, 0.0f);
    gNormal = vec4(vec3(0.0f), 0.0f);
    gAO = vec4(vec3(0.0f), 1.0f);
}