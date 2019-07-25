#version 330

in vec3 normal;
in vec2 uv;

uniform sampler2D uTexture;
uniform vec3 uLightWorld;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(uTexture, uv);
}
