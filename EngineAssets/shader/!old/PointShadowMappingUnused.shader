#vertex
#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 PIPELINE_MODEL_MAT;

uniform mat4 PIPELINE_MODEL_MAT_ARRAY[200];
uniform int PIPELINE_INTANCE_COUNT = 0;

out vec3 fragPos;

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

    vec4 worldPos = modelMat *  vec4(aPos, 1.0);
    fragPos = worldPos.xyz;
    gl_Position = worldPos;
}

#geometry
#version 450 core
layout (triangles) in;
layout(invocations = 6) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 fragPos[];
out vec3 geo_fragPos;

uniform mat4 PIPELINE_LIGHT_MAT[6];

void main()
{
    gl_Position = PIPELINE_LIGHT_MAT[gl_InvocationID] * gl_in[0].gl_Position;
    geo_fragPos = fragPos[0];
    gl_ViewportIndex = gl_InvocationID;
    EmitVertex();

    gl_ViewportIndex = gl_InvocationID;
    geo_fragPos = fragPos[1];
    gl_Position = PIPELINE_LIGHT_MAT[gl_InvocationID] * gl_in[1].gl_Position;
    EmitVertex();

    gl_ViewportIndex = gl_InvocationID;
    geo_fragPos = fragPos[2];
    gl_Position = PIPELINE_LIGHT_MAT[gl_InvocationID] * gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}

#fragment
#version 450 core

uniform vec3 PIPELINE_LIGHT_POS;

in vec3 geo_fragPos;
out vec3 lightFragDist;

void main()
{
    lightFragDist = vec3(length(geo_fragPos - PIPELINE_LIGHT_POS), 0, gl_FragCoord.z);
}