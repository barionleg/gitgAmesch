#version 150

// Backfaces -- glDisable( GL_CULL_FACE ); has to be set
uniform bool  backCulling    = true;
uniform bool  backLighting   = true;
uniform vec4  colorSolidBack = vec4( 128.0/255.0, 92.0/255.0, 92.0/255.0, 1.0 );

// Switch betwenn flat and smooth shading
uniform bool  flatShade = false;

// Selection for the rendered color:
// 1 ... Color per vertex
// 2 ... Function values
// 3 ... Labels
// in all other case, the solid color is used.
uniform int       uRenderColor = 0;

// Label settings:
uniform sampler2D uLabelTexMap;                                          // Texturemap storing the label colors.
uniform float     uLabelTexMapSel       =  1.0;                          // Selected row within the texture map.
uniform float     uLabelColorCount      = 11.0;                          // Number of colors available within the selected row of the texture map.
uniform float     uLabelCountOffset     =  0.0;                          // Offset to shift the color with the row of the texture map.
uniform vec4      uLabelBorderColor     = vec4( 0.25, 0.25, 0.25, 1.0 ); // Color for connected components tagged as background
uniform vec4      uLabelBackgroundColor = vec4( 0.25, 0.25, 0.25, 1.0 ); // Color for connected components tagged as background
uniform vec4      uLabelNoColor         = vec4( 0.61, 0.09, 0.09, 1.0 ); // Color for areas not labeled - i.e. not being part of a connect component nor the backgroud
uniform vec4      uLabelSingleColor     = vec4( 0.00, 0.00, 0.00, 1.0 ); // Show all labels with the same color (instead of using sampler2D uLabelTexMap).
uniform bool      uLabelSameColor       = false;                         // Show all labels with the same color (instead of using sampler2D uLabelTexMap).

// Solid color
uniform vec4  colorSolid     = vec4( 186.0/255.0, 186.0/255.0, 186.0/255.0, 1.0 );
// Properties for Phong shading
uniform vec4  AmbientProduct = vec4( 1.0, 1.0, 1.0, 1.0 ); // vec4( 0.1, 0.1, 0.1, 1.0 );
uniform float Shininess      = 14.154; // exp(2.65)

// Properties for Phong shading related to light sources
uniform bool uLightEnabled = false;
uniform vec4 FixedCam_DiffuseProduct;
uniform vec4 FixedCam_SpecularProduct;
uniform vec4 FixedWorld_DiffuseProduct;
uniform vec4 FixedWorld_SpecularProduct;

// Fog
uniform bool fogPresent = false;
uniform struct FogParameters {
	vec4  vFogColor; // Fog color
	float fStart;    // This is only for linear fog
	float fEnd;      // This is only for linear fog
	float fDensity;  // For exp and exp2 equation
	int   iEquation; // 0 = linear, 1 = exp, 2 = exp2 -- see defines GLSL_FOG_EQUATION_*
} fogParams;

// Function value of the vertex
uniform sampler2D uFuncValTexMap;

// Isolines for function values
uniform struct grFuncValIsoLineParams {
	bool  mIsoLinesShow;
	bool  mIsoLinesOnly;
	float mIsoDist;
	float mIsoOffset;
	float mIsoPixelWidth;
} funcValIsoLineParams;
uniform bool uIsoSolidFlag  = true;
uniform vec4 uIsoSolidColor = vec4( 0.0, 0.0, 0.0, 1.0 );

// +++ Values to be passed from the vertex or geometry shader
in struct grVertex {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
	//+++ Color of the vertex
	vec4  vertexColor;
	// +++ Function value of the vertex passed to the fragment shader:
	float vertexFuncVal;
	vec2  vertexFuncValTexCoord;
	// +++ Labels
	float labelNr;       // corresponds to vLabelID
	float flagNoLabel; 
} gVertex;

flat in uint gInvertColor;

// +++ Edge/Wireframe Rendering
noperspective in vec3 vEdgeDist;                           // Barycenter coordinates.

in vec3 vBarycenter;            // normalized Barycenter coordinates
flat in vec3 vLabelNumbers;                        // vector to hold all three labelNr's to get uninterpolated result

uniform vec4 uEdgeColor      = vec4( 0.1, 0.1, 0.1, 1.0 ); // Color of the edge
uniform bool uEdgeShown      = false;

// Output i.e. color of the fragment
out vec4 fragColor;

// --- Light function (Phong) ----------------------------------------------------------------------------------------------------------------------------------
vec4 getLightAmount( vec3 L, vec3 normal, vec3 halfVector, vec4 diffuseProduct, vec4 specularProduct ) { // Product essentially means color
	// Compute terms in the illumination equation for the lights:
	float Kd = max( dot( L, normal ), 0.0 );
	vec4  diffuse = Kd * diffuseProduct;
	float Ks = pow( max( dot( normal,halfVector ), 0.0 ), Shininess );
	vec4  specular = Ks * specularProduct;
	if( dot( L,normal ) < 0.0 ) {
		// specular = vec4( 0.0, 0.0, 0.0, 1.0 );
		return diffuse; // As there is no specular amount.
	}
	vec4 lightTotal = diffuse + specular;
	return lightTotal;
}

// --- Fog function ---------------------------------------------------------------------------------------------------------------------------------------------
float getFogFactor( FogParameters params, float fFogCoord ) {
	float fResult = 0.0;
	if( params.iEquation == 0 ) {
		fResult = ( params.fEnd-fFogCoord )/( params.fEnd-params.fStart );
	} else if( params.iEquation == 1 ) {
		fResult = exp(-params.fDensity*fFogCoord );
	} else if( params.iEquation == 2 ) {
		fResult = exp(-pow(params.fDensity*fFogCoord, 2.0));
	}
	fResult = 1.0 - clamp( fResult, 0.0, 1.0 );
	return fResult;
}

// --- Isoline -------------------------------------------------------------------------------------------------------------------------------------------------
float getIsoLineAlpha( float vertexFuncVal, grFuncValIsoLineParams rFuncValIsoLineParams ) {
	float isoDist    = rFuncValIsoLineParams.mIsoDist;
	float isoOffset  = rFuncValIsoLineParams.mIsoOffset;
	float pixelWidth = rFuncValIsoLineParams.mIsoPixelWidth;
	float funcValModIsoDist = mod(vertexFuncVal + isoOffset , isoDist);
	float pixelWidthNormalized = pixelWidth * fwidth(vertexFuncVal);

	float funcValRemainder = funcValModIsoDist - pixelWidthNormalized;
	float alphaValue = 0.0;
	if( funcValRemainder <= 0.0 ) {
		alphaValue = -funcValRemainder/pixelWidthNormalized;
	}
	float funcValRemainderPos = funcValModIsoDist + pixelWidthNormalized - isoDist;
	if( funcValRemainderPos >= 0.0 ) {
		alphaValue = +funcValRemainderPos/pixelWidthNormalized;
	}
	return alphaValue;
}


// --- MAIN ----------------------------------------------------------------------------------------------------------------------------------------------------
void main(void) {
	vec3 normal = gVertex.normal_interp; // face normal in eyespace:

	if( flatShade ) // flat shading
	{
		normal = normalize( cross( dFdx( gVertex.ec_pos.xyz ), dFdy( gVertex.ec_pos.xyz ) ) ); // normal of the triangle
	}

	if(!gl_FrontFacing)
	{
		normal = -normal;
	}

	// ++++ Compute the illumination equation for the lights:
	vec4 colorLight = vec4( 1.0, 1.0, 1.0, 1.0 );
	if( uLightEnabled )
	{
		vec4 fixedCamLight   = getLightAmount( gVertex.FixedCam_L,   normal, gVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
		vec4 fixedWorldLight = getLightAmount( gVertex.FixedWorld_L, normal, gVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
		// ++++ Sum up all parts of the light:
		colorLight = AmbientProduct + fixedCamLight + fixedWorldLight;
		colorLight.a = 1.0;
	}

	// Set a default color to be overwritten later:
	vec4 outputColor = vec4( 0.5, 0.0, 0.0, 1.0 );

	if( gl_FrontFacing ) {
	// ++++ Solid color:
		outputColor = colorSolid;
	// ++++ Color per Vertex:
		if( uRenderColor == 1 ) {
			outputColor  =  gVertex.vertexColor;
		}
	// ++++ Function value mapped to color ramp:
		if( uRenderColor == 2 ) {
			vec4 texColor = texture( uFuncValTexMap, gVertex.vertexFuncValTexCoord );
			outputColor  =  texColor;
		}
	// ++++ Label ID mapped to color:
		// Shade background faces:
		// ... todo ...
		// Shade faces with a certain color:
		if( uRenderColor == 3 ) {
			int labelIndex = vBarycenter[0] > vBarycenter[1] ? 0 : 1;
			if(vBarycenter[2] > vBarycenter[labelIndex])
				labelIndex = 2;

			float labelNr = vLabelNumbers[labelIndex];
			// Shade labeled areas:
			float labelNrShifted = round(labelNr) + uLabelCountOffset;
			float labelIDMod = mod(labelNrShifted , uLabelColorCount);
			float labelTexCoordMap = (512.0 - 10.0*uLabelTexMapSel+4.5)/512.0; // Texture map with color ramps is 512x512 pixel and each colorramp is 10 pixel wide (in y).
			float labelTexCoord = (4.5 + 10.0*labelIDMod)/512.0;
			if( uLabelSameColor ) {
				outputColor  =  uLabelSingleColor;
			} else {
				vec4 texColor = texture( uLabelTexMap, vec2( labelTexCoord, labelTexCoordMap ) );
				outputColor  =  texColor;
			}
			// Shade faces between different labels (or background vertices) differently:
			if( fwidth( labelNr ) > 0.0 ) {	//cannot compare to 0.0 because of noise generated by fragment interpolation
				outputColor = uLabelBorderColor;
			}
			// Shading color if the fragment is part of the background label:
			if( false ) {
				outputColor = uLabelBackgroundColor;
			}
			// Shade faces not being a label with a given color:
			if( gVertex.flagNoLabel > 0.0 ) {
				outputColor = uLabelNoColor;
			}
		}

	// +++ Invert color, when request e.g. for normals:
		if( gInvertColor > 0u ) {
			outputColor.rgb = 1.0 - outputColor.rgb;
		}	
	}
	else if( backCulling ) // Backfaces color & culling
	{
		if(!backLighting)
			colorLight = vec4(1.0);

		vec4 outputColor  = colorSolidBack;
	}
	else
	{ // No backfaces at all.
		discard;
	}

	outputColor *= colorLight;

	// ++++ Isolines based on the function value:
	if( funcValIsoLineParams.mIsoLinesShow)
	{
		float isoLineIntensity = getIsoLineAlpha( gVertex.vertexFuncVal, funcValIsoLineParams );
		if( funcValIsoLineParams.mIsoLinesOnly ) {
			if( isoLineIntensity == 0.0 ) {
				discard;
			}
			outputColor   = uIsoSolidColor;
			outputColor.a = isoLineIntensity;
		}
		else if (gl_FrontFacing)
		{
			vec4  isoLineColor = uIsoSolidColor;
			if( !uIsoSolidFlag )
			{
				float grayValInv = 1.0 - length( outputColor.xyz );
				isoLineColor = vec4( grayValInv, grayValInv, grayValInv, 1.0 );
			}
			outputColor = mix( outputColor, isoLineColor, isoLineIntensity );
		}
	}

	// +++ Edge/Wireframe Rendering
	float edgeIntensity = 0.0;
	if( uEdgeShown ) {
		float nearD = min( min( vEdgeDist[0], vEdgeDist[1] ), vEdgeDist[2] );  // determine frag distance to closest edge
		edgeIntensity = exp2( -1.0*nearD*nearD );         // -1.0 correlates to the width - the smaller the number the broader the line.
	}
	outputColor = mix( outputColor, uEdgeColor, edgeIntensity );

	// +++ Add fog (if present):
	float fFogCoord  = 0.0;
	float fFogFactor = 0.0;
	if( fogPresent ) {
		fFogCoord  = abs( gVertex.ec_pos.z / gVertex.ec_pos.w );
		fFogFactor = getFogFactor( fogParams, fFogCoord );
	}
	if( fogPresent ) {
		outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
	}

	fragColor = outputColor;
}
