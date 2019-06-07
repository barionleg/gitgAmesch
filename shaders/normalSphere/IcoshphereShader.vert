#version 150

in vec3 vPosition;
in float vData;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

uniform sampler2D uSelectionTexture;
uniform int uTextureWidth;

out float data;
out float selected;

void main(void)
{
	data = vData;

	ivec2 texCoord = ivec2( mod(gl_VertexID ,uTextureWidth) , gl_VertexID / uTextureWidth );

	selected = texelFetch(uSelectionTexture, texCoord, 0).r;

	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(vPosition, 1.0);
}
