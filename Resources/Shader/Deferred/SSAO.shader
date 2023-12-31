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

layout (location = 0) out float saoOutput;

in vec2 uv0;

//gBuff textures
uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D texNoise;

uniform vec3 samples[64];

uniform int kernelSize = 64;
uniform float radius = 0.5f;
uniform float bias = 0.1f;
uniform float magnitude = 1.1f;

uniform vec2 screenSize;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main(void){
    // move position and normals to view space
    vec3 fragPos = (viewMat * vec4(texture(positionMap, uv0).xyz, 1.0f)).xyz;
    vec3 normal = normalize(transpose(inverse(mat3(viewMat))) * texture(normalMap, uv0).rgb);

    vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0); 
    vec3 randomVec = normalize(texture(texNoise, uv0 * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position and move from tangent to view space
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius; 
        
        // project sample to screen space and move to [0,1] range
        vec4 offset = vec4(samplePos, 1.0);
        offset =  projectionMat * offset;
        offset.xyz /= offset.w; 
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        // Get sample depth Z axis
        float sampleDepth = (viewMat * vec4(texture(positionMap, offset.xy).xyz, 1.0f)).z;
        //Get fragment linear depth
        float linearDepth = texture(positionMap, offset.xy).a;

        //check if out of rendered objects
        if(linearDepth != 0)
        {
            float cutoff = abs(fragPos.z - sampleDepth) < radius ? 1.0f : 0.0f;
            occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * cutoff;           
        }
    }
    
    occlusion = 1.0 - (occlusion / kernelSize);
    saoOutput = pow(occlusion, magnitude);
}
