#version 150
//#extension GL_ARB_shader_bit_encoding : enable

// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;

// This matrix and the following parameters are used to scale, rotate and position unit objects, like box, sphere and cone/cylinder.
uniform highp mat4 uModelViewExtra = mat4( 1.0, 0.0, 0.0, 0.0,
                                           0.0, 1.0, 0.0, 0.0,
                                           0.0, 0.0, 1.0, 0.0,
                                           0.0, 0.0, 0.0, 1.0
                                         );

uniform float uScaleHeight       = 1.0;
uniform float uScaleRadialBottom = 1.0;
uniform float uScaleRadialTop    = 1.0;

// +++ Vertex buffers -- this corresponds to MeshGL::grVertexElmentBasic
in vec3  position;
in vec3  vNormal;
in vec4  vColor;
in float vFuncVal;
// +++ Vertex buffers -- this corresponds to MeshGL::grVertexStripeElment
in float vLabelID; // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
in float vFlags;   // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Direction of the light fixed in relation to the camera:
uniform vec3 uLightDirectionFixedCamera = vec3( 0.0, 0.0, 1.0 );
uniform vec3 uLightDirectionFixedWorld  = vec3( 0.0, 0.0, 1.0 );

// +++ Clipping
uniform vec4 uClipPlane0 = vec4( 0.0, -1.0, 0.0, 0.0 ); // Classic clipping plane, provided by the plane of the Mesh.
uniform vec3 uClipBefore = vec3( 0.0, 0.0, 0.0 );       // Point in world coordinates sed when a single primitve is selected, than everything in front of it is clipped.

// +++ Function value of the vertex - values to compute the texture coordinate for the fragment shader:
uniform bool  uFuncValInvert = false;
uniform float uFuncValMin;
uniform float uFuncValMax;
uniform float uFuncValColorMap  = 1.0;
uniform bool  uFuncValRepeat    = false;
uniform float uFuncValIntervall = 10.0;
uniform float uFuncValLogGamma  =  1.0;

// +++ Values to be passed on:
out struct grVertex {
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

//flat out float labelNr;
//flat out float flagNoLabel;
uniform float uFaceShiftViewZ = 0.0; // offset in view coordinates to prevent z-fighting. e.g. when selected faces are drawn.

void main(void)
{
    highp mat4  modelViewMat  = modelview * uModelViewExtra;
          float scaleRadial   = uScaleRadialBottom * ( 0.5 - position.z ) + uScaleRadialTop * ( 0.5 + position.z );
          vec3  modelPosition = vec3( position.x*scaleRadial, position.y*scaleRadial, position.z * uScaleHeight );

    gVertex.ec_pos        = modelViewMat * vec4( modelPosition.xyz, 1.0 );//modelViewMat * vec4( modelPosition, 1.0 );                  // Transform the vertex position into the eye coordinate system.
    gVertex.normal_interp = normalize( modelViewMat * vec4( vNormal, 0.0 ) ).xyz;  // Transform the vertex normal into eye coordinates.
    vec3 E = normalize( -gVertex.ec_pos.xyz );                                  // "eye" vector, which is the vector from the vertex's eye-space position to the origin.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the camera -----
    gVertex.FixedCam_L = normalize( uLightDirectionFixedCamera );
    gVertex.FixedCam_halfVector = normalize( gVertex.FixedCam_L + E );                  // "half vector"  which is the normalized vector half-way between the light and eye vectors.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the world/object -----
    gVertex.FixedWorld_L = normalize( modelViewMat * vec4( uLightDirectionFixedWorld, 0.0 ) ).xyz;
    gVertex.FixedWorld_halfVector = normalize( gVertex.FixedWorld_L + E );              // "half vector"  which is the normalized vector half-way between the light and eye vectors.

    gl_Position   = projection * gVertex.ec_pos;
    gl_ClipDistance[0] = dot( uClipPlane0, uModelViewExtra * vec4( modelPosition, 1.0 ) );     //  using "ec_pos" instead of "vec4( position, 1.0 )" will clip in view space!
    gl_ClipDistance[2] = ( modelview * vec4( uClipBefore, 1.0 ) ).z - gVertex.ec_pos.z;
    gVertex.vertexColor   = vColor;
    gVertex.vertexFuncVal = vFuncVal;
    gVertex.vertexFuncVal_normalized = ( vFuncVal - uFuncValMin ) / ( uFuncValMax - uFuncValMin );
    gVertex.labelNr       = vLabelID;

    // Or more generally for pixel i in a N wide texture the proper texture coordinate is :=  (2i + 1)/(2N)
    float iPixel = 10.0*uFuncValColorMap+4.0;
    gVertex.vertexFuncValTexCoord.t = ( 2.0*iPixel + 1.0 )/1024.0; // Texture map with color ramps is 512x512 pixel and each colorramp is 10 pixel wide (in y).
    // Repeat color ramp, when requested:
    if( uFuncValRepeat ) {
            float funcValForTexCoords = 2.0 * mod( vFuncVal, uFuncValIntervall ) / uFuncValIntervall;
            if( funcValForTexCoords > 1.0 ) {
                    funcValForTexCoords = ( -funcValForTexCoords + 2.0 );
            }
            gVertex.vertexFuncValTexCoord.s = ( 511.0 * funcValForTexCoords + 0.5 ) / 512.0;
    } else {
            float normRange = ( vFuncVal - uFuncValMin ) / ( uFuncValMax - uFuncValMin );
            if( normRange < 0.0 ) {
                    normRange = 0.0;
            }
            if( normRange > 1.0 ) {
                    normRange = 1.0;
            }
            normRange = pow( normRange, uFuncValLogGamma );
            gVertex.vertexFuncValTexCoord.s = ( 511.0 * normRange ) / 512.0 + 0.5 / 512.0;
    }
    if( uFuncValInvert ) {
            gVertex.vertexFuncValTexCoord.s = 1.0 - gVertex.vertexFuncValTexCoord.s;
    }

    // Prevent z-fighting
    gl_Position.z += uFaceShiftViewZ;

    // EXAMPLE for passing on flags:
    // See MeshGL::vboPrepareVerticesStriped() for the values used to pass the flags.
    if( bool( uint(vFlags) & 0x08u )  ) {
            gVertex.flagNoLabel = 1.0;
    } else {
            gVertex.flagNoLabel = 0.0;
    }

}
