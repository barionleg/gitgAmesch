#version 430

//#define NUM_LAYERS 10
uniform int NUM_LAYERS = 10;

uniform vec2 uViewPortSize;

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
        float vertexFuncVal_normalized; //normalized to 0.0-1.0 for transparency...
        vec2  vertexFuncValTexCoord;
        // +++ Labels
        float labelNr;       // corresponds to vLabelID
        float flagNoLabel;
} gVertex;

//flat in float labelNr;
//flat in float flagNoLabel;

// Switch between flat and smooth shading
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
uniform bool uLightEnabled = true;
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
// --- Light function (Phong) ----------------------------------------------------------------------------------------------------------------------------------
vec4 getLightAmount( vec3 L, vec3 normal, vec3 halfVector, vec4 diffuseProduct, vec4 specularProduct ) { // Product essentially means color
        // Compute terms in the illumination equation for the lights:
        float Kd = max( dot( L, normal ), 0.0 );
        vec4  diffuse = Kd * diffuseProduct;
        if( Kd <= 0.0 ) {
                // specular = vec4( 0.0, 0.0, 0.0, 1.0 );
                return diffuse; // As there is no specular amount.
        }

        float Ks = pow( max( dot( normal,halfVector ), 0.0 ), Shininess );
        vec4  specular = Ks * specularProduct;

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


vec4 shadeFragment()
{
    vec3 normal = normalize(gVertex.normal_interp); // face normal in eyespace:
    if( flatShade ) { // flat shading
            normal = normalize( cross( dFdx( gVertex.ec_pos.xyz ), dFdy( gVertex.ec_pos.xyz ) ) ); // normal of the triangle
    }

    // +++ Add fog (if present):
    float fFogCoord  = 0.0;
    float fFogFactor = 0.0;
    if( fogPresent ) {
            fFogCoord  = abs( gVertex.ec_pos.z / gVertex.ec_pos.w );
            fFogFactor = getFogFactor( fogParams, fFogCoord );
    }
    // Set a default color to be overwritten later:
    vec4 outputColor = vec4( 0.5, 0.0, 0.0, 1.0 );
    // ++++ Compute the illumination equation for the lights:
    vec4 colorLight = vec4( 1.0, 1.0, 1.0, 1.0 );
    if( uLightEnabled ) {
            vec4 fixedCamLight   = getLightAmount( gVertex.FixedCam_L,   normal, gVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
            vec4 fixedWorldLight = getLightAmount( gVertex.FixedWorld_L, normal, gVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
            // ++++ Sum up all parts of the light:
            colorLight = AmbientProduct + fixedCamLight + fixedWorldLight;
            colorLight.a = 1.0;
    }
    // ++++ Solid color:
    outputColor = colorSolid * colorLight;
    // ++++ Color per Vertex:
    if( uRenderColor == 1 ) {
            outputColor  =  gVertex.vertexColor * colorLight;
    }
    // ++++ Function value mapped to color ramp:
    if( uRenderColor == 2 ) {
            vec4 texColor = texture( uFuncValTexMap, gVertex.vertexFuncValTexCoord );
            outputColor  =  texColor * colorLight;
    }
    // ++++ Label ID mapped to color:
    // Shade faces with a certain color:
    if( uRenderColor == 3 ) {
            // Shade labeled areas:
            float labelNrShifted = gVertex.labelNr + uLabelCountOffset;
            float labelIDMod = labelNrShifted - uLabelColorCount * floor( labelNrShifted / uLabelColorCount );
            float labelTexCoordMap = (512.0 - 10.0*uLabelTexMapSel+4.5)/512.0; // Texture map with color ramps is 512x512 pixel and each colorramp is 10 pixel wide (in y).
            float labelTexCoord = (4.5 + 10.0*labelIDMod)/512.0;
            if( uLabelSameColor ) {
                    outputColor  =  uLabelSingleColor * colorLight;
            } else {
                    vec4 texColor = texture( uLabelTexMap, vec2( labelTexCoord, labelTexCoordMap ) );
                    outputColor  =  texColor * colorLight;
            }
            // Shade faces between different labels (or background vertices) differently:
            if( abs( gVertex.labelNr - float( floor( gVertex.labelNr ) ) ) > 0.0 ) {
                    outputColor = uLabelBorderColor * colorLight;
            }
            // Shading color if the fragment is part of the background label:
            if( false ) {
                    outputColor = uLabelBackgroundColor * colorLight;
            }
            // Shade faces not being a label with a given color:
            if( gVertex.flagNoLabel > 0.0 ) {
                    outputColor = uLabelNoColor * colorLight;
            }
    }
    // ++++ Isolines based on the function value:
    /*
    if( funcValIsoLineParams.mIsoLinesShow ) {
            float isoLineIntensity = getIsoLineAlpha( gVertex.vertexFuncVal, funcValIsoLineParams );
            if( funcValIsoLineParams.mIsoLinesOnly ) {
                    if( isoLineIntensity == 0.0 ) {
                            discard;
                    }
                    outputColor   = uIsoSolidColor;
                    outputColor.a = isoLineIntensity;
            } else {
                    vec4  isoLineColor = uIsoSolidColor;
                    if( !uIsoSolidFlag ) {
                            float grayValInv = 1.0 - length( outputColor.xyz );
                            isoLineColor = vec4( grayValInv, grayValInv, grayValInv, 1.0 );
                    }
                    outputColor = mix( outputColor, isoLineColor, isoLineIntensity );
            }
    }
    */
    // +++ Add fog (if present):
    if( fogPresent ) {
            outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
    }
    /*
    // +++ Invert color, when request e.g. for normals:
    if( gInvertColor > 0u ) {
            outputColor.rgb = 1.0 - outputColor.rgb;
    }
    */
    return outputColor;

}


//new stuff for transparency
//-------------------------------

uniform float uUniformAlpha = 0.2;
uniform float uAlpha2 = 1.0;

uniform int uTransparencyType = 0; //0 = uniform transparency, 1 = by vertex color, 2 = by funcval, 3 = by angle, 4 = by label, 5 = normal variation
uniform int uOverflowHandling = 0; // 0 = no handling, 1 = mix additively (mlab)
uniform float uTransLabelNr = 0.0;
uniform float uGamma = 0.2; //exponent in interpolation between uUnifomAlpha and uAlpha2

layout (std430, binding = 0) buffer DepthArray
{
  uint depthVals[];
};

layout (std430, binding = 1) buffer ColorArray
{
  //packed
  uint colorVals[];
  //not packed
  //vec4 colorVals[];
};
//-----------------------------------

layout(location=0) out vec4 oSumTailColor;
layout(location=1) out vec4 oSumTailWeight;

void main(void)
{
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 screenSize = ivec2(uViewPortSize);


    if(!(coords.x>=0 && coords.y>=0 && coords.x<screenSize.x && coords.y<screenSize.y )){
        discard;
    }

    //int offset = coords.x * NUM_LAYERS + coords.y * screenSize.x * NUM_LAYERS; //offset in the Fragment buffer
    uint offset = coords.x + coords.y * screenSize.x;
    uint pageSize = screenSize.x * screenSize.y;

    uint depth = floatBitsToUint(gl_FragCoord.z);



    if(depth > depthVals[offset + (NUM_LAYERS - 1) * pageSize] && uOverflowHandling == 0) {
        discard;
    }

    vec4 fragment;

    fragment = shadeFragment();

    if(uTransparencyType == 0)
        fragment.a = uUniformAlpha;

    else if(uTransparencyType == 2)
        fragment.a = mix(uUniformAlpha, uAlpha2, pow(clamp(gVertex.vertexFuncVal_normalized,0.0,1.0), uGamma));

    else if(uTransparencyType == 3){
        fragment.a = mix(uUniformAlpha, uAlpha2, pow(
                             1.0 - clamp(
                                 abs(dot(normalize(gVertex.normal_interp), vec3(0,0,1.0)))
                                 ,0.0,1.0)
                             ,uGamma)); //angle-based transparency
    }
    else if(uTransparencyType == 4)
    {
        fragment.a = (gVertex.labelNr == uTransLabelNr && !(gVertex.flagNoLabel > 0.0) )? uAlpha2 : uUniformAlpha;
    }

    else if(uTransparencyType == 5)
    {
        float dx = dFdx(gVertex.normal_interp.z);
        float dy = dFdy(gVertex.normal_interp.z);
        float av = clamp(pow(dx * dx + dy * dy,0.5),0.0,1.0);
        fragment.a = mix(uUniformAlpha, uAlpha2, pow(av, uGamma));
    }

    //premultiply alpha
    fragment.rgb *= fragment.a;



    int insertPos = 0;
    int left = 0;
    int right = NUM_LAYERS;

    for(int i = 0; i < log2(NUM_LAYERS + 2) + 1; ++i)
    {
        insertPos = left + ((right - left) / 2);

	if(depthVals[offset + insertPos * pageSize] < depth) {
            left = insertPos + 1;
        }

        else {
            right = insertPos;
        }
    }

/*
    for(int i = 0; i< numLayers; ++i)
    {
        if(depthVals[offset + i] == depth)
            insertPos = i;
    }
*/
    if(insertPos < NUM_LAYERS && depthVals[offset + insertPos * pageSize] == depth)
    {
	//packed:
	colorVals[offset + insertPos * pageSize] = packUnorm4x8(fragment);
	//not packed:
	//colorVals[offset + insertPos * pageSize] = fragment;
	    discard;
    }

    else if (uOverflowHandling == 1)
    {
	//colorVals[offset + numLayers] += fragment;
	//atomicAdd(depthVals[offset + numLayers], 1);
	oSumTailColor = fragment;
	oSumTailWeight = vec4(fragment.a);
    }

    else
    {
	discard;
    }
}

