#vertex
#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec4 aTangents;
layout (location = 3) in vec2 aUV0;

out vec2 uv0;

void main()
{
    uv0 = aUV0;
    gl_Position = vec4(aPos, 1.0);
}

//---------------------------------------------------
#fragment
#version 450 core

in vec2 uv0;
out vec4 FragColor;

uniform sampler2D inputTex;
uniform vec2 texelSize;

void main()
{
	vec2 halfpixel = texelSize.xy;

	vec4 sum = vec4(0.0);

	sum += (2.0 / 16.0) * texture2D(inputTex, uv0 + vec2(-halfpixel.x,  0.0) );
	sum += (2.0 / 16.0) * texture2D(inputTex, uv0 + vec2( 0.0,          halfpixel.y) );
	sum += (2.0 / 16.0) * texture2D(inputTex, uv0 + vec2( halfpixel.x,  0.0) );
	sum += (2.0 / 16.0) * texture2D(inputTex, uv0 + vec2( 0.0,         -halfpixel.y) );

	sum += (1.0 / 16.0) * texture2D(inputTex, uv0 + vec2(-halfpixel.x, -halfpixel.y) );
	sum += (1.0 / 16.0) * texture2D(inputTex, uv0 + vec2(-halfpixel.x,  halfpixel.y) );
	sum += (1.0 / 16.0) * texture2D(inputTex, uv0 + vec2( halfpixel.x, -halfpixel.y) );
	sum += (1.0 / 16.0) * texture2D(inputTex, uv0 + vec2( halfpixel.x,  halfpixel.y) );

	sum += (4.0 / 16.0) * texture2D(inputTex, uv0);

	FragColor = vec4(sum.rgb, 1.0f);
}