#version 150

in vec3 vPosition;
in float vData;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

uniform sampler2D uSelectionTexture;
uniform int uTextureWidth;

out float data;
out float selected;


uniform float uMaxData;
uniform float uMinData = 0.0f;
uniform float uNormalScale = 1.0f;

void main(void)
{
	data = vData;

	float scale = clamp((data - uMinData) / (uMaxData - uMinData), 0.0, 1.0) * (1.0 - uNormalScale) +  uNormalScale;
	ivec2 texCoord = ivec2( mod(gl_VertexID ,uTextureWidth) , gl_VertexID / uTextureWidth );

	selected = texelFetch(uSelectionTexture, texCoord, 0).r;

	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(vPosition * scale, 1.0);
}
