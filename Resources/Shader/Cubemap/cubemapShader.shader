#vertex

#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 texCoords;

uniform mat4 proj;
uniform mat4 view;

void main()
{
  vec3 pos = aPos;
  mat4 invproj = inverse(proj);
  pos.xy *= vec2(invproj[0][0], invproj[1][1]);
  pos.z = -1.0f;
  texCoords = transpose(mat3(view)) * normalize(pos);
  gl_Position = vec4(aPos,1.0f).xyww;
}

#fragment

#version 450 core

in vec3 texCoords;
out vec4 FragColor;

uniform samplerCube skybox;
uniform float intensity = 1.0f;

void main()
{
    vec3 color = texture(skybox, normalize(texCoords)).rgb * intensity;
    
    color = pow(color, vec3(2.2f));
    FragColor = vec4(color, 1.0f);
} 