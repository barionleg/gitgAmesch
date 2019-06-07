#version 150

in vec3 vPosition;
in float vData;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 norm;
out float data;

void main(void)
{
	norm = vPosition;
	data = vData;
	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(vPosition, 1.0);
}
