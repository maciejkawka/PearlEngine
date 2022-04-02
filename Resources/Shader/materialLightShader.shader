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

#define MAX_LIGHT_NUM 4

uniform vec4 _color;
uniform sampler2D mainTex;
uniform sampler2D specularTex;

uniform float shininess;
uniform vec3 ambientColor;
uniform mat4 lightMat[4];
uniform int lightNumber = 0;
uniform vec3 camPos;

//uniform vec3 lightPos;
//float lightRadius = 100.0;
//vec3 lightColor = vec3(1.0,0.0,1.0);

vec3 PhongModel();
vec3 DirLightCalc(mat4 light, vec3 viewDir);
vec3 PointLightCalc(mat4 light, vec3 viewDir);
vec3 SpotLightCalc(mat4 light, vec3 viewDir);

void main()
{
    vec3 result = PhongModel();
    FragColor = vec4(result,1.0);
} 

vec3 DirLightCalc(mat4 light, vec3 viewDir)
{
    vec3 lightDir = normalize(-light[1].xyz);

    //diffuse
    float diff = max(dot(IN.normals, lightDir), 0.0);
    //specular
    vec3 reflectDir = reflect(-lightDir, IN.normals);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    // combine results
    vec3 diffuse = light[2].xyz * diff * texture(mainTex, IN.uv0).rgb;
    vec3 specular = light[2].xyz * spec * texture(specularTex, IN.uv0).rgb;
    return (diffuse + specular);
}

vec3 PointLightCalc(mat4 light, vec3 viewDir)
{
    vec3 lightDir = normalize(light[0].xyz - IN.pos);

    //diffuse
    float diff = max(dot(IN.normals, lightDir), 0.0);

    //specular
    vec3 reflectDir = reflect(-lightDir, IN.normals);
    float spec = pow(max(dot(viewDir, reflectDir),0.0), 64.0);

    //atenuation
    float distance = length(light[0].xyz - IN.pos);
    if(distance > light[3].w)
        return vec3(0,0,0);

    float attenuation = 1.0 / (light[3].z + light[3].y * distance + light[3].x * (distance * distance));  

    //combine result
    vec3 diffuse = light[2].xyz *  diff * texture(mainTex, IN.uv0).rgb;
    vec3 specular = light[2].xyz *  spec * texture(specularTex, IN.uv0).rgb;
    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse + specular);
}

vec3 SpotLightCalc(mat4 light, vec3 viewDir)
{
    vec3 lightDir = normalize(light[0].xyz - IN.pos);

    //diffuse
    float diff = max(dot(IN.normals, lightDir), 0.0);

    //specular
    vec3 reflectDir = reflect(-lightDir, IN.normals);
    float spec = pow(max(dot(viewDir, reflectDir),0.0), 64.0);

    //attenuation
    float distance = length(light[0].xyz - IN.pos);
    float attenuation = 1.0 / (light[3].z + light[3].y * distance + light[3].x * (distance * distance));  

    float theta = dot(lightDir, normalize(-light[1].xyz));
    float epsilon = light[1].w - light[2].w;
    float intensity = clamp((theta - light[2].w) / epsilon, 0.0, 1.0);

    //combine result
    vec3 diffuse = light[2].xyz * diff * texture(mainTex, IN.uv0).rgb;
    vec3 specular = light[2].xyz * spec * texture(specularTex, IN.uv0).rgb;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return(diffuse + specular);
}

vec3 PhongModel()
{
    vec3 viewDir = normalize(camPos - IN.pos);
    vec3 returnLight = texture(mainTex, IN.uv0).xyz * ambientColor;
    int clampedLightNumber = clamp(lightNumber, 0, 4);
   
    for(int i=0;i< clampedLightNumber;i++)
    {
        mat4 light = lightMat[i];
        int type = int(light[0].w);

        if(type == 0)
            returnLight += DirLightCalc(light, viewDir);

        if(type == 1)
            returnLight += PointLightCalc(light, viewDir);

        if(type == 2)
            returnLight += SpotLightCalc(light, viewDir);
    }

    return returnLight;
}