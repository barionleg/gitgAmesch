#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vUV;

out vec3 normal;
out vec3 FixedCam_halfVector;
out vec3 FixedWorld_halfVector;
out vec3 FixedCam_L;
out vec3 FixedWorld_L;
out vec2 uv;

uniform mat4 modelViewMat;
uniform mat4 projectionMat;

// +++ Direction of the light fixed in relation to the camera:
uniform vec3 uLightDirectionFixedCamera = vec3( 0.0, 0.0, 1.0 );
uniform vec3 uLightDirectionFixedWorld  = vec3( 0.0, 0.0, 1.0 );

void main(void)
{
    mat4 mvp = projectionMat * modelViewMat;
	gl_Position = mvp * vec4(vPosition, 1.0);
	normal = (modelViewMat * vec4(vNormal, 0.0)).xyz;
    uv = vUV;

	vec3 E = normalize( -normal );                                  // "eye" vector, which is the vector from the vertex's eye-space position to the origin.
	// ---- Light direction fixed to the camera -----
	FixedCam_L = normalize(uLightDirectionFixedCamera);
	FixedCam_halfVector = normalize(FixedCam_L + E);                  // "half vector"  which is the normalized vector half-way between the light and eye vectors.

	// ---- Light direction fixed to the world/object -----
	FixedWorld_L = normalize( modelViewMat * vec4( uLightDirectionFixedWorld, 0.0 ) ).xyz;
	FixedWorld_halfVector = normalize( FixedWorld_L + E );              // "half vector"  which is the normalized vector half-way between the light and eye vectors.

}
