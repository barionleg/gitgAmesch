#version 150

in vec2 vPosition;

uniform mat4 uProjMat;

uniform vec2 uScreenSize;
uniform vec2 uCursorLocation;
uniform float uSelectionRadius;

void main(void)
{
	vec2 position = (uCursorLocation + vPosition * (uSelectionRadius * 2.0)) / uScreenSize;
	gl_Position = uProjMat * vec4(position, 0.0, 1.0);
}
