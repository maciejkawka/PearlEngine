#vertex
#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 modelMatrix;
uniform mat4 lightMatrix;

uniform mat4 modelMatrixArray[200];
uniform int instancedCount = 0;

out vec3 fragPos;

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

    vec4 worldPos = modelMat *  vec4(aPos, 1.0);
    fragPos = worldPos.xyz;
    
    gl_Position = lightMatrix * worldPos;
}


#fragment
#version 450 core

in vec3 fragPos;

uniform vec3 lightPos;

out vec3 lightFragDist;

void main()
{
    lightFragDist = vec3(length(fragPos - lightPos), 0, gl_FragCoord.z);
}
