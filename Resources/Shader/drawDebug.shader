#vertex
#version 450 core
layout (location = 0) in vec3 aPos;

// Universal uniform set by pipeline
uniform mat4 PIPELINE_VP_MAT;
uniform mat4 PIPELINE_MODEL_MAT;
uniform mat4 PIPELINE_MODEL_MAT_ARRAY[200];
uniform int PIPELINE_INTANCE_COUNT = 0;

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

    vec4 worldPos = (modelMat * vec4(aPos, 1.0));
    gl_Position = PIPELINE_VP_MAT * worldPos;
}


#fragment
#version 450 core

out vec4 FragColor;
uniform vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
    FragColor = color;
}
