#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vUV;

out vec3 normal;
out vec2 uv;

uniform mat4 modelViewMat;
uniform mat4 projectionMat;

void main(void)
{
    mat4 mvp = projectionMat * modelViewMat;
	gl_Position = mvp * vec4(vPosition, 1.0);
	normal = (mvp * vec4(vNormal, 0.0)).xyz;
    uv = vUV;
}
