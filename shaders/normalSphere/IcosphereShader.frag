#version 150
in float data;
in float selected;

uniform sampler2D uFuncValTexture;
uniform float uColorMapIndex = 0.0f;

uniform float uMaxData;
uniform float uMinData = 0.0f;

out vec4 fragColor;

void main(void)
{
	float funcVal = clamp((data - uMinData) / (uMaxData - uMinData), 0.0, 1.0);
	vec2 funcvalTexCoord = vec2(funcVal, (uColorMapIndex * 10.0f + 5.0f)  / 512.0);

	fragColor = texture(uFuncValTexture, funcvalTexCoord);
	fragColor.rgb = mix(fragColor.rgb, vec3(0,1,0), selected);
}
