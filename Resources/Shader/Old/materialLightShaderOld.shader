#vertex

#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aUV0;

uniform mat4 VPMatrix;
uniform mat4 modelMatrix;

uniform vec2 mainTex_scale = vec2(1,1);
uniform vec2 mainTex_offset = vec2(0, 0);


out Vertex{
    vec3 pos;
    vec3 normals;
    vec2 uv0;
} OUT;


void main()
{
    mat3 normalMat = transpose(inverse(mat3(modelMatrix)));
    OUT.normals = normalize(normalMat * normalize(aNormals)); 
    OUT.uv0 = aUV0 * mainTex_scale + mainTex_offset;
    
    vec4 worldPos = (modelMatrix * vec4(aPos, 1.0));
    OUT.pos = worldPos.xyz;

    gl_Position = VPMatrix * worldPos;
}

#fragment

#version 450 core
out vec4 FragColor;

in Vertex{
    vec3 pos;
    vec3 normals;
    vec2 uv0;
} IN;


uniform vec4 _color;
uniform sampler2D mainTex;
uniform vec3 camPos;

uniform vec3 lightPos;
float lightRadius = 100.0;
vec3 lightColor = vec3(1.0,0.0,1.0);

void main()
{
    vec3 incident = normalize(lightPos - IN.pos);
    vec3 viewDir = normalize(camPos - IN.pos);
    vec3 halfDir = normalize(incident + viewDir);

    vec4 diffuse = texture(mainTex, IN.uv0);

    float lambert = max(dot(incident, IN.normals), 0.0f);
    float distance = length(lightPos - IN.pos);
    float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);

    float specFactor = clamp(dot(halfDir, IN.normals), 0.0, 1.0);
    specFactor = pow(specFactor ,60.0);

    vec3 surface = (diffuse.rgb * lightColor.rgb);
    FragColor.rgb = surface * lambert * attenuation;
    FragColor.rgb += (lightColor.rgb * specFactor) * attenuation * 0.33;
    FragColor.rgb += surface * 0.1f;
    FragColor.a = diffuse.a;
} 