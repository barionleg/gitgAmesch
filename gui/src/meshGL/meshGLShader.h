/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESHGLSHADER_H
#define MESHGLSHADER_H

#include "meshGL.h"

#include <QImage>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions_4_3_Core>
#include "PinRenderer.h"
#include "TexturedMeshRenderer.h"
//!
//! \brief Shader extension for the Mesh class. (Layer 1.5)
//!
//! Extends the MeshGL class with rendering using shaders written in GLSL.
//!
//! Layer 1.5
//!

class MeshGLShader : public MeshGL {

public:
	MeshGLShader( QGLContext* rGLContext, const std::filesystem::path& rFileName, bool& rReadSuccess );
	~MeshGLShader();

	virtual void glPaint();
	virtual void glPaintTransparent();
	virtual void glPaintOverlay();
	virtual void glPaintDepth( const QMatrix4x4 &rTransformMat );
	virtual void glPaintFrontalLightPerVertex( const QMatrix4x4 &rTransformMat, GLint rXResolution, GLint rYResolution, GLuint rDepthTexture, GLfloat rZTolerance, GLint rFirstVertIdx );

private:
	bool shaderLink(QOpenGLShaderProgram** rShaderProgram, const QString& rVertSrc, const QString& rGeomSrc, const QString& rFragSrc, const QString& rName );

	// Parameters for rendering the fog using shaders.
	enum eGLSLFogFunctions {
		GLSL_FOG_EQUATION_LINEAR = 0, //!< Linear fog density.
		GLSL_FOG_EQUATION_EXP    = 1, //!< Exponential fog density.
		GLSL_FOG_EQUATION_EXP2   = 2  //!< exp2 fog density.
	};

	// Shader programs
	QOpenGLShaderProgram* mShaderBoundingBox;           //!< Shader program rendering the bounding box of the mesh.
	QOpenGLShaderProgram* mShaderVertexSprites;         //!< Shader program rendering only vertices as sprites.
	QOpenGLShaderProgram* mShaderVertexNormals;         //!< Shader program rendering only normals of vertices.
	QOpenGLShaderProgram* mShaderVertexFuncValProgram;  //!< Shader program rendering a mesh with colors mapped to the (generic) function value.
	QOpenGLShaderProgram* mShaderWireframe;             //!< Shader program rendering a mesh as wireframe.
	QOpenGLShaderProgram* mShaderPolyLines;             //!< Shader program rendering cones using two vertices with radii as input.

	QOpenGLShaderProgram* mShaderNPR_BuildFBO;          //!< Shader program generating a FBO texture for NPR-sketch-shading
	QOpenGLShaderProgram* mShaderNPR_BuildSobel;        //!< Shader program generating the sobel image
	QOpenGLShaderProgram* mShaderNPR_gaussianBlur;      //!< Shader program to blur an image using a 3x3 gausian kernel
	QOpenGLShaderProgram* mShaderNPR_hatching;          //!< Shader program to generate hatch-shading
	QOpenGLShaderProgram* mShaderNPR_toonify;           //!< Shader program to generate toon-like images
	QOpenGLShaderProgram* mShaderNPR_composit;          //!< Shader program tom composit multiple NPR textures into a final image

	QOpenGLShaderProgram* mShaderTransparencyABClear;   //!< Shader program to clear Buffers for transparency (ABuffer variant)
	QOpenGLShaderProgram* mShaderTransparencyABFill;    //!< Shader program to fill fragment buffers for transparency (ABuffer variant)
	QOpenGLShaderProgram* mShaderTransparencyABRender;  //!< Shader program to render fragment buffer for transparency (ABuffer variant)
	QOpenGLShaderProgram* mShaderTransparencyCountFrags;//!< Shader program to count the maximal number of fragments generated of an object

	QOpenGLShaderProgram* mShaderTransparencyALClear;   //!< Shader program to clear Buffers for transparency (Atomic Loop variant)
	QOpenGLShaderProgram* mShaderTransparencyALFill;    //!< Shader program to fill fragment buffers for transparency (Atomic Loop variant)
	QOpenGLShaderProgram* mShaderTransparencyALRender;  //!< Shader program to render fragment buffer for transparency (Atomic Loop variant)
	QOpenGLShaderProgram* mShaderTransparencyALDepthCollect;//!< Shader program to collect depth-values (Atomic Loop)

	QOpenGLShaderProgram* mShaderTransparencyGeomWOIT;  //!< Geometry pass of Weighted OIT
	QOpenGLShaderProgram* mShaderTransparencyBlendWOIT; //!< Blending pass of Weighted OIT

	QOpenGLShaderProgram* mShaderLightToFBO;            //!< Geometry pass to create Geometry-Buffer with lighting information
	QOpenGLShaderProgram* mShaderPaintLightningOverlay; //!< Shader program to paint lightning information of over-/under-lit areas
	QOpenGLShaderProgram* mShaderDepth;                 //!< Shader program for just filling the depth buffer
	QOpenGLShaderProgram* mShaderFrontalLightPerVertex; //!< Shader program for painting per vertex light intensities (obtained by simulating frontal lighting) to individual pixels

	QOpenGLShaderProgram* mShaderPointcloud;            //!< Shader program for painting point-clouds

	// Texture maps used by the shader programs
	QOpenGLTexture* mTextureMaps[1];  //!< Texturemap IDs for the shader. Currently there is just one holding the color ramps.
	QImage          mFuncValTexMapGL; //!< QImage holding the colorramps for rendering the function values. Has to be formatted for use with OpenGL!

	// Set Locations (without init)
	void shaderSetLocationBasicMatrices( QOpenGLShaderProgram* rShaderProgram );
	void shaderSetLocationBasicAttribs(  QOpenGLShaderProgram* rShaderProgram, eVertBufObjs rBufferId, int rStride );
	void shaderSetLocationBasicFaces(    QOpenGLShaderProgram* rShaderProgram );
	void shaderSetLocationBasicLight(    QOpenGLShaderProgram* rShaderProgram );
	void shaderSetLocationBasicFog(      QOpenGLShaderProgram* rShaderProgram );
	void shaderSetLocationVertexSprites( QOpenGLShaderProgram* rShaderProgram );

	void shaderSetMeshBasicFuncVals(     QOpenGLShaderProgram* rShaderProgram );
	// Re-generate buffer in case of changes.
	virtual bool changedBoundingBox();

	// Paint
	void vboPaintBoundingBox();
	void vboPaintVertices();
	void vboPaintFaces();
	void vboPaintFacesIndexed( eTexMapType rRenderColor );
	void vboPaintWireframe();
	void vboPaintPolylines();
	void vboPaintNPR();
	void vboPaintTextured();
	void prepareFrambuffersNPR();
	void releaseFramebuffersNPR();
	void vboPaintLightingOverlay();

	void vboPaintPins(std::vector<PinRenderer::PinVertex> &singlePoints);

	void vboPaintTransparencyABuffer();     //render function for A-Buffer
	void vboPaintTransparencyALBuffer();     //render function for atomic loop

	void vboPaintTransparencyWAVG();
	void transparencyPrepareFBO();
	void transparencyDeleteFBO();

	void vboPaintPointcloud(eTexMapType rRenderColor, bool limitPoints);

	// The following methods are replacment for gluCylinder/Cone/Sphere and some related functions.
	void drawTruncatedCone( Vector3D rAxisTop, Vector3D rAxisBottom, double rUpperRadius, double rLowerRadius );

	//FBOs and Textures for the NPR-sketching
	//QOpenGLFramebufferObject* mFboNPR; //<---- Replace with "real" FBO's with the QOpenGLFunctions_3_2_Core Class: more flexible
	//QOpenGLFramebufferObject* mSobelNPR;

	//NPR Framebuffers:
	//0: Output for geometry pass
	//1: Output for Sobel pass
	//2: Output for Toon pass
	//3: Output for Hatch pass
	GLuint mFbosNPR[4];


	//NPR Textures:
	//0: Geometry Buffer
	//1: Lighting Information
	//2: Raw Color
	//3: Depth Buffer
	//4: Sobel Filter Output
	//5: Toon Filter Output
	//6: Hatch Filter Output
	GLuint mNPRFboTextures[7];
	QOpenGLTexture*           mTextureHatchlinesNPR[4]; //<---Texture 0-1 = Hatch, 2-3 = Dots
	QImage                    mImageHatchlinesNPR[4];   //<---Texture 0-1 = Hatch, 2-3 = Dots
	bool                      mIsFboInitialized;

	//SSBOs and variables for Transparency rendering
	QOpenGLFunctions_4_3_Core mGL4_3Functions;  //<-------needed for ssbo´s

	std::array<GLuint,3> mSSBOs;

	GLuint mTransFragmentQuery;

	GLuint mTransAviableFragments;
	int mCurrentNumLayers;
	int mTransIsInitialized; //1: kpBuffer is initialized, 2: ABuffer is initialized, 3: AL is initialized

	GLuint mTransFboTextures[2];
	GLuint mTransFbo;

	//FBOs for overlay rendering
	QOpenGLFramebufferObject* mFboOverlay;

	PinRenderer mPinRenderer;
	TexturedMeshRenderer mTexturedMeshRenderer;
};

#endif // MESHGLSHADER_H
