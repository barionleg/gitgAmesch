#version 150

/*
	Shader Program to draw a Sphere from a plane.
	The Plane is basically the bounding box
*/

uniform sampler2D uNormalTexture;
uniform sampler2D uFuncValTexture;
uniform sampler2D uSelectionMaskTexture;

uniform float uMaxNormalDensity = 1.0;

in vec3 planePosition;
in vec3 planeDirection;

out vec4 fragColor;


//from: https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
//simplified, because we make a couple of strong assumptions (unit sphere at origin (0.0,0.0,0.0)
float raySphereIntersect(vec3 r0, vec3 rd) {
	// - r0: ray origin
	// - rd: normalized ray direction
	// - Returns distance from r0 to first intersecion with sphere,
	//   or -1.0 if no intersection.
	float b = 2.0 * dot(rd, r0);
	float c = dot(r0, r0) - 1.0;

	float test = b*b - 4.0*c;

	if (test < 0.0) {
		return -1.0;
	}
	return (-b - sqrt(test))/ 2.0;
}

void main(void)
{
	const float PI =  3.14159265359;

	float distToInterSect = raySphereIntersect(planePosition, planeDirection);

	//no sphere intersection. Should not happen...
	if(distToInterSect <= 0.0)
		discard;

	vec3 sphereCoordScreen = planePosition + planeDirection * distToInterSect;

	//convert into spherical coordinates to sample the normal-texture
	vec2 sphereTexCoord;
	sphereTexCoord.x = (atan(sphereCoordScreen.y, sphereCoordScreen.x) / PI + 1.0) * 0.5 ;
	sphereTexCoord.y = acos(sphereCoordScreen.z) / PI;

	//colormap the normal-density
	float density = clamp(texture(uNormalTexture, sphereTexCoord).r / uMaxNormalDensity, 0.0, 1.0);
	vec2 funcvalTexCoord = vec2(density, 105.0 / 512.0); //105 should be morgenstemming

	fragColor = texture(uFuncValTexture, funcvalTexCoord);
	fragColor.rgb = mix(fragColor.rgb, vec3(0.3,1.0,0.2), texture(uSelectionMaskTexture, sphereTexCoord).r * 0.7);
}

