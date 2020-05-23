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

#ifndef MESHWIDGET_H
#define MESHWIDGET_H

//#define GL_GLEXT_PROTOTYPES
//#define DEAD_CORE_PROFILE

// generic C++ includes:
#include <chrono>

// C++ includes:
#include "meshwidget_params.h"

// generic Qt includes:
//.

// Qt includes:
#include "meshQt.h"

#include "QDir"

#include "svg/SvgWriter.h"


// OpenGL stuff
// OpenGL headers should better be included AFTER Qt headers and any header that includes Qt ...
#ifdef MACXPORT
	#include <gl.h>
	#include <OpenGL/glu.h>
#else
	//#include <QtOpenGL>
	//#include <GL/glu.h>
#endif
#include <QtOpenGL/QGLWidget>
#include <QOpenGLShaderProgram>

class QGMMainWindow;
class MeshQt;

//!
//! \brief Qt OpenGL widget for visualization of a 3D-mesh. (Layer 2)
//!
//! Initalizes OpenGL and provides interactive rotation of a mesh; its viewpoint
//! and light sources using Qt.
//!
//! Layer 2
//!
//! Navigation using a two-button mouse with wheel (required!)
//! <ul>
//!   <li>Leftbutton          = rotate up/down and left/right</li>
//!   <li>Rightbutton         = shift up/down and left/right</li>
//!   <li>Rightbutton + Shift = shift forward/backward and left/right</li>
//!   <li>Wheel               = zoom in/out</li>
//! </ul>
//!
//! Selection of a Primitive: only one type (Vertex, Edge or Face) can be selected.

class MeshWidget : public QGLWidget, public MeshWidgetParams, public MeshGLColors {
    Q_OBJECT

public:
	// Constructor and Destructor:
	MeshWidget( const QGLFormat& format, QWidget* parent );
	~MeshWidget() override;

	bool    getViewPortResolution( double& rRealWidth, double& rRealHeight );
	bool    getViewPortPixelWorldSize( double& rPixelWidth, double& rPixelHeight );
	bool    getViewPortDPI(double& rDPI);
	bool    getViewPortDPM(double& rDPM);

public slots: // ... overloaded from MeshWidgetParams:
	bool    setParamFlagMeshWidget(    MeshWidgetParams::eParamFlag rFlagNr,  bool   rState  ) override;
	bool    toggleShowFlag(            MeshWidgetParams::eParamFlag rFlagNr ) override;
	virtual bool    setParamIntegerMeshWidget( MeshWidgetParams::eParamInt  rParam ); // will trigger an enter-text-dialog.
	bool    setParamIntegerMeshWidget( MeshWidgetParams::eParamInt  rParam,   int    rValue  ) override;
	bool    setParamFloatMeshWidget(   MeshWidgetParams::eParamFlt  rParamID, double rValue  ) override;
	virtual bool    setParamFloatMeshWidget(   MeshWidgetParams::eParamFlt  rParamID, double rMinValue, double rMaxValue ); // will trigger a slider-dialog.
	virtual bool    setParamFloatMeshWidget(   MeshWidgetParams::eParamFlt  rParamID ); // will trigger text-enter-dialog.
	bool    setParamStringMeshWidget(const eParamStr rParamID, const std::string& rString ) override;
private slots: // ... to be avoided - for compatibility of old methods only!
	virtual bool    setParamFloatMeshWidgetSlider( int rParamID, double rValue );
	// new method:
	        bool    callFunctionMeshWidget( MeshWidgetParams::eFunctionCall rFunctionID, bool rFlagOptional=false );

private:
			bool    paramFlagChanged(const eParamFlag rFlagNr );

private slots:
    virtual bool    setPlaneHNFByView();

public slots:
	// File menu (and related)
	bool fileOpen( const QString& rFileName );
	bool reloadFile();
	//.
	bool saveStillImagesSettings();
	void saveStillImages360( Vector3D rotCenter, Vector3D rotAxis );
	void saveStillImages360HLR();
	void saveStillImages360VUp();
	void saveStillImages360PrimN();
	void saveStillImages360PlaneN();
	void saveStillImages360Angles( float** stepAngles, int* stepAnglesNr );
	//.
	void sphericalImagesLight();
	void sphericalImagesLight(const QString& rFileName );
	void sphericalImagesLight(const QString& rFileName, const bool rUseTiled );
	void sphericalImages();
	void sphericalImages(const QString& rFileName );
	void sphericalImages(const QString& rFileName, const bool rUseTiled );
	bool sphericalImagesStateNr();
	//.
	void unloadMesh();

private:
	// Screenshot - Single
	bool screenshotSingle();
	bool screenshotSingle( const QString& rFileName );
	bool screenshotSingle( const QString& rFileName, bool rUseTiled, double& rWidthReal, double& rHeigthReal );

	// Screenshot - Wrapping methods
	bool screenshotPDFUser();
	bool screenshotPDF( const QString& rFileName, const bool rUseTiled );

private:
	// Screenshot - Views - Wrapping methods
	bool screenshotViewsDirectory( QString& rPathChoosen, QStringList& rCurrFiles ); // Internal use only
public:
	bool screenshotViewsPDFDirectory();
	bool screenshotViewsPNGDirectory();
	bool screenshotViewsPDFUser();
	bool screenshotViewsPDF( const QString& rFileName );
private:
	bool screenshotPDFMake( const QString& rPrefixPath, const QString& rFilePrefixTex );

public slots:
	// Screenshot - Views - Rendering
	void screenshotViews();
	void screenshotViews( const QString& rFileName );
	void screenshotViews( const QString&          rFilePattern,
	                      const QString&          rFilePrefix,
	                      const bool              rUseTiled,
	                      std::vector<QString>&   rImageFiles,
	                      std::vector<double>&    rImageSizes );

	// === LEGACY to be removed! ===========================================================================================================================
	void generateLatexFile();
	void generateLatexCatalog();
	QStringList generateLatexCatalog(int depth, const QString& rPath, bool rUseTiled,
	                                 const QStringList& rFilters, const QList<float>& paperProperties,
	                                 const QList<QStringList>& pageCombinations, const QString& suffix,
	                                 float dpiFactorf, const QString& mainPath );
	QStringList generateLatexCatalogPage(const QString& rFilePath, bool rUseTiled, const QList<float>& paperPropertiesf,
	                                     const QList<QStringList>& pageCombinations, const QString& suffix,
	                                     float dpiFactorf, float rDPIf, const QString& mainPath );
	// =====================================================================================================================================================

private:

	// Helper class to render Screenshots into an OffscreenBuffer => only have one of the class initialized at a time
	class OffscreenBuffer
	{
	public:
		OffscreenBuffer(QGLContext *context);
		~OffscreenBuffer();

		OffscreenBuffer(const OffscreenBuffer& other) = delete;
		OffscreenBuffer(const OffscreenBuffer&& other) = delete;

		OffscreenBuffer& operator=(const OffscreenBuffer& other) = delete;
		OffscreenBuffer& operator=(const OffscreenBuffer&& other) = delete;

		GLuint getFboID();
		unsigned char* getColorTexture(int &width, int &height);
		void getColorTextureRegion(unsigned char* &data, int width, int height, int xOffset, int yOffset);
		float* getDepthTexture(int &width, int &height);
		void getDepthTextureRegion(float* &data, int width, int height, int xOffset, int yOffset);

	private:
		GLuint mColorTextureID;
		GLuint mDepthTextureID;
		GLuint mFboID;
		QGLContext* mContext;
		int mTexWidth;
		int mTexHeight;
		unsigned char* mColorTextureBuffer;
		float* mDepthTextureBuffer;
	};

	void bindFramebuffer(int framebufferID);

	// Fetch screenshots:
	bool prepareTile(uint64_t rTilesX, uint64_t rTilesY, unsigned char** rImRGBA, uint64_t* rImWidth, uint64_t* rImHeight, uint64_t rBorderSize = 0 );
	bool fetchFrameAndZBufferTile(unsigned int rTilesX, unsigned int rTilesY, unsigned int rTX, unsigned int rTY, unsigned char* rImRGBA, uint64_t rImWidth, uint64_t rImHeight, OffscreenBuffer* offscreenBuffer, long rBorderSize = 0 );
	bool fetchFrameAndZBuffer( unsigned char*& rImRGBA, uint64_t& rImWidth, uint64_t& rImHeight, bool rCropUsingZBuffer, OffscreenBuffer* offscreenBuffer );
	bool fetchFrameBuffer( unsigned char** rImArray, int* rImWidth, int* rImHeight, bool rCropUsingZBuffer, OffscreenBuffer* offscreenBuffer );
	// Write screenshots:
	bool screenshotTIFF(const QString& rFileName , OffscreenBuffer *offscreenBuffer);
	bool screenshotPNG(const QString& rFileName, double& rWidthReal, double& rHeigthReal , OffscreenBuffer *offscreenBuffer);


public slots:
	void selectColorBackground();

	// View menu
	bool showViewMatrix();
	bool setViewMatrix();
	bool setViewMatrix( std::vector<double> rMatrix );
	bool setViewAxisUp();
	//.
	bool orthoSetDPI();
	bool orthoSetDPI( double rSetTo );
	//.
	bool screenshotSVG();
	bool screenshotSVG(const QString& rFileName, const QString& rFileNamePNG );

	bool exportPlaneIntersectPolyLinesSVG();
	bool screenshotSVGexportPlaneIntersections( double rOffsetX, double rOffsetY, double rPolyLineWidth, double axisOffset, SvgWriter& svgWriter, const std::set<unsigned int>& polylineIDs );

	bool screenshotSVGexportPolyLines( Vector3D& cameraViewDir, Matrix4D& matView, double polyScaleWdith, double polyScaleHeight, double polyLineWidth, SvgWriter& svgWriter );

	void screenshotRuler();
	bool screenshotRuler( const QString& rFileName );
	bool screenshotTiledPNG(QString rFileName, double& rWidthReal, double& rHeigthReal, OffscreenBuffer *offscreenBuffer, int rBorderSize=0);
	bool fetchRuler( unsigned char** rImArray, int* rImWidth, int* rImHeight, double& rPixelWidth, double& rPixelHeight );
	bool writePNG( const QString& rFileName, uint64_t rImWidth, uint64_t rImHeight, unsigned char* rImRGBA, int rDotsPerMeterWidth, int rDotsPerMeterHeight );

	bool cropRGBAbyAlpha( uint64_t* rImWidth, uint64_t* rImHeight, unsigned char** rImRGBA, double& rRealWidth, double& rRealHeight );
	//.
	void defaultViewLight();
	void defaultViewLightZoom();
	//.
	bool rotYaw();
	bool rotPitch();
	bool rotRoll();
	//..
	bool rotYaw( double rAngle );
	bool rotPitch( double rAngle );
	bool rotRoll( double rAngle );
	bool rotRollPitchYaw(double rAngle, double pAngle, double yAngle);
	void rotOrthoPlane();
	void rotArbitAxis( Vector3D rCenter, Vector3D rAxis, double rAngle );
	//.
    bool rotPlaneYaw( double rAngle );
    bool rotPlanePitch( double rAngle );
    bool rotPlaneRoll( double rAngle );
	//.
	void selPrimViewReference();
	//.
	bool currentViewToDefault();

	void openNormalSphereSelectionDialog(bool faces);
	void setCameraRotation(QQuaternion rotationQuat);

signals:
	void sStatusMessage(QString);                                   //!< Notify (the MainWindow) about status changes.
	void sViewPortInfo(MeshWidgetParams::eViewPortInfo,QString);    //!< Infos emitted by the viewport e.g. for display purposes.
	// signals to the select menu:
	void sParamFlagMesh(MeshGLParams::eParamFlag,bool);             //!< Sets a specific display flag see MeshGL::setShowFlag.
	void sParamFlagMeshWidget(MeshWidgetParams::eParamFlag,bool);   //!< Sets a specific display flag see MeshWidget::setShowFlag.
	void sParamIntegerMeshWidget(MeshWidgetParams::eParamInt,int);  //!< Sets a specific display integer see MeshWidget::
	void sSelectPoly(std::vector<QPoint>&);                              //!< Pixel coordinates for polygonal/prism selection.
	// changes to plane position
	void sApplyTransfromToPlane(Matrix4D);                          //!< Emitted when the planes postion was (interactivly) changed.
	// guide for docked window
	void sGuideIDCommon(MeshWidgetParams::eGuideIDCommon);          //!< guide ID for sidebar
	void sGuideIDSelection(MeshWidgetParams::eGuideIDSelection);    //!< guide ID for sidebar

	void camRotationChanged(Vector3D, Vector3D);                            //!< signal emitted when camera is rotated by mouse

	void loadedMeshIsTextured(bool);
private:
	void initializeGL() override;
	void initializeVAO();
	void initializeShaders();
	void resizeGL( int width, int height ) override;
	void paintEvent( QPaintEvent *rEvent ) override;
	void paintSelection();
	bool paintHistogram();
	bool paintHistogramScence();
	void resizeEvent( QResizeEvent * event ) override;

	// Keyboard and Mouse interaction:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	void mousePressEvent( QMouseEvent *rEvent ) override;
	void mouseDoubleClickEvent( QMouseEvent* rEvent) override;
	void mouseReleaseEvent( QMouseEvent *rEvent) override;
	void mouseMoveEvent( QMouseEvent *rEvent ) override;
	void wheelEvent( QWheelEvent * rEvent ) override;
	void wheelEventZoom( double rWheelDelta ); // Helper function
	void keyPressEvent( QKeyEvent *rEvent ) override;

	// User Interaction:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	bool userSetConeAxisCentralPixel();
	bool userSelectByMouseClick( QPoint rPoint, QFlags<Qt::MouseButton> rMouseButton );
	bool userSelectAtMouseLeft( const QPoint& rPoint );
	bool userSelectAtMouseRight( const QPoint& rPoint );

	// Main members
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QGMMainWindow*      mMainWindow;     //!< reference to our main/top-level window (AKA parent)
	MeshQt*             mMeshVisual;     //!< Pointer to the Mesh (overloaded with functions for OpenGL and Qt) shown.

	// drawing stuff and setting the camera:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	void setViewInitial();
	void setViewInitialZoom();
	void setView( GLdouble* rOrthoViewPort=nullptr );
	void setViewModelMat();

	// Camera parameters (OpenGL)
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	Vector3D   mCenterView;            //!< the central point we(=camera/eye) are looking at.
	Vector3D   mCameraCenter;          //!< center of the camera(=eye) (X-coordinate).
	Vector3D   mCameraUp;              //!< camera orientation.
	// Matrices for OpenGL:
	QMatrix4x4 mMatProjection;      //!< OpenGL projection matrix.
	QMatrix4x4 mMatModelView;       //!< OpenGL modelview matrix.

	// Mouse and keyboard interaction
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QPoint  mLastPos;                 //!< Stores the last cursor position. Used to determine movement of the mouse for interaction.
	QPoint  mLastPosRelease;          //!< Stores the last cursor position when mouse is clicked, for selection if we know position where mouse is released
	std::vector<QPoint> mSelectionPoly;    //!< Screen coordinates of the selection poylgon.

	// performance evaluation (frames per second):
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QElapsedTimer mFrameTime;  //!< Timer to estimate the frames per second (OpenGL).
	int   mFrameCount; //!< Counter to estimate the frames per second (OpenGL).

	// Vertex Buffer Objects and Shaders for the widget's background
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	GLuint mVAO; //!< Vertex Array Object - has to be created!
	enum eVBOs {
		VBO_BACKGROUND_VERTICES, //!< VBO: with four vertices for the background canvas.
		VBO_ARRAY_COUNT          //!< Number of VBOs.
	};
	QOpenGLBuffer mVertBufObjs[VBO_ARRAY_COUNT]; //!< Array of Vertex Buffer Objects (within the VAO).

	// One shader per background type and one shader for images:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QOpenGLShaderProgram* mShaderGridOrtho;        //!< Shader program for rectangular grid.
	QOpenGLShaderProgram* mShaderGridPolarLines;   //!< Shader program for polar grid -- lines.
	QOpenGLShaderProgram* mShaderGridPolarCircles; //!< Shader program for polar grid -- concentric circles.
	QOpenGLShaderProgram* mShaderGridHighLightCenter;  //!< Shader program highlighting the central pixel.
	QOpenGLShaderProgram* mShaderImage;            //!< Shader program for display of images e.g. logos, because QPainter is not compliant with OpenGL CoreProfile.

	// Texture maps used by the shader programs
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	enum eTextureMaps {
		TEXMAP_GIGAMESH_LOGO,              //!< Image showing the GigaMesh logo.
		TEXMAP_KEYBOARD_LAYOUT,            //!< Image showing the keyboard layout.
		TEXMAP_HISTOGRAM_SCENE,            //!< Computed image showing the histogram of the scence.
		TEXMAP_HISTOGRAM_MESH_FUNCVAL,     //!< Computed using the function values of the mesh.
		TEXMAP_SELECTION_POLYGON_OVERLAY,  //!< Polygon related to the selection.
		TEXMAP_COUNT                       //!< Number of texture maps.
	};
	QOpenGLTexture* mTextureMaps[TEXMAP_COUNT];  //!< Texturemap IDs for the shader. Currently there is just one holding the color ramps.

	// Methods for initalization and painting:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	bool initializeTextureMap( eTextureMaps rTextureMap, const QString& rFileName );
	bool initializeTextureMap( eTextureMaps rTextureMap, QImage* rImage );
	void initializeShader(const QString& rFileName, QOpenGLShaderProgram** rShaderProgram );
	bool paintBackgroundShader( QOpenGLShaderProgram** rShaderProgram );
	bool paintRasterImage( eTextureMaps rTexMap, int rPixelX, int rPixelY, int rPixelWidth, int rPixelHeight );
    //------------------------------------------------------------------------------------------------------------------------------------------------------

	//! Checkes if mesh might cause problems. E.g. too small or georeferenced (far away from origin)
	void checkMeshSanity();
	void checkMissingTextures(ModelMetaData& metadata);
};

#endif // MESHWIDGET_H
