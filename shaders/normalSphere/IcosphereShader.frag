#version 150

out vec4 fragColor;

uniform sampler2D uFuncValTexture;

in vec3 norm;
in float data;

uniform float uMaxData;
uniform float uMinData = 0.0f;


void main(void)
{
	float funcVal = clamp((data - uMinData) / (uMaxData - uMinData), 0.0, 1.0);

	vec2 funcvalTexCoord = vec2(funcVal, 115.0 / 512.0); //105 should be morgenstemming

	fragColor = texture(uFuncValTexture, funcvalTexCoord);
	fragColor.a = 1.0;

}
