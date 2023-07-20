#vertex
#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 modelMatrix;
uniform mat4 lightMatrix;

uniform mat4 modelMatrixArray[200];
uniform int instancedCount = 0;

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
    gl_Position = lightMatrix * worldPos;
}


#fragment
#version 450 core

out vec4 FragColor;

void main()
{

}
