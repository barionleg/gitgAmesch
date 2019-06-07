#version 150

//uniform float uScaleX   = 1.0;    // Scale for x-axis -- typically mm/pixel.
//uniform float uScaleY   = 1.0;    // Scale for y-axis -- should always be the same value as uScaleX.
uniform mat4 modelMat;

// +++ Vertex buffers
in vec2 vertPosition;        // Positions of the vertices defining the screenquad

out vec3 planePosition;	 //artificial view-plane position
out vec3 planeDirection; //artidicial view-plane direction

void main(void) {
		gl_Position = vec4( vertPosition, 0.0, 1.0 );

		planePosition = vec3(modelMat * vec4(vertPosition ,1.5, 1.0));
		planeDirection = mat3(modelMat) *vec3(0.0,0.0, -1.0);
}
