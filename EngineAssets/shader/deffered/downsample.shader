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
uniform vec4 threshold;

const float epsilon = 1.0e-4;

vec4 quadratic_threshold(vec4 color, float threshold, vec3 curve)
{
	// Pixel brightness
    float br = max(color.r, max(color.g, color.b));

    // Under-threshold part: quadratic curve
    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    // Combine and apply the brightness response curve.
    color *= max(rq, br - threshold) / max(br, epsilon);

    return color;
}

void main()
{
	vec2 halfpixel = 0.5f * vec2(texelSize.x, texelSize.y);
	vec2 oneepixel = 1.0f * vec2(texelSize.x, texelSize.y);

	vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);

	sum += (4.0/32.0) * texture2D(inputTex, uv0).rgba;

	sum += (4.0/32.0) * texture2D(inputTex, uv0 + vec2(-halfpixel.x, -halfpixel.y) );
	sum += (4.0/32.0) * texture2D(inputTex, uv0 + vec2(+halfpixel.x, +halfpixel.y) );
	sum += (4.0/32.0) * texture2D(inputTex, uv0 + vec2(+halfpixel.x, -halfpixel.y) );
	sum += (4.0/32.0) * texture2D(inputTex, uv0 + vec2(-halfpixel.x, +halfpixel.y) );

	sum += (2.0/32.0) * texture2D(inputTex, uv0 + vec2(+oneepixel.x, 0.0) );
	sum += (2.0/32.0) * texture2D(inputTex, uv0 + vec2(-oneepixel.x, 0.0) );
	sum += (2.0/32.0) * texture2D(inputTex, uv0 + vec2(0.0, +oneepixel.y) );
	sum += (2.0/32.0) * texture2D(inputTex, uv0 + vec2(0.0, -oneepixel.y) );

	sum += (1.0/32.0) * texture2D(inputTex, uv0 + vec2(+oneepixel.x, +oneepixel.y) );
	sum += (1.0/32.0) * texture2D(inputTex, uv0 + vec2(-oneepixel.x, +oneepixel.y) );
	sum += (1.0/32.0) * texture2D(inputTex, uv0 + vec2(+oneepixel.x, -oneepixel.y) );
	sum += (1.0/32.0) * texture2D(inputTex, uv0 + vec2(-oneepixel.x, -oneepixel.y) );

	//Uncomment if bloom creates artefacts
	// if(isnan(quadratic_threshold(sum, threshold.x, threshold.yzw).rgb) == true)
	// {
	// 	FragColor = vec4(1.0f);
	// 	return;
	// }

    FragColor = vec4(quadratic_threshold(sum, threshold.x, threshold.yzw).rgb, 1.0f);
}