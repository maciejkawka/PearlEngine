#vertex

#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec3 aTangents;
layout (location = 3) in vec2 aUV0;

uniform mat4 VPMatrix;
uniform mat4 modelMatrix;

uniform vec2 mainTex_scale = vec2(1,1);
uniform vec2 mainTex_offset = vec2(0, 0);

out Vertex{
    vec3 pos;
    vec3 normals;
    vec3 tangents;
    vec3 bitangents;
    vec2 uv0;
} OUT;


void main()
{
    mat3 normalMat = transpose(inverse(mat3(modelMatrix)));
    OUT.normals = normalize(normalMat * normalize(aNormals)); 
    OUT.tangents = normalize(normalMat * normalize(aTangents.xyz));
    OUT.uv0 = aUV0 * mainTex_scale + mainTex_offset;
    OUT.bitangents = cross(OUT.tangents, OUT.normals);

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
    vec3 tangents;
    vec3 bitangents;
    vec2 uv0;
} IN;

#define MAX_LIGHT_NUM 4

uniform vec4 _color;
uniform sampler2D mainTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;

uniform float shininess;
uniform vec3 ambientColor;
uniform mat4 lightMat[4];
uniform int lightNumber = 0;
uniform vec3 camPos;

bool useMainTex = true;
bool useNormalTex = true;
bool useSpecularTex = true;

vec3 PhongModel();
vec3 DirLightCalc(mat4 light, vec3 viewDir, vec3 normalMap);
vec3 PointLightCalc(mat4 light, vec3 viewDir, vec3 normalMap);
vec3 SpotLightCalc(mat4 light, vec3 viewDir, vec3 normalMap);

void main()
{
    if(length(texture(mainTex, IN.uv0).rgb) == 0)
        useMainTex = false;
    if(length(texture(specularTex, IN.uv0).rgb) == 0)
        useSpecularTex = false;   
    if(length(texture(normalTex, IN.uv0).rgb) == 0)
        useNormalTex = false;

    vec3 result = PhongModel();
    FragColor = vec4(result,1.0);
} 

vec3 DirLightCalc(mat4 light, vec3 viewDir, vec3 normalMap)
{
    vec3 lightDir = normalize(-light[1].xyz);

    //diffuse
    float diff = max(dot(normalMap, lightDir), 0.0);
    //specular
    vec3 reflectDir = reflect(-lightDir, normalMap);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);

    vec3 specularMap = vec3(1,1,1);
    if(useSpecularTex)
        specularMap = texture(specularTex, IN.uv0).rgb;

    // combine results
    vec3 diffuse = light[2].xyz * diff * texture(mainTex, IN.uv0).rgb;
    vec3 specular = light[2].xyz * spec * specularMap;
    return (diffuse + specular);
}

vec3 PointLightCalc(mat4 light, vec3 viewDir, vec3 normalMap)
{
    vec3 lightDir = normalize(light[0].xyz - IN.pos);

    //diffuse
    float diff = max(dot(normalMap, lightDir), 0.0);

    //specular
    vec3 reflectDir = reflect(-lightDir, normalMap);
    float spec = pow(max(dot(viewDir, reflectDir),0.0), 64);

    //atenuation
    float distance = length(light[0].xyz - IN.pos);
    if(distance > light[3].w)
        return vec3(0,0,0);

    float attenuation = 1.0 / (light[3].z + light[3].y * distance + light[3].x * (distance * distance));  

    vec3 specularMap = vec3(1,1,1);
    if(useSpecularTex)
        specularMap = texture(specularTex, IN.uv0).rgb;

    //combine result
    vec3 diffuse = light[2].xyz *  diff * texture(mainTex, IN.uv0).rgb;
    vec3 specular = light[2].xyz *  spec * specularMap;
    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse + specular);
}

vec3 SpotLightCalc(mat4 light, vec3 viewDir, vec3 normalMap)
{
    vec3 lightDir = normalize(light[0].xyz - IN.pos);

    //diffuse
    float diff = max(dot(normalMap, lightDir), 0.0);

    //specular
    vec3 reflectDir = reflect(-lightDir, normalMap);
    float spec = pow(max(dot(viewDir, reflectDir),0.0), 64);

    //attenuation
    float distance = length(light[0].xyz - IN.pos);
    float attenuation = 1.0 / (light[3].z + light[3].y * distance + light[3].x * (distance * distance));  

    float theta = dot(lightDir, normalize(-light[1].xyz));
    float epsilon = light[1].w - light[2].w;
    float intensity = clamp((theta - light[2].w) / epsilon, 0.0, 1.0);

    vec3 specularMap = vec3(1,1,1);
    if(useSpecularTex)
        specularMap = texture(specularTex, IN.uv0).rgb;

    //combine result
    vec3 diffuse = light[2].xyz * diff * texture(mainTex, IN.uv0).rgb;
    vec3 specular = light[2].xyz * spec * specularMap;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return(diffuse + specular);
}

vec3 PhongModel()
{
    vec3 normalMap = IN.normals;
    if(useNormalTex == true)
    {
        mat3 TBN = mat3(normalize(IN.tangents), normalize(IN.bitangents), normalize(IN.normals));
        normalMap = texture(normalTex, IN.uv0).rgb;
        normalMap = normalize(TBN * normalize(normalMap * 2.0 - 1.0));
    }

    vec3 viewDir = normalize(camPos - IN.pos);
    vec3 returnLight = texture(mainTex, IN.uv0).xyz * ambientColor;
    int clampedLightNumber = clamp(lightNumber, 0, 4);
   
    for(int i=0;i< clampedLightNumber;i++)
    {
        mat4 light = lightMat[i];
        int type = int(light[0].w);

        if(type == 0)
            returnLight += DirLightCalc(light, viewDir, normalMap);

        if(type == 1)
            returnLight += PointLightCalc(light, viewDir, normalMap);

        if(type == 2)
            returnLight += SpotLightCalc(light, viewDir, normalMap);
    }

    return returnLight;
}