#version 150

in vec3 vNormal;

uniform mat4 modelMat;
uniform mat4 projMat;

void main(void)
{
	gl_Position = projMat * modelMat * vec4(vNormal, 1.0);
}
