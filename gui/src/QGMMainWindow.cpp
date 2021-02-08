//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

#include "QGMMainWindow.h"

// Qt includes
#include <QFileDialog>
// Qt Network
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "QGMMacros.h"

#include "meshwidget.h"
#include "qgmdocksidebar.h"
#include "qgmdockinfo.h"
#include "qgmdockview.h"
#include "ExternalProgramsDialog.h"
#include "dialogGridCenterSelect.h"
#include "oauth/githubmodel.h"
#include "QGMDialogAuthorize.h"

using namespace std;

// Sets default values - to be used by all contructors!
// ----------------------------------------------------
#define QGMMAINWINDOWINITDEFAULTS  \
	mMeshWidget( nullptr ),           \
	mDockSurface( nullptr ),          \
	mDockInfo( nullptr ),             \
	mMeshWidgetFlag( nullptr ),       \
	mRecentFiles( nullptr ),          \
	mNetworkManager( nullptr )

//! Constructor
QGMMainWindow::QGMMainWindow( QWidget *parent, Qt::WindowFlags flags )
    : QMainWindow( parent, flags ), QGMMAINWINDOWINITDEFAULTS {
	setupUi( this );

	createLanguageMenu();
	//uiMainToolBar.show();

	//adding all menu actions to main window, so that they are not deaktivated in fullscreen-mode
        addActions(this->menubar->actions());

	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	setWindowTitle( QString( "GigaMesh" ) );

	// +++ Mesh/MeshGL/MeshQT init -> see private method +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	initMeshSignals(); // THIS is the one and only legit place to call this method!!!

	// +++ MeshWidget init -> see private method +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	initMeshWidgetSignals(); // THIS is the one and only legit place to call this method!!!

	// Group all menu entries, which relate to ONLY a valid selection
	menuContextToSelection = new QActionGroup( this );
	menuContextToSelection->addAction( actionSaveStillImages360PrimN );
	menuContextToSelection->addAction( actionFeatDistSelEuc );
	menuContextToSelection->addAction( actionFeatDistSelMan );
	menuContextToSelection->addAction( actionFeatCorrelationSelectedVert );
	menuContextToSelection->addAction( actionFeatAutoSelectedVertCorr );

	// === RECENT FILES ====================================================================================================================================

	updateRecentFileMenu();

	// === SIGNALS & SLOTS =================================================================================================================================
	QObject::connect( actionImportFunctionValues,     &QAction::triggered, this,   &QGMMainWindow::menuImportFunctionValues        );
	// connect the main windows menu entries with slots:
	// --- File ---
	QObject::connect( actionFileOpen,                 SIGNAL(triggered()), this,       SLOT(load())                      );
	QObject::connect( actionFileReload,               SIGNAL(triggered()), this,       SIGNAL(sFileReload())             );
	//.
	QObject::connect( actionSaveFlagBinary,           SIGNAL(toggled(bool)), this,     SIGNAL(sFileSaveFlagBinary(bool))   );
	QObject::connect( actionSaveFlagGMExtras,         SIGNAL(toggled(bool)), this,     SIGNAL(sFileSaveFlagGMExtras(bool)) );
	QObject::connect( actionSaveFlagTextureExport,    SIGNAL(toggled(bool)), this,     SIGNAL(sFileSaveFlagExportTexture(bool)) );
	//.
	QObject::connect( actionImportTexMap,             SIGNAL(triggered()), this,       SLOT(menuImportTexMap())          );
	QObject::connect( actionImportFeatureVectors,     SIGNAL(triggered()), this,       SLOT(menuImportFeatureVectors())  );
	QObject::connect( actionExportFeatureVectors,     SIGNAL(triggered()), this,       SIGNAL(sExportFeatureVectors())  );
	QObject::connect( actionImportNormals,            SIGNAL(triggered()), this,       SLOT(menuImportNormalVectors())   );
	//.
	QObject::connect( actionExportPolylines,          SIGNAL(triggered()), this,       SIGNAL(exportPolyLinesCoords())          );
	QObject::connect( actionExportPolylinesProjected, SIGNAL(triggered()), this,       SIGNAL(exportPolyLinesCoordsProjected()) );
	QObject::connect( actionExportPolylinesFuncVals,  SIGNAL(triggered()), this,       SIGNAL(exportPolyLinesFuncVals())        );
	//.
	QObject::connect( actionExportFuncVals,           SIGNAL(triggered()), this,       SIGNAL(exportFuncVals())                 );
	QObject::connect( actionExportFaceNormalAngles,   SIGNAL(triggered()), this,       SIGNAL(exportFaceNormalAngles())         );

	QObject::connect( actionExport_Normal_Sphere_Data, &QAction::triggered, this,       &QGMMainWindow::exportNormalSphereData);
	//.
	QObject::connect( actionSaveStillImages360HLR,    SIGNAL(triggered()), this,       SIGNAL(saveStillImages360HLR())    );
	QObject::connect( actionSaveStillImages360VUp,    SIGNAL(triggered()), this,       SIGNAL(saveStillImages360VUp())    );
	QObject::connect( actionSaveStillImages360PrimN,  SIGNAL(triggered()), this,       SIGNAL(saveStillImages360PrimN())  );
	QObject::connect( actionSaveStillImages360PlaneN, SIGNAL(triggered()), this,       SIGNAL(saveStillImages360PlaneN()) );
	//.
	QObject::connect( actionSphericalImagesLight,     SIGNAL(triggered()),   this,     SIGNAL(sphericalImagesLight())          );
	QObject::connect( actionSphericalImages,          SIGNAL(triggered()),   this,     SIGNAL(sphericalImages())               );
	QObject::connect( actionSphericalImagesStateNr,   SIGNAL(triggered()),   this,     SIGNAL(sphericalImagesStateNr())        );
	//.
	QObject::connect( actionUnload3D,                 SIGNAL(triggered()),   this,     SIGNAL(unloadMesh())                    );
	//.
	QObject::connect( actionQuit,                     SIGNAL(triggered()),   this,     SLOT(close())                           );

	// --- Edit --------------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionRemoveVerticesSelected, SIGNAL(triggered()), this,       SIGNAL(removeVerticesSelected())  );
	QObject::connect( actionRemoveUncleanSmall,     SIGNAL(triggered()), this,       SIGNAL(removeUncleanSmall())      );
	//.
	QObject::connect( actionCutOffFeatureVertex,    SIGNAL(triggered()), this,       SIGNAL(cutOffFeatureVertex())     );
	QObject::connect( actionCutOffFeatureFace,      SIGNAL(triggered()), this,       SIGNAL(cutOffFeatureFace())       );
	//. 
	QObject::connect( actionFuncValSet,             SIGNAL(triggered()), this,       SIGNAL(funcValSet())              );
	QObject::connect( actionFuncValueCutOff,        SIGNAL(triggered()), this,       SIGNAL(funcValueCutOff())         );
	QObject::connect( actionFuncValsNormalize,      SIGNAL(triggered()), this,       SIGNAL(funcValsNormalize())       );
	QObject::connect( actionFuncValsAbs,            SIGNAL(triggered()), this,       SIGNAL(funcValsAbs())             );
	QObject::connect( actionFuncValsAdd,            SIGNAL(triggered()), this,       SIGNAL(funcValsAdd())             );
	//.
	QObject::connect( actionSetConeData,            SIGNAL(triggered()), this,       SIGNAL(setConeData()));
	QObject::connect( actionCenterAroundCone,       SIGNAL(triggered()), this,       SIGNAL(centerAroundCone()));
	//.
	QObject::connect( actionSplitByPlaneAdvanced,   SIGNAL(triggered()), this,       SIGNAL(sSplitByPlane())           );
	QObject::connect( actionSplitByIsoValue,        SIGNAL(triggered()), this,       SIGNAL(sSplitByIsoValue()));
	//.
	QObject::connect( actionCenterAroundSphere,     SIGNAL(triggered()), this,       SIGNAL(centerAroundSphere()));
	QObject::connect( actionUnrollAroundSphere,     SIGNAL(triggered()), this,       SIGNAL(unrollAroundSphere()));
	//.
	QObject::connect( actionApplyMeltingSphere,     SIGNAL(triggered()), this,       SIGNAL(sApplyMeltingSphere())     );
    //.
    QObject::connect( actionVertApplyNormalShift,   SIGNAL(triggered()), this,       SIGNAL(sApplyNormalShift())       );

	// --- De-Selection ------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionDeSelVertsAll,          SIGNAL(triggered()),         this, SIGNAL(sDeSelVertsAll())         );
	QObject::connect( actionDeSelVertsNoLabel,      SIGNAL(triggered()),         this, SIGNAL(sDeSelVertsNoLabel())     );
	// --- Selection ---------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionPlaneGetVPos,           SIGNAL(triggered()),         this, SIGNAL(getPlaneVPos())           );
	QObject::connect( actionPlaneGetHNF,            SIGNAL(triggered()),         this, SIGNAL(getPlaneHNF())            );
	QObject::connect( actionPlaneSetVPos,           SIGNAL(triggered()),         this, SIGNAL(setPlaneVPos())           );
	QObject::connect( actionPlaneSetHNF,            SIGNAL(triggered()),         this, SIGNAL(setPlaneHNF())            );
    QObject::connect( actionPlaneSetHNFByView,      SIGNAL(triggered()),         this, SIGNAL(setPlaneHNFByView())      );
	//.
	QObject::connect( actionSelVertFuncLT,          SIGNAL(triggered()),         this, SIGNAL(selectVertFuncLT())      );
	QObject::connect( actionSelVertFuncGT,          SIGNAL(triggered()),         this, SIGNAL(selectVertFuncGT())      );
    //.
    QObject::connect( actionSelVertNonMax,          SIGNAL(triggered()),         this, SIGNAL(selectVertNonMax())      );
	//. 
	QObject::connect( actionSelVertLocalMin,        SIGNAL(triggered()),         this, SIGNAL(selectVertLocalMin())      );
	QObject::connect( actionSelVertLocalMax,        SIGNAL(triggered()),         this, SIGNAL(selectVertLocalMax())      );
	//.
	QObject::connect( actionSelVertSolo,            SIGNAL(triggered()),         this, SIGNAL(selectVertSolo())                );
	QObject::connect( actionSelVertNonManifoldFace, SIGNAL(triggered()),         this, SIGNAL(selectVertNonManifoldFaces())    );
	QObject::connect( actionSelVertDoubleCone,      SIGNAL(triggered()),         this, SIGNAL(selectVertDoubleCone())          );
	QObject::connect( actionSelVertLabelAreaLT,     SIGNAL(triggered()),         this, SIGNAL(selectVertLabelAreaLT())         );
	QObject::connect( actionSelVertLabelAreaRelLT,  SIGNAL(triggered()),         this, SIGNAL(selectVertLabelAreaRelativeLT()) );
	QObject::connect( actionSelVertBorder,          SIGNAL(triggered()),         this, SIGNAL(selectVertBorder())              );
	QObject::connect( actionSelVertFaceMinAngleLT,  SIGNAL(triggered()),         this, SIGNAL(selectVertFaceMinAngleLT())      );
	QObject::connect( actionSelVertFaceMaxAngleGT,  SIGNAL(triggered()),         this, SIGNAL(selectVertFaceMaxAngleGT())      );
	QObject::connect( actionSelVertLabeledNot,      SIGNAL(triggered()),         this, SIGNAL(sSelVertLabeledNot())            );
	//.
	QObject::connect( actionSelFaceNone,            SIGNAL(triggered()),         this, SIGNAL(selectFaceNone())                );
	QObject::connect( actionSelFaceSticky,          SIGNAL(triggered()),         this, SIGNAL(selectFaceSticky())              );
	QObject::connect( actionSelFaceNonManifold,     SIGNAL(triggered()),         this, SIGNAL(selectFaceNonManifold())         );
	QObject::connect( actionSelFaceZeroArea,        SIGNAL(triggered()),         this, SIGNAL(selectFaceZeroArea())            );
	QObject::connect( actionSelFaceInSphere,        SIGNAL(triggered()),         this, SIGNAL(selectFaceInSphere())            );
	QObject::connect( actionSelFaceRandom,          SIGNAL(triggered()),         this, SIGNAL(selectFaceRandom())              );
	//.
	QObject::connect( actionSelPolyNoLabel,         SIGNAL(triggered()),         this, SIGNAL(selectPolyNoLabel())             );
	QObject::connect( actionSelPolyNotLabeled,      SIGNAL(triggered()),         this, SIGNAL(selectPolyNotLabeled())          );
	QObject::connect( actionSelPolyRunLenGT,        SIGNAL(triggered()),         this, SIGNAL(selectPolyRunLenGT())            );
	QObject::connect( actionSelPolyRunLenLT,        SIGNAL(triggered()),         this, SIGNAL(selectPolyRunLenLT())            );
	QObject::connect( actionSelPolyLongest,         SIGNAL(triggered()),         this, SIGNAL(selectPolyLongest())             );
	QObject::connect( actionSelPolyShortest,        SIGNAL(triggered()),         this, SIGNAL(selectPolyShortest())            );
	QObject::connect( actionSelPolyLabelNo,         SIGNAL(triggered()),         this, SIGNAL(selectPolyLabelNo())             );

	QObject::connect( actionVertices_Normal_Sphere, &QAction::triggered,         this, &QGMMainWindow::sOpenNormalSphereSelectionDialogVertices );
	QObject::connect( actionFaces_Normal_Sphere,    &QAction::triggered,         this, &QGMMainWindow::sOpenNormalSphereSelectionDialogFaces    );

	// --- View --------------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionFullscreen,    &QAction::triggered,   this,          &QGMMainWindow::toggleFullscreen   );
	QObject::connect( actionMenu,          &QAction::toggled,     menubar,       &QMenuBar::setVisible              );
	QObject::connect( actionToolbar,       &QAction::toggled,     uiMainToolBar, &QToolBar::setVisible              );
	QObject::connect( actionStatusbar,     &QAction::toggled,     statusbar,     &QStatusBar::setVisible            );

	QObject::connect( actionViewMatrix,                  SIGNAL(triggered()),   this, SIGNAL(showViewMatrix())                   );
	QObject::connect( actionViewMatrixSet,               SIGNAL(triggered()),   this, SIGNAL(setViewMatrix())                    );
	QObject::connect( actionViewAxisUp,    &QAction::triggered,   this,          &QGMMainWindow::sSetViewAxisUp     );

	QObject::connect( actionViewActivateInspectionOptions,      SIGNAL(triggered()), this, SLOT(activateInspectionOptions())     );

	// ... Vertices 
	QObject::connect( actionViewPolylinesCurvScale,      SIGNAL(triggered()),   this, SIGNAL(polylinesCurvScale())               );
	//.
	QObject::connect( actionScreenshotsCrop,         SIGNAL(toggled(bool)), this,       SIGNAL(screenshotsCrop(bool))  );
	QObject::connect( actionScreenshotSVG,           SIGNAL(triggered()),   this,       SIGNAL(screenshotSVG())        );
	QObject::connect( actionScreenshotRuler,         SIGNAL(triggered()),   this,       SIGNAL(screenshotRuler())      );

    QObject::connect( actionScreenshotDirectory,     SIGNAL(triggered()),   this,       SIGNAL(screenshotDirectory()) );

    QObject::connect( actionGenerateLatexFile,       SIGNAL(triggered()),   this,       SIGNAL(generateLatexFile()) );
    QObject::connect( actionGenerateLatexCatalog,    SIGNAL(triggered()),   this,       SIGNAL(generateLatexCatalog()) );

	//.
	QObject::connect( actionViewDefaultViewLight,     SIGNAL(triggered()),  this,       SIGNAL(sDefaultViewLight())     );
	QObject::connect( actionViewDefaultViewLightZoom, SIGNAL(triggered()),  this,       SIGNAL(sDefaultViewLightZoom()) );
	//.
	QObject::connect( actionRotYaw,                  SIGNAL(triggered()),   this,       SIGNAL(rotYaw())                );
	QObject::connect( actionRotPitch,                SIGNAL(triggered()),   this,       SIGNAL(rotPitch())              );
	QObject::connect( actionRotRoll,                 SIGNAL(triggered()),   this,       SIGNAL(rotRoll())               );
	QObject::connect( actionRotOrthoPlane,           SIGNAL(triggered()),   this,       SIGNAL(rotOrthoPlane())         );
	//.
	QObject::connect( actionSelPrimViewReference,     SIGNAL(triggered()),  this,       SIGNAL(sSelPrimViewReference()) );

	// --- Analyze------------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionLabelFaces,                    SIGNAL(triggered()), this, SIGNAL(labelFaces())            );
	QObject::connect( actionLabelSelectionToSeeds,         SIGNAL(triggered()), this, SIGNAL(labelSelectionToSeeds()) );
	QObject::connect( actionLabelVertEqualFV,              SIGNAL(triggered()), this, SIGNAL(labelVerticesEqualFV())  );
	QObject::connect( actionLabelSelMVertsBackground,      SIGNAL(triggered()), this, SIGNAL(sLabelSelMVertsToBack()) );
	//.
	QObject::connect( actionSelectionToPolyline,           SIGNAL(triggered()), this, SIGNAL(convertSelectedVerticesToPolyline()) );
	QObject::connect( actionMeshBordersToPolylines,        SIGNAL(triggered()), this, SIGNAL(convertBordersToPolylines())         );
	QObject::connect( actionLabelbordersToPolylines,       SIGNAL(triggered()), this, SIGNAL(convertLabelBordersToPolylines())    );
    QObject::connect( actionCreate_SkeletonLine,           SIGNAL(triggered()), this, SIGNAL(createSkeletonLine())                );
    QObject::connect( actionAdvancePolyThres,              SIGNAL(triggered()), this, SIGNAL(advancePolyThres())                  );
	QObject::connect( actionPolylinesCompIntInvRunLen,     SIGNAL(triggered()), this, SIGNAL(sPolylinesCompIntInvRunLen())        );
	QObject::connect( actionPolylinesCompIntInvAngle,      SIGNAL(triggered()), this, SIGNAL(sPolylinesCompIntInvAngle())         );
	QObject::connect( actionPolylineExtrema,               SIGNAL(triggered()), this, SIGNAL(sPolylinesCompCurv())                );
	QObject::connect( actionSetLengthSmooth,               SIGNAL(triggered()), this, SIGNAL(setLengthSmooth())                   );
	QObject::connect( actionPolylinesCopyNormalToVertices, SIGNAL(triggered()), this, SIGNAL(sPolylinesCopyNormalToVertices())    );

	//.
    QObject::connect( actionCheckNonMaxCorr,         SIGNAL(triggered()), this, SIGNAL(nonMaxCorrCheck())                   );
    //.
	QObject::connect( actionLabelFuncValMinima,      SIGNAL(triggered()), this, SIGNAL(sLabelFindFuncValMinima())           );
	QObject::connect( actionLabelFuncValMaxima,      SIGNAL(triggered()), this, SIGNAL(sLabelFindFuncValMaxima())           );
	//.
	QObject::connect( actionEstimateMSIIFeature,     SIGNAL(triggered()), this, SIGNAL(estimateMSIIFeat())                  );
	//.
	QObject::connect( actionGeodPatchVertSel,        SIGNAL(triggered()), this, SIGNAL(sGeodPatchVertSel())                 );
	QObject::connect( actionGeodPatchVertSelOrdered, SIGNAL(triggered()), this, SIGNAL(sGeodPatchVertSelOrder())            );
	//.
	QObject::connect( actionSphereIntersect,         SIGNAL(triggered()), this, SIGNAL(intersectSphere())                   );
	//.
	QObject::connect( actionFillHoles,               SIGNAL(triggered()), this, SIGNAL(fillHoles())      );
	//.
	QObject::connect( actionEstimateVolume,          SIGNAL(triggered()), this, SIGNAL(estimateVolume())   );
	QObject::connect( actionComputeVolumePlane,      SIGNAL(triggered()), this, SIGNAL(compVolumePlane())  );
	QObject::connect( actionPolylinesLength,         SIGNAL(triggered()), this, SIGNAL(sPolylinesLength()) );
	//.
	QObject::connect( actionHueToFuncVal,            SIGNAL(triggered()), this, SIGNAL(hueToFuncVal())   );
	//.
	QObject::connect( actionDatumAddSphere,          SIGNAL(triggered()), this, SIGNAL(sDatumAddSphere())   );

        // --- Octree related ----------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionGenerateOctree,          SIGNAL(triggered()), this, SIGNAL(generateOctree())            );
	QObject::connect( actionRemove_Drawing_of_Octree,SIGNAL(triggered()), this, SIGNAL(removeOctreedraw())            );
	QObject::connect( actionDraw_Octree,             SIGNAL(triggered()), this, SIGNAL(drawOctree())            );
	QObject::connect( actionDelete_Octree,           SIGNAL(triggered()), this, SIGNAL(deleteOctree())            );


	// #####################################################################################################################################################
	// # FUNCTION VALUE
	// #####################################################################################################################################################
	// # Feature Vector related
	QObject::connect( actionFeatLengthEuc,                  SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatLengthEuc())               ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatLengthMan,                  SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatLengthMan())               ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatBVFunc,                     SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatBVFunc())                  ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatTVSeqn,                     SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatTVSeqn())                  ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatDistSelEuc,                 SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatDistSelVertEuc())          ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatDistSelEucNorm,             SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatDistSelVertEucNorm())      ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatDistSelMan,                 SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatDistSelVertMan())          ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatDistSelCosSim,              SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatDistSelVertCosSim())       ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatDistSelTanimoto,            SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatDistSelVertTanimoto())     ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatCorrelationSelectedVert,    SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatCorrSelVert())             ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatAutoCorrelationVert,        SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatAutoCorrVert())            ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFeatAutoSelectedVertCorr,       SIGNAL(triggered()),   this,       SIGNAL(sFuncVertFeatAutoCorrSelVert())         ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFuncValToFeatureVector,         SIGNAL(triggered()),   this,       SIGNAL(sFuncValToFeatureVector())              );
	//! \todo Rename regarding new menu structure.
	// # Distance to plane, line, selected primitive and cone
	QObject::connect( actionDistanceToPlane,                SIGNAL(triggered()),   this,       SIGNAL(visualizeDistanceToPlane())             ); // <- OLD
	QObject::connect( actionDistanceToCone,                 SIGNAL(triggered()),   this,       SIGNAL(visualizeDistanceToCone())              ); // <- OLD
	// # Other
	QObject::connect( actionVisVertIndex,                   SIGNAL(triggered()),   this,       SIGNAL(visualizeVertexIndices())               ); // <- OLD
	QObject::connect( actionFuncVert1RingRMin,              SIGNAL(triggered()),   this,       SIGNAL(sFuncVert1RingRMin())                   ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionFuncVert1RingVolInt,            SIGNAL(triggered()),   this,       SIGNAL(sFuncVert1RingVolInt())                 ); // <- NEW naming convention based on new menu structure!
	QObject::connect( actionVisVertOctree,                  SIGNAL(triggered()),   this,       SIGNAL(visualizeVertexOctree())                ); // <- OLD
	QObject::connect( actionVisVertFaceSphereAngleMax,      SIGNAL(triggered()),   this,       SIGNAL(visualizeVertexFaceSphereAngleMax())    ); // <- OLD
	QObject::connect( actionVisVertFaceSphereMeanAngleMax,  SIGNAL(triggered()),   this,       SIGNAL(visualizeVertFaceSphereMeanAngleMax())  ); // <- OLD
	// #####################################################################################################################################################

	// --- Colors ------------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( actionSelectColorBackground,  SIGNAL(triggered()),   this,       SIGNAL(selectColorBackground())  );

	// --- ? -----------------------------------------------------------------------------------------------------------------------------------------------
	// New Qt5 Signal-Slot concept:
        QObject::connect( actionInfoKeyShortcuts,      &QAction::triggered, this, &QGMMainWindow::infoKeyShortcuts      );
	QObject::connect( actionVisitVideoTutorials,   &QAction::triggered, this, &QGMMainWindow::visitVideoTutorials   );
	QObject::connect( actionVisitWebSite,          &QAction::triggered, this, &QGMMainWindow::visitWebSite          );
	QObject::connect( actionAbout,                 &QAction::triggered, this, &QGMMainWindow::aboutBox              );
	// -----------------------------------------------------------------------------------------------------------------------------------------------------

	// --- DOCK Widgets: Surface ---------------------------------------------------------------------------------------------------------------------------
	mDockSurface = new QGMDockSideBar( this );
	addDockWidget( Qt::RightDockWidgetArea, mDockSurface );
	QObject::connect( mDockSurface, SIGNAL(sShowParamIntMeshGL(MeshGLParams::eParamInt,int)), this,         SIGNAL(sShowParamIntMeshGL(MeshGLParams::eParamInt,int)) );
	QObject::connect( this,         SIGNAL(sShowParamIntMeshGL(MeshGLParams::eParamInt,int)), mDockSurface, SLOT(updateMeshParamInt(MeshGLParams::eParamInt,int))    );
	QObject::connect( mDockSurface, SIGNAL(sShowNPRSettings()), this, SIGNAL(sShowNPRSettings()));
	QObject::connect( mDockSurface, SIGNAL(sShowTransparencySettings()), this, SIGNAL(sShowTransparencySettings()));
	// -----------------------------------------------------------------------------------------------------------------------------------------------------

	// --- DOCK Widgets: Info ------------------------------------------------------------------------------------------------------------------------------
	mDockInfo = new QGMDockInfo( this );
	addDockWidget( Qt::RightDockWidgetArea, mDockInfo );
	QObject::connect( mDockInfo, SIGNAL(sShowFlagMeshWidget(MeshWidgetParams::eParamFlag,bool)),    this,      SIGNAL(sShowFlagMeshWidget(MeshWidgetParams::eParamFlag,bool))   );
	QObject::connect( mDockInfo, SIGNAL(sShowParamIntMeshWidget(MeshWidgetParams::eParamInt,int)),  this,      SIGNAL(sShowParamIntMeshWidget(MeshWidgetParams::eParamInt,int)) );
	QObject::connect( mDockInfo, SIGNAL(sReloadFromFile()),                                         this,      SIGNAL(sFileReload())                                            );
	QObject::connect( this,      SIGNAL(sSelectMouseModeDefault()),                                 mDockInfo, SLOT(selectMouseModeDefault())                                   );
	QObject::connect( this,      SIGNAL(sSelectMouseModeExtra(bool,MeshWidgetParams::eMouseModes)), mDockInfo, SLOT(selectMouseModeExtra(bool,MeshWidgetParams::eMouseModes))   );
	// New Qt5 Signal-Slot concept:
	QObject::connect( this,      &QGMMainWindow::sGuideIDCommon,                                    mDockInfo, &QGMDockInfo::setGuideIDCommon                                   );
	QObject::connect( this,      &QGMMainWindow::sGuideIDSelection,                                 mDockInfo, &QGMDockInfo::setGuideIDSelection                                );
	QObject::connect( this,      &QGMMainWindow::sShowProgressStart,                                mDockInfo, &QGMDockInfo::showProgressStart                                  );
	QObject::connect( this,      &QGMMainWindow::sShowProgress,                                     mDockInfo, &QGMDockInfo::showProgress                                       );
	QObject::connect( this,      &QGMMainWindow::sShowProgressStop,                                 mDockInfo, &QGMDockInfo::showProgressStop                                   );
        QObject::connect( this,      &QGMMainWindow::sViewUserInfo,                                     mDockInfo, &QGMDockInfo::viewUserInfo                                       );
        // -----------------------------------------------------------------------------------------------------------------------------------------------------

	// --- DOCK Widgets: Viewport --------------------------------------------------------------------------------------------------------------------------
	mDockView = new QGMDockView( this );
	addDockWidget( Qt::LeftDockWidgetArea, mDockView );
	QObject::connect( this, SIGNAL(sViewPortInfo(MeshWidgetParams::eViewPortInfo,QString)), mDockView, SLOT(viewPortInfo(MeshWidgetParams::eViewPortInfo,QString)) );
	QObject::connect( this, SIGNAL(sInfoMesh(MeshGLParams::eInfoMesh,QString)),             mDockView, SLOT(infoMesh(MeshGLParams::eInfoMesh,QString))             );
        // -----------------------------------------------------------------------------------------------------------------------------------------------------

	QObject::connect( actionExternal_Programs, SIGNAL(triggered()), this, SLOT(openExternalProgramsDialog()));
	QObject::connect( actionGridCenter, SIGNAL(triggered()), this, SLOT(openGridPositionDialog()));

	// --- Network connection & Version check --------------------------------------------------------------------------------------------------------------
	// Limit the number of request:
	time_t timeNow, timeLast;
	time( &timeNow );
	QSettings settings;
	timeLast = settings.value( "lastVersionCheck" ).toLongLong();
        double daysSinceLastCheck = difftime( timeNow, timeLast ) / ( 24.0 * 3600.0 );
	// daysSinceLastCheck = 356.0; // for testing (1/2)
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] Last check " << daysSinceLastCheck << " days ago." << endl;
	if( daysSinceLastCheck > 3.0 ) {
		mNetworkManager = new QNetworkAccessManager( this );
		QObject::connect( mNetworkManager, &QNetworkAccessManager::finished, this, &QGMMainWindow::slotHttpCheckVersion );
		QNetworkRequest request;
		request.setUrl( QUrl( "https://www.gigamesh.eu/api.php/currentversion" ) );
		request.setRawHeader( "User-Agent", QString( "GigaMesh/%1" ).arg( VERSION_PACKAGE ).toStdString().c_str() );
		mNetworkManager->get( request );
	}
	// -----------------------------------------------------------------------------------------------------------------------------------------------------

        // -----------------------------------------------------------------------------------------------------------------------------------------------------

	// --- Check external Tools i.e. Inkscape and convert/ImageMagick --------------------------------------------------------------------------------------
	auto inkscapePath = settings.value("Inkscape_Path", "").toString();
	if(inkscapePath.length() == 0)
	    inkscapePath = "inkscape";
	bool checkInkscapeFailed = false;
	QProcess testRunInkscape;
	testRunInkscape.start( inkscapePath + " --version" );
	if( !testRunInkscape.waitForFinished() ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing Inkscape had a timeout!" << endl;
		checkInkscapeFailed = true;
	}
	if( testRunInkscape.exitStatus() != QProcess::NormalExit ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing Inkscape had no normal exit!" << endl;
		checkInkscapeFailed = true;
	}
	if( testRunInkscape.exitCode() != 0 ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR Inkscape exit code: " << testRunInkscape.exitCode() << endl;
		QString outInkscapeErr( testRunInkscape.readAllStandardError() );
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] Inkscape error: " << outInkscapeErr.toStdString().c_str() << endl;
		checkInkscapeFailed = true;
	}
	QString outInkscape( testRunInkscape.readAllStandardOutput() );
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] Inkscape check: " << outInkscape.simplified().toStdString().c_str() << endl;
	if( checkInkscapeFailed ) {
		SHOW_MSGBOX_WARN_TIMEOUT( tr("Inkscape error"), tr("Checking Inkscape for presence and functionality failed!"), 5000 );
	}
	// -----------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef REQUIRE_CONVERT_IMAGEMAGICK_OPTION
	// Due to DPI set within PNGs by Qt this is currently not required. Maybe for writings TIFFs, which need some source revision.
	bool checkConvertFailed = false;
	QProcess testRunConvert;
	testRunConvert.start( "convert -version" );
	if( !testRunConvert.waitForFinished() ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing Convert had a timeout!" << endl;
		checkConvertFailed = true;
	}
	if( testRunConvert.exitStatus() != QProcess::NormalExit ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing Convert had no normal exit!" << endl;
		checkConvertFailed = true;
	}
	if( testRunConvert.exitCode() != 0 ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR Convert exit code: " << testRunConvert.exitCode() << endl;
		QString outConvertErr( testRunConvert.readAllStandardError() );
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] Convert error: " << outConvertErr.toStdString().c_str() << endl;
		checkConvertFailed = true;
	}
	QString outConvert( testRunConvert.readAllStandardOutput() );
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] Convert check: " << outConvert.simplified().toStdString().c_str() << endl;
	if( checkConvertFailed ) {
		int timerMSec = 5000;
		if( checkInkscapeFailed ) {
			timerMSec +=  5000;
		}
		SHOW_MSGBOX_WARN_TIMEOUT( tr("ImageMagick error"), tr("Checking convert from the ImageMagick for presence and functionality failed!"), timerMSec );
	}
	// -----------------------------------------------------------------------------------------------------------------------------------------------------
#endif

	// --- Drag and Drop -----------------------------------------------------------------------------------------------------------------------------------
	setAcceptDrops( true );
	// -----------------------------------------------------------------------------------------------------------------------------------------------------
}

//! Destructor
QGMMainWindow::~QGMMainWindow() {

}

//! Initialization regarding signals to the MeshWidget - ONLY to be called ONCE from the constructor!
void QGMMainWindow::initMeshWidgetSignals() {
	// Setup group of flags for visualization - see mMeshWidget
	QList<QAction*> allActions = findChildren<QAction*>();

	// === mMeshWidget === FLAGS ============================================================================================================================

	// Add flag IDs for mMeshWidget class to menu actions:
	actionGridRectangular->setProperty(           "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_GRID_RECTANGULAR );
	actionGridHighlightCenter->setProperty(       "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_GRID_HIGHLIGHTCENTER );
	actionGrid_Center_Cross_in_front->setProperty("gmMeshWidgetFlag",       MeshWidgetParams::SHOW_GRID_HIGHLIGHTCENTER_FRONT);
	actionGridPolarLines->setProperty(            "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_GRID_POLAR_LINES );
	actionGridPolarCircles->setProperty(          "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_GRID_POLAR_CIRCLES );
	actionHistShow->setProperty(                  "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_HISTOGRAM );
	actionHistLog->setProperty(                   "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_HISTOGRAM_LOG );
	actionHistSceneRGB->setProperty(              "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_HISTOGRAM_SCENE );
	actionHistSceneRGBLog->setProperty(           "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_HISTOGRAM_SCENE_LOG );
	actionGigaMeshLogo->setProperty(              "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_GIGAMESH_LOGO_CANVAS );
	actionKeyboardLayout->setProperty(            "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_KEYBOARD_CAMERA );
	actionFog->setProperty(                       "gmMeshWidgetFlag",       MeshWidgetParams::SHOW_FOG );
	actionLightning->setProperty(                 "gmMeshWidgetFlag",       MeshWidgetParams::LIGHT_ENABLED );
	actionLightFixedCam->setProperty(             "gmMeshWidgetFlag",       MeshWidgetParams::LIGHT_FIXED_CAM );
	actionLightFixedWorld->setProperty(           "gmMeshWidgetFlag",       MeshWidgetParams::LIGHT_FIXED_WORLD );
	actionLightAmbient->setProperty(              "gmMeshWidgetFlag",       MeshWidgetParams::LIGHT_AMBIENT );
	actionScreenshotsCrop->setProperty(           "gmMeshWidgetFlag",       MeshWidgetParams::CROP_SCREENSHOTS );
	actionVideoFrameSize->setProperty(            "gmMeshWidgetFlag",       MeshWidgetParams::VIDEO_FRAME_FIXED );
	actionExportScreenShotsViewsSix->setProperty( "gmMeshWidgetFlag",       MeshWidgetParams::EXPORT_SIDE_VIEWS_SIX );
	actionExportSVGDashedAxis->setProperty(       "gmMeshWidgetFlag",       MeshWidgetParams::EXPORT_SVG_AXIS_DASHED );
	actionScreenshotDPISuffix->setProperty(       "gmMeshWidgetFlag",       MeshWidgetParams::SCREENSHOT_FILENAME_WITH_DPI );
	actionDisplay_as_pointcloud_when_moving->setProperty( "gmMeshWidgetFlag", MeshWidgetParams::ENABLE_SHOW_MESH_REDUCED);

	mMeshWidgetFlag = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshWidgetFlag" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshWidgetFlag );
	}
	mMeshWidgetFlag->setExclusive( false );
	// Connect the non-exclusive group:
	QObject::connect( mMeshWidgetFlag, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetFlag(QAction*)) );

	// Group the two radio buttons to switch between perspective and orthographic projection
	actionProjectOrthographic->setProperty(       "gmMeshWidgetFlag",       MeshWidgetParams::ORTHO_MODE );
	actionProjectPerspective->setProperty(        "gmMeshWidgetFlagInvert", MeshWidgetParams::ORTHO_MODE );
	mGroupPerspOrtho = new QActionGroup( this );
	actionProjectPerspective->setActionGroup( mGroupPerspOrtho );
	actionProjectOrthographic->setActionGroup( mGroupPerspOrtho );
	// Connect this exclusive group:
	QObject::connect( mGroupPerspOrtho, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetFlag(QAction*)) );

	// Group the two radio buttons to switch between perspective and orthographic projection
	// Used in File -> Export Image Stack to choose the axis for the spherical panorama
	actionSphericalImagesVertical->setProperty(   "gmMeshWidgetFlag",       MeshWidgetParams::SPHERICAL_VERTICAL );
	actionSphericalImagesHorizontal->setProperty( "gmMeshWidgetFlagInvert", MeshWidgetParams::SPHERICAL_VERTICAL );
	mGroupSpherePanoAxis = new QActionGroup( this );
	actionSphericalImagesVertical->setActionGroup( mGroupSpherePanoAxis );
	actionSphericalImagesHorizontal->setActionGroup( mGroupSpherePanoAxis );
	// Connect this exclusive group:
	QObject::connect( mGroupSpherePanoAxis, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetFlag(QAction*)) );

	// === mMeshWidget === INTEGER ==========================================================================================================================

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Non-Exclusive Group of Single integer parameters: (has to be first!)
	actionVideoFrameSizeSet->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::VIDEO_FRAME_WIDTH );
	actionLightVectorsShown->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::LIGHT_VECTORS_SHOWN_MAX );

	mMeshWidgetInteger = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshWidgetParamInt" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshWidgetInteger );
	}
	mMeshWidgetInteger->setExclusive( false );
	QObject::connect( mMeshWidgetInteger, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetParamInt(QAction*)) );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Exclusive Group in selection menu as radio-buttons:
	mGroupSelPrimitive = new QActionGroup( this );
	actionSelMVertsGUIPinPoint->setActionGroup( mGroupSelPrimitive );
	actionSelMVertsGUILasso->setActionGroup(    mGroupSelPrimitive );
	actionSelMFacesGUIPinPoint->setActionGroup( mGroupSelPrimitive );
	actionSelectVertex->setActionGroup(    mGroupSelPrimitive );
	actionSelectFace->setActionGroup(      mGroupSelPrimitive );
	actionSelectPlane3FP->setActionGroup(  mGroupSelPrimitive );
	actionSelectCone->setActionGroup(      mGroupSelPrimitive );
	actionSelectSphere->setActionGroup(    mGroupSelPrimitive );
	actionSelectPositions->setActionGroup( mGroupSelPrimitive );
	mGroupSelPrimitive->setExclusive( true );

	actionSelMVertsGUIPinPoint->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelMVertsGUILasso->setProperty(    "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelMFacesGUIPinPoint->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelectVertex->setProperty(    "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelectFace->setProperty(      "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelectPlane3FP->setProperty(  "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelectCone->setProperty(      "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelectSphere->setProperty(    "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );
	actionSelectPositions->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::SELECTION_MODE );

	actionSelMVertsGUIPinPoint->setProperty( "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_VERTICES  );
	actionSelMVertsGUILasso->setProperty(    "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO  );
	actionSelMFacesGUIPinPoint->setProperty( "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_MULTI_FACES  );
	actionSelectVertex->setProperty(    "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_VERTEX    );
	actionSelectFace->setProperty(      "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_FACE      );
	actionSelectPlane3FP->setProperty(  "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_PLANE_3FP );
	actionSelectCone->setProperty(      "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_CONE      );
	actionSelectSphere->setProperty(    "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_SPHERE    );
	actionSelectPositions->setProperty( "gmMeshWidgetParamValue", MeshWidgetParams::SELECTION_MODE_POSITIONS );

	// Connect this exclusive group:
	QObject::connect( mGroupSelPrimitive, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetParamInt(QAction*)) );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Exclusive Group in histogram menu as radio-buttons:
	mGroupSelHistType = new QActionGroup( this );
	actionHistFunctionValuesVertex->setActionGroup( mGroupSelHistType );
	actionHistFunctionValuesVertexLocalMin->setActionGroup( mGroupSelHistType );
	actionHistFunctionValuesVertexLocalMax->setActionGroup( mGroupSelHistType );
	actionHistFVELementsVertex->setActionGroup(     mGroupSelHistType );
	actionHistFVELementsVertexDim->setActionGroup(  mGroupSelHistType );
	actionHistFacesEdgeLength->setActionGroup(      mGroupSelHistType );
	actionHistFacesAnglesMin->setActionGroup(       mGroupSelHistType );
	actionHistFacesAnglesMax->setActionGroup(       mGroupSelHistType );
	actionPlotPolylineRunlength->setActionGroup(    mGroupSelHistType );
	mGroupSelHistType->setExclusive( true );

	actionHistFunctionValuesVertex->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFunctionValuesVertexLocalMin->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFunctionValuesVertexLocalMax->setProperty( "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFVELementsVertex->setProperty(     "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFVELementsVertexDim->setProperty(  "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFacesEdgeLength->setProperty(      "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFacesAnglesMin->setProperty(       "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionHistFacesAnglesMax->setProperty(       "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );
	actionPlotPolylineRunlength->setProperty(    "gmMeshWidgetParamInt", MeshWidgetParams::HISTOGRAM_TYPE );

	actionHistFunctionValuesVertex->setProperty( "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_FUNCTION_VALUES_VERTEX      );
	actionHistFunctionValuesVertexLocalMin->setProperty( "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_FUNCTION_VALUES_VERTEX_LOCAL_MINIMA   );
	actionHistFunctionValuesVertexLocalMax->setProperty( "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_FUNCTION_VALUES_VERTEX_LOCAL_MAXIMA   );
	actionHistFVELementsVertex->setProperty(     "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_FEATURE_ELEMENTS_VERTEX     );
	actionHistFVELementsVertexDim->setProperty(  "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_FEATURE_ELEMENTS_VERTEX_DIM );
	actionHistFacesEdgeLength->setProperty(      "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_EDGE_LENGTH                 );
	actionHistFacesAnglesMin->setProperty(       "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_ANGLES_FACES_MINIMUM        );
	actionHistFacesAnglesMax->setProperty(       "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_ANGLES_FACES_MAXIMUM        );
	actionPlotPolylineRunlength->setProperty(    "gmMeshWidgetParamValue", MeshParams::HISTOGRAM_POLYLINE_RUNLENGHTS         );

	// Connect this exclusive group:
	QObject::connect( mGroupSelHistType, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetParamInt(QAction*)) );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// =====================================================================================================================================================

	// === mMeshWidget === FLOAT ===========================================================================================================================

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Non-Exclusive Group of Single float parameters: (has to be first!)

	actionGridShiftDepth->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::GRID_SHIFT_DEPTH );
	actionGridShiftDepth->setProperty( "gmMeshWidgetParamValue",      0.0 );
	actionGridShiftDepth->setProperty( "gmMeshWidgetParamValueMin",  -2.0 );
	actionGridShiftDepth->setProperty( "gmMeshWidgetParamValueMax",   0.0 );

	actionSetFOV->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::FOV_ANGLE );
	actionSetFOV->setProperty( "gmMeshWidgetParamValue",     40.0 );
	actionSetFOV->setProperty( "gmMeshWidgetParamValueMin",   0.0 );
	actionSetFOV->setProperty( "gmMeshWidgetParamValueMax", 180.0 );

	actionSelectAmbient->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::AMBIENT_LIGHT );
	actionSelectAmbient->setProperty( "gmMeshWidgetParamValue",    1.0 );
	actionSelectAmbient->setProperty( "gmMeshWidgetParamValueMin", 0.0 );
	actionSelectAmbient->setProperty( "gmMeshWidgetParamValueMax", 5.0 );

	actionLightFixedCamIntensity->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::LIGHT_FIXED_CAM_INTENSITY );
	actionLightFixedCamIntensity->setProperty( "gmMeshWidgetParamValue",    1.0 );
	actionLightFixedCamIntensity->setProperty( "gmMeshWidgetParamValueMin", 0.0 );
	actionLightFixedCamIntensity->setProperty( "gmMeshWidgetParamValueMax", 5.0 );

	actionLightFixedWorldIntensity->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::LIGHT_FIXED_WORLD_INTENSITY );
	actionLightFixedWorldIntensity->setProperty( "gmMeshWidgetParamValue",    1.0 );
	actionLightFixedWorldIntensity->setProperty( "gmMeshWidgetParamValueMin", 0.0 );
	actionLightFixedWorldIntensity->setProperty( "gmMeshWidgetParamValueMax", 5.0 );

	actionLightFixedDirectionPhi->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::LIGHT_FIXED_CAM_ANGLE_PHI );
	actionLightFixedDirectionPhi->setProperty( "gmMeshWidgetParamValue",      40.0 );
	actionLightFixedDirectionPhi->setProperty( "gmMeshWidgetParamValueMin", -180.0 );
	actionLightFixedDirectionPhi->setProperty( "gmMeshWidgetParamValueMax",  180.0 );

	actionLightFixedDirectionTheta->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::LIGHT_FIXED_CAM_ANGLE_THETA );
	actionLightFixedDirectionTheta->setProperty( "gmMeshWidgetParamValue",      40.0 );
	actionLightFixedDirectionTheta->setProperty( "gmMeshWidgetParamValueMin", -180.0 );
	actionLightFixedDirectionTheta->setProperty( "gmMeshWidgetParamValueMax",  180.0 );

	actionSelMatSpecular->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::MATERIAL_SPECULAR );
	actionSelMatSpecular->setProperty( "gmMeshWidgetParamValue",    1.0 );
	actionSelMatSpecular->setProperty( "gmMeshWidgetParamValueMin", 0.0 );
	actionSelMatSpecular->setProperty( "gmMeshWidgetParamValueMax", 1.0 );

	actionSelMatShininess->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::MATERIAL_SHININESS );
	actionSelMatShininess->setProperty( "gmMeshWidgetParamValue",          2.65 );
	actionSelMatShininess->setProperty( "gmMeshWidgetParamValueMin", log(  1.0) ); // equal 0.0
	actionSelMatShininess->setProperty( "gmMeshWidgetParamValueMax", log(129.0) ); // log(128.0+1.0)!

	actionFogLinearDistMin->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::FOG_LINEAR_START );
	actionFogLinearDistMax->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::FOG_LINEAR_END );

	actionSaveStillImages360DurationSlow->setProperty( "gmMeshWidgetParamFloat", MeshWidgetParams::VIDEO_SLOW_STARTSTOP );

	mMeshWidgetFloat = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshWidgetParamFloat" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshWidgetFloat );
	}
	mMeshWidgetFloat->setExclusive( false );
	QObject::connect( mMeshWidgetFloat, SIGNAL(triggered(QAction*)), this, SLOT(setMeshWidgetParamFloat(QAction*)) );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// =====================================================================================================================================================
}

//! Initialization regarding signals to the Mesh/MeshGL/MeshQt - ONLY to be called ONCE from the constructor!
void QGMMainWindow::initMeshSignals() {
	// Fetch all actions:
	QList<QAction*> allActions = findChildren<QAction*>();

	// === MeshGL - FLOAT ==================================================================================================================================

	// DOUBLE: Add parameter, double IDs for MeshGL class:
	actionIsoValSet->setProperty(         "gmMeshParamFloat", MeshParams::FUNC_VALUE_THRES   );
	actionCylinderSetRadius->setProperty( "gmMeshParamFloat", MeshParams::CYLINDER_RADIUS );

	// DOUBLE: Setup parameter group of menu items
	mMeshParDbl = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshParamFloat" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshParDbl );
	}
	mMeshParDbl->setExclusive( false );

	// ALL: Connect:
	QObject::connect( mMeshParDbl, &QActionGroup::triggered, this, &QGMMainWindow::setMeshGLParamFloat );

	// === MeshGL - FLAGS ==================================================================================================================================

	// Add flag IDs for MeshGL class to menu actions:
	actionViewVerticesAll->setProperty(             "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_ALL );
	actionViewVerticesSolo->setProperty(            "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_SOLO );
	actionViewVerticesBorder->setProperty(          "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_BORDER );
	actionViewVerticesNonManifold->setProperty(     "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_NON_MANIFOLD );
	actionViewVerticesSingular->setProperty(        "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_SINGULAR );
	actionViewVerticesLocalMinima->setProperty(     "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_LOCAL_MIN );
	actionViewVerticesLocalMaxima->setProperty(     "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_LOCAL_MAX );
	actionViewVerticesSelected->setProperty(        "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_SELECTION );
	actionViewVerticesSynthetic->setProperty(       "gmMeshGLFlag", MeshGLParams::SHOW_VERTICES_SYNTHETIC );
	actionViewFacesSelected->setProperty(           "gmMeshGLFlag", MeshGLParams::SHOW_FACES_SELECTION );
	actionViewMeshPlaneClipping->setProperty(       "gmMeshGLFlag", MeshGLParams::SHOW_MESH_PLANE_AS_CLIPLANE );
	actionViewClipThruSelPrim->setProperty(         "gmMeshGLFlag", MeshGLParams::SHOW_CLIP_THRU_SEL );
	actionFacesBackfaceCulling->setProperty(        "gmMeshGLFlag", MeshGLParams::SHOW_FACES_CULLED );
	actionFace_Backface_Lightning->setProperty(		"gmMeshGLFlag", MeshGLParams::BACKFACE_LIGHTING );
	actionViewDatumSpheres->setProperty(            "gmMeshGLFlag", MeshGLParams::SHOW_DATUM_SPHERES );
	actionViewDatumBoxes->setProperty(              "gmMeshGLFlag", MeshGLParams::SHOW_DATUM_BOXES );
	actionViewFaces->setProperty(                   "gmMeshGLFlag", MeshGLParams::SHOW_FACES );
	actionViewEdges->setProperty(                   "gmMeshGLFlag", MeshGLParams::SHOW_FACES_EDGES );
	actionViewMeshAxis->setProperty(                "gmMeshGLFlag", MeshGLParams::SHOW_MESH_AXIS );
	actionViewMeshPlane->setProperty(               "gmMeshGLFlag", MeshGLParams::SHOW_MESH_PLANE );
	actionNormalsVertex->setProperty(               "gmMeshGLFlag", MeshGLParams::SHOW_NORMALS_VERTEX );
	actionNormalsFace->setProperty(                 "gmMeshGLFlag", MeshGLParams::SHOW_NORMALS_FACE );
	actionNormalsPolyline->setProperty(             "gmMeshGLFlag", MeshGLParams::SHOW_NORMALS_POLYLINE );
	actionNormalsPolylineMain->setProperty(         "gmMeshGLFlag", MeshGLParams::SHOW_NORMALS_POLYLINE_MAIN );
	actionViewBoundingBox->setProperty(             "gmMeshGLFlag", MeshGLParams::SHOW_BOUNDING_BOX );
	actionViewBoundingBoxEnclosed->setProperty(     "gmMeshGLFlag", MeshGLParams::SHOW_BOUNDING_BOX_ENCLOSED );
	actionViewPolylines->setProperty(               "gmMeshGLFlag", MeshGLParams::SHOW_POLYLINES );
	actionViewPolylinesCurv->setProperty(           "gmMeshGLFlag", MeshGLParams::SHOW_POLYLINES_CURVATURE );
	actionViewPolylinesCurvAbs->setProperty(        "gmMeshGLFlag", MeshGLParams::SHOW_POLYLINES_CURVATURE_ABS );
	actionSmoothShading->setProperty(               "gmMeshGLFlag", MeshGLParams::SHOW_SMOOTH );
	actionShowLablesMono->setProperty(              "gmMeshGLFlag", MeshGLParams::SHOW_LABELS_MONO_COLOR );
	actionVisMapInvert->setProperty(                "gmMeshGLFlag", MeshGLParams::SHOW_COLMAP_INVERT );
	actionIsolines->setProperty(                    "gmMeshGLFlag", MeshGLParams::SHOW_FUNC_VALUES_ISOLINES );
	actionIsolinesOnly->setProperty(                "gmMeshGLFlag", MeshGLParams::SHOW_FUNC_VALUES_ISOLINES_ONLY );
	actionIsolinesSolid->setProperty(               "gmMeshGLFlag", MeshGLParams::SHOW_FUNC_VALUES_ISOLINES_SOLID );
	actionRepeatMapWaves->setProperty(              "gmMeshGLFlag", MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL );
	actionBad_Lit_Areas->setProperty(               "gmMeshGLFlag", MeshGLParams::SHOW_BADLIT_AREAS);

	// Setup group of flags for visualization - see MeshGL and MeshQT
	mMeshGLFlag = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshGLFlag" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setCheckable( true ); // Ensure that it can be toggled instead of triggered.
		currAction->setActionGroup( mMeshGLFlag );
	}
	mMeshGLFlag->setExclusive( false );
	// Connect the non-exclusive group:
	QObject::connect( mMeshGLFlag, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLFlag(QAction*)) );

	// === MeshGL - INTEGER ================================================================================================================================

	// INT: Add parameter, double IDs for MeshGL class:
	actionLabelColorShift->setProperty( "gmMeshGLParamInt", MeshGLParams::COLMAP_LABEL_OFFSET );

	// INT: Setup parameter group of menu items
	mMeshGLParInt = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshGLParamInt" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshGLParInt );
	}
	mMeshGLParInt->setExclusive( false );

	// Connect:
	QObject::connect( mMeshGLParInt, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLParamInt(QAction*)) );

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Exclusive Group for colormaps:
	mMeshGLGroupSelColormap = new QActionGroup( this );
	actionVisMapHot->setActionGroup(            mMeshGLGroupSelColormap );
	actionVisMapCold->setActionGroup(           mMeshGLGroupSelColormap );
	actionVisMapHSV->setActionGroup(            mMeshGLGroupSelColormap );
	actionVisMapHSVPart->setActionGroup(        mMeshGLGroupSelColormap );
	actionVisMapGrayscale->setActionGroup(      mMeshGLGroupSelColormap );
	actionVisMapHypsometric->setActionGroup(    mMeshGLGroupSelColormap );
	actionVisMapRdGy->setActionGroup(           mMeshGLGroupSelColormap );
	actionVisMapSpectral->setActionGroup(       mMeshGLGroupSelColormap );
	actionVisMapRdYlGn->setActionGroup(         mMeshGLGroupSelColormap );
	actionVisMapJet->setActionGroup(            mMeshGLGroupSelColormap );
	actionVisMapMorgenstemning->setActionGroup( mMeshGLGroupSelColormap );
	actionVisMapHypsoHirise1->setActionGroup(   mMeshGLGroupSelColormap );
	actionVisMapHypsoHirise2->setActionGroup(   mMeshGLGroupSelColormap );
	actionVisMapParula->setActionGroup(         mMeshGLGroupSelColormap );
	actionVisMapYlOrBr->setActionGroup(         mMeshGLGroupSelColormap );
	actionVisMapCopper->setActionGroup(         mMeshGLGroupSelColormap );
	actionVisMapRusttones->setActionGroup(      mMeshGLGroupSelColormap );
	actionVisMapSiennatones->setActionGroup(    mMeshGLGroupSelColormap );
	actionVisMapHypsoArid->setActionGroup(      mMeshGLGroupSelColormap );
	mMeshGLGroupSelColormap->setExclusive( true );

	actionVisMapHot->setProperty(            "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapHot->setProperty(            "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HOT             );

	actionVisMapCold->setProperty(           "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapCold->setProperty(           "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_COLD            );

	actionVisMapHSV->setProperty(            "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapHSV->setProperty(            "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HSV             );

	actionVisMapHSVPart->setProperty(        "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapHSVPart->setProperty(        "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HSV_PART        );

	actionVisMapGrayscale->setProperty(      "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapGrayscale->setProperty(      "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_GRAYSCALE       );

	actionVisMapHypsometric->setProperty(    "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapHypsometric->setProperty(    "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HYPSO           );

	actionVisMapRdGy->setProperty(           "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapRdGy->setProperty(           "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_BREWER_RDGY     );

	actionVisMapSpectral->setProperty(       "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapSpectral->setProperty(       "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_BREWER_SPECTRAL );

	actionVisMapRdYlGn->setProperty(         "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapRdYlGn->setProperty(         "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_BREWER_RDYLGN   );

	actionVisMapJet->setProperty(            "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapJet->setProperty(            "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_OCTAVE_JET      );

	actionVisMapMorgenstemning->setProperty( "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapMorgenstemning->setProperty( "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_MORGENSTEMNING  );

	actionVisMapHypsoHirise1->setProperty(    "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapHypsoHirise1->setProperty(    "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HYPSO_HIRISE1   );

	actionVisMapHypsoHirise2->setProperty(    "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapHypsoHirise2->setProperty(    "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HYPSO_HIRISE2   );

	actionVisMapParula->setProperty(          "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapParula->setProperty(          "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_PARULA          );

	actionVisMapYlOrBr->setProperty(          "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapYlOrBr->setProperty(          "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_BREWER_YLORBR   );

	actionVisMapCopper->setProperty(          "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapCopper->setProperty(          "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_OCTAVE_COPPER   );

	actionVisMapRusttones->setProperty(       "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE          );
	actionVisMapRusttones->setProperty(       "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_RUSTTONES       );

	actionVisMapSiennatones->setProperty(       "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE        );
	actionVisMapSiennatones->setProperty(       "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_SIENNATONES   );

	actionVisMapHypsoArid->setProperty(         "gmMeshGLParamInt",   MeshGLParams::GLSL_COLMAP_CHOICE        );
	actionVisMapHypsoArid->setProperty(         "gmMeshGLParamValue", MeshGLParams::GLSL_COLMAP_HYPSO_ARID    );

	// Connect:
	QObject::connect( mMeshGLGroupSelColormap, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLParamInt(QAction*)) );

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Exclusive Group for vertex sprite shapes
	mMeshGLGroupSelSpriteShape = new QActionGroup( this );
	actionVisSpriteShapeBox->setActionGroup(          mMeshGLGroupSelSpriteShape );
	actionVisSpriteShapeDisc->setActionGroup(         mMeshGLGroupSelSpriteShape );
	actionVisSpriteShapePolarRose->setActionGroup(    mMeshGLGroupSelSpriteShape );
	actionVisSpriteShapeStarRounded->setActionGroup(  mMeshGLGroupSelSpriteShape );
	mMeshGLGroupSelSpriteShape->setExclusive( true );

	actionVisSpriteShapeBox->setProperty(          "gmMeshGLParamInt",   MeshGLParams::VERTEX_SPRITE_SHAPE );
	actionVisSpriteShapeDisc->setProperty(         "gmMeshGLParamInt",   MeshGLParams::VERTEX_SPRITE_SHAPE );
	actionVisSpriteShapePolarRose->setProperty(    "gmMeshGLParamInt",   MeshGLParams::VERTEX_SPRITE_SHAPE );
	actionVisSpriteShapeStarRounded->setProperty(  "gmMeshGLParamInt",   MeshGLParams::VERTEX_SPRITE_SHAPE );

	actionVisSpriteShapeBox->setProperty(          "gmMeshGLParamValue", MeshGLParams::SPRITE_SHAPE_BOX          );
	actionVisSpriteShapeDisc->setProperty(         "gmMeshGLParamValue", MeshGLParams::SPRITE_SHAPE_DISC         );
	actionVisSpriteShapePolarRose->setProperty(    "gmMeshGLParamValue", MeshGLParams::SPRITE_SHAPE_POLAR_ROSE   );
	actionVisSpriteShapeStarRounded->setProperty(  "gmMeshGLParamValue", MeshGLParams::SPRITE_SHAPE_STAR_ROUNDED );

	// Connect:
	QObject::connect( mMeshGLGroupSelSpriteShape, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLParamInt(QAction*)) );

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Exclusive Group for min/max treatment of function value visualization
	mGroupVisFuncCutOff = new QActionGroup( this );
	actionVisMapAutoMinMax->setActionGroup(  mGroupVisFuncCutOff );
	actionVisMapQuantil->setActionGroup(     mGroupVisFuncCutOff );
	actionVisMapFixedMinMax->setActionGroup( mGroupVisFuncCutOff );
	mGroupVisFuncCutOff->setExclusive( true );

	actionVisMapAutoMinMax->setProperty(  "gmMeshGLParamInt", MeshGLParams::FUNCVAL_CUTOFF_CHOICE );
	actionVisMapQuantil->setProperty(     "gmMeshGLParamInt", MeshGLParams::FUNCVAL_CUTOFF_CHOICE );
	actionVisMapFixedMinMax->setProperty( "gmMeshGLParamInt", MeshGLParams::FUNCVAL_CUTOFF_CHOICE );

	actionVisMapAutoMinMax->setProperty(  "gmMeshGLParamValue", MeshGLParams::FUNCVAL_CUTOFF_MINMAX_AUTO );
	actionVisMapQuantil->setProperty(     "gmMeshGLParamValue", MeshGLParams::FUNCVAL_CUTOFF_QUANTIL     );
	actionVisMapFixedMinMax->setProperty( "gmMeshGLParamValue", MeshGLParams::FUNCVAL_CUTOFF_MINMAX_USER );

	// Connect:
	QObject::connect( mGroupVisFuncCutOff, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLParamInt(QAction*)) );

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Exclusive Group for min/max treatment of function value visualization
	mMeshGLGroupVertexSpriteColor = new QActionGroup( this );
	actionViewVerticesAllMono->setActionGroup(    mMeshGLGroupVertexSpriteColor );
	actionViewVerticesAllColor->setActionGroup(   mMeshGLGroupVertexSpriteColor );
	actionViewVerticesAllFuncVal->setActionGroup( mMeshGLGroupVertexSpriteColor );
	actionViewVerticesAllLabel->setActionGroup(   mMeshGLGroupVertexSpriteColor );
	mMeshGLGroupVertexSpriteColor->setExclusive( true );

	actionViewVerticesAllMono->setProperty(    "gmMeshGLParamInt", MeshGLParams::TEXMAP_CHOICE_VERETX_SPRITES );
	actionViewVerticesAllColor->setProperty(   "gmMeshGLParamInt", MeshGLParams::TEXMAP_CHOICE_VERETX_SPRITES );
	actionViewVerticesAllFuncVal->setProperty( "gmMeshGLParamInt", MeshGLParams::TEXMAP_CHOICE_VERETX_SPRITES );
	actionViewVerticesAllLabel->setProperty(   "gmMeshGLParamInt", MeshGLParams::TEXMAP_CHOICE_VERETX_SPRITES );

	actionViewVerticesAllMono->setProperty(    "gmMeshGLParamValue", MeshGLParams::TEXMAP_VERT_MONO    );
	actionViewVerticesAllColor->setProperty(   "gmMeshGLParamValue", MeshGLParams::TEXMAP_VERT_RGB     );
	actionViewVerticesAllFuncVal->setProperty( "gmMeshGLParamValue", MeshGLParams::TEXMAP_VERT_FUNCVAL );
	actionViewVerticesAllLabel->setProperty(   "gmMeshGLParamValue", MeshGLParams::TEXMAP_VERT_LABELS  );

	// Connect:
	QObject::connect( mMeshGLGroupVertexSpriteColor, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLParamInt(QAction*)) );

	// === MeshGL - FLOAT ==================================================================================================================================

	// DOUBLE: Add parameter, double IDs for MeshGL class:
	actionBoundingBoxLineWidth->setProperty( "gmMeshGLParamFloat", MeshGLParams::BOUNDING_BOX_LINEWIDTH );

	actionSelDatSphereTransp->setProperty( "gmMeshGLParamFloat", MeshGLParams::DATUM_SPHERE_TRANS );
	actionSelDatSphereTransp->setProperty( "gmMeshGLParamValue",    0.90f );
	actionSelDatSphereTransp->setProperty( "gmParamValueMin", 0.00f );
	actionSelDatSphereTransp->setProperty( "gmParamValueMax", 1.00f );

	actionVisMapQuantilSetMin->setProperty( "gmMeshGLParamFloat", MeshGLParams::TEXMAP_QUANTIL_MIN );
	actionVisMapQuantilSetMin->setProperty( "gmMeshGLParamValue",    0.01f );
	actionVisMapQuantilSetMin->setProperty( "gmParamValueMin", 0.00f );
	actionVisMapQuantilSetMin->setProperty( "gmParamValueMax", 1.00f );

	actionVisMapQuantilSetMax->setProperty( "gmMeshGLParamFloat", MeshGLParams::TEXMAP_QUANTIL_MAX );
	actionVisMapQuantilSetMax->setProperty( "gmMeshGLParamValue",    0.99f );
	actionVisMapQuantilSetMax->setProperty( "gmParamValueMin", 0.00f );
	actionVisMapQuantilSetMax->setProperty( "gmParamValueMax", 1.00f );

	actionVisMapFixedSetMin->setProperty( "gmMeshGLParamFloat", MeshGLParams::TEXMAP_FIXED_MIN );
	actionVisMapFixedSetMax->setProperty( "gmMeshGLParamFloat", MeshGLParams::TEXMAP_FIXED_MAX );

	actionVisPolyLineWidth->setProperty( "gmMeshGLParamFloat", MeshGLParams::POLYLINE_WIDTH );

	actionIsolinesDistance->setProperty( "gmMeshGLParamFloat", MeshGLParams::ISOLINES_DISTANCE );

	actionIsolinesOffset->setProperty( "gmMeshGLParamFloat", MeshGLParams::ISOLINES_OFFSET );

	actionIsolinesWidthPixel->setProperty( "gmMeshGLParamFloat", MeshGLParams::ISOLINES_PIXEL_WIDTH );
	actionIsolinesWidthPixel->setProperty( "gmMeshGLParamValue",    1.50f );
	actionIsolinesWidthPixel->setProperty( "gmParamValueMin",       0.00f );
	actionIsolinesWidthPixel->setProperty( "gmParamValueMax",      16.00f );

	actionVisMapWavelength->setProperty( "gmMeshGLParamFloat", MeshGLParams::WAVES_COLMAP_LEN );

	actionNormalsLength->setProperty( "gmMeshGLParamFloat", MeshGLParams::NORMALS_LENGTH );
	actionNormalsWidth->setProperty(  "gmMeshGLParamFloat", MeshGLParams::NORMALS_WIDTH  );

	actionVisMapLog->setProperty( "gmMeshGLParamFloat", MeshGLParams::FUNC_VALUE_LOG_GAMMA );
	actionVisMapLog->setProperty( "gmMeshGLParamValue",     1.00f );
	actionVisMapLog->setProperty( "gmParamValueMin", -2.50f );
	actionVisMapLog->setProperty( "gmParamValueMax", +2.50f );

    actionBad_Lit_Areas_Lower_Threshold->setProperty( "gmMeshGLParamFloat", MeshGLParams::BADLIT_LOWER_THRESHOLD);
    actionBad_Lit_Areas_Lower_Threshold->setProperty( "gmMeshGLParamValue",     0.05f );
    actionBad_Lit_Areas_Lower_Threshold->setProperty( "gmParamValueMin",  0.00f );
    actionBad_Lit_Areas_Lower_Threshold->setProperty( "gmParamValueMax",  1.00f );

    actionBad_Lit_Areas_Upper_Threshold->setProperty( "gmMeshGLParamFloat", MeshGLParams::BADLIT_UPPER_THRESHOLD);
    actionBad_Lit_Areas_Upper_Threshold->setProperty( "gmMeshGLParamValue",     0.995f );
    actionBad_Lit_Areas_Upper_Threshold->setProperty( "gmParamValueMin",  0.00f );
    actionBad_Lit_Areas_Upper_Threshold->setProperty( "gmParamValueMax",  1.00f );

	actionPin_Size->setProperty( "gmMeshGLParamFloat", MeshGLParams::PIN_SIZE);
	actionPin_Size->setProperty( "gmMeshGLParamValue", 1.0f);
	actionPin_Size->setProperty( "gmParamValueMin",  0.01f );
	actionPin_Size->setProperty( "gmParamValueMax",  100.00f );

	actionPin_Line_Height->setProperty( "gmMeshGLParamFloat", MeshGLParams::PIN_LINE_HEIGHT);
	actionPin_Line_Height->setProperty( "gmMeshGLParamValue", 0.5f);
	actionPin_Line_Height->setProperty( "gmParamValueMin",  0.0f );
	actionPin_Line_Height->setProperty( "gmParamValueMax",  1.0f );
	
	actionPointcloud_pointsize->setProperty( "gmMeshGLParamFloat", MeshGLParams::POINTCLOUD_POINTSIZE);
	actionPointcloud_pointsize->setProperty( " gmMeshGLParamValue", 3.0f);
	actionPointcloud_pointsize->setProperty( "gmParamValueMin", 1.0f);
	actionPointcloud_pointsize->setProperty( "gmParamValueMax", 50.0f);

	// DOUBLE: Setup parameter group of menu items
	mMeshGLParDbl = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshGLParamFloat" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshGLParDbl );
	}
	mMeshGLParDbl->setExclusive( false );

	// ALL: Connect:
	QObject::connect( mMeshGLParDbl, &QActionGroup::triggered, this, &QGMMainWindow::setMeshGLParamFloat );

	// === MeshGL/MeshQt - COLOR ===========================================================================================================================
	actionSelectColorSolid->setProperty(          "gmMeshGLColor", MeshGLColors::COLOR_MESH_SOLID         );
	actionSelectColorBackface->setProperty(       "gmMeshGLColor", MeshGLColors::COLOR_MESH_BACKFACE      );
	actionSelectColorVertexMono->setProperty(     "gmMeshGLColor", MeshGLColors::COLOR_VERTEX_MONO        );
	actionSelectColorVertexLocalMin->setProperty( "gmMeshGLColor", MeshGLColors::COLOR_VERTEX_LOCAL_MIN   );
	actionSelectColorVertexLocalMax->setProperty( "gmMeshGLColor", MeshGLColors::COLOR_VERTEX_LOCAL_MAX   );
	actionSelectColorEdgeMono->setProperty(       "gmMeshGLColor", MeshGLColors::COLOR_EDGE_MONO          );
	actionSelectColorPolyline->setProperty(       "gmMeshGLColor", MeshGLColors::COLOR_POLYLINE_MONO      );
	actionSelectColorNoLabel->setProperty(        "gmMeshGLColor", MeshGLColors::COLOR_LABEL_NOT_ASSIGNED );
	actionSelectColorLabelsMono->setProperty(     "gmMeshGLColor", MeshGLColors::COLOR_LABEL_SOLID        );

	mMeshGLColors = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshGLColor" );
		if( !someFlag.isValid() ) {
			continue;
		}
		currAction->setActionGroup( mMeshGLColors );
	}
	mMeshGLColors->setExclusive( false );

	// ALL: Connect:
	QObject::connect( mMeshGLColors, SIGNAL(triggered(QAction*)), this, SLOT(setMeshGLColor(QAction*)) );

	// === MeshGL/MeshQt - Function/Method CALL ============================================================================================================
	// ... File load, save, import, export  ................................................................................................................
	actionFileSaveAs->setProperty(                                "gmMeshFunctionCall", MeshParams::FILE_SAVE_AS                                 );
	actionImportVertexCoordinatesFromCSV->setProperty(            "gmMeshGLFunctionCall", MeshGLParams::IMPORT_COORDINATES_OF_VERTICES           );
	actionExportCoordinatesOfAllVerticesAsCSV->setProperty(       "gmMeshFunctionCall", MeshParams::EXPORT_COORDINATES_OF_VERTICES               );
	actionExportCoordinatesOfSelectedVerticesAsCSV->setProperty(  "gmMeshFunctionCall", MeshParams::EXPORT_COORDINATES_OF_SELECTED_VERTICES      );
	actionExportCoordinatesOfSelectedPrimitivesAsCSV->setProperty("gmMeshFunctionCall", MeshParams::EXPORT_SELPRIMS_POSITIONS                    );
	// ... View ...........................................................................................................................................
	actionMeshPlaneFlip->setProperty(                             "gmMeshFunctionCall", MeshParams::PLANE_FLIP                                   );
	// ... Selection & Definition ..........................................................................................................................
	actionSelectVertexEnterIndex->setProperty(                    "gmMeshFunctionCall", MeshParams::SELPRIM_VERTEX_BY_INDEX                      );
	actionSelVertFlagSynthetic->setProperty(                      "gmMeshFunctionCall", MeshParams::SELMVERTS_FLAG_SYNTHETIC                     );
	actionSelVertFlagCircleCenter->setProperty(                   "gmMeshFunctionCall", MeshParams::SELMVERTS_FLAG_CIRCLE_CENTER                 );
	actionSelVertInvert->setProperty(                             "gmMeshFunctionCall", MeshParams::SELMVERTS_INVERT                             );
	actionSelVertLabelNo->setProperty(                            "gmMeshFunctionCall", MeshParams::SELMVERTS_LABEL_IDS                          );
	actionSelVertLabelBackGrd->setProperty(                       "gmMeshFunctionCall", MeshParams::SELMVERTS_LABEL_BACKGROUND                   );
	actionSelVertFromSelMFaces->setProperty(                      "gmMeshFunctionCall", MeshParams::SELMVERTS_FROMSELMFACES                      );
	actionSelVertRidges->setProperty(                             "gmMeshFunctionCall", MeshParams::SELMVERTS_RIDGES                             );
	actionSelVertsByIdxShow->setProperty(                         "gmMeshFunctionCall", MeshParams::SELMVERTS_SHOW_INDICES                       );
	actionSelVertsByIdx->setProperty(                             "gmMeshFunctionCall", MeshParams::SELMVERTS_SELECT_INDICES                     );
	actionSelVertsRandom->setProperty(                            "gmMeshFunctionCall", MeshParams::SELMVERTS_RANDOM                             );
	actionSelMFacesWithSyntheticVertices->setProperty(            "gmMeshFunctionCall", MeshParams::SELMFACES_WITH_SYNTHETIC_VERTICES            );
	actionSelMFacesBorderWithThreeVertices->setProperty(          "gmMeshFunctionCall", MeshParams::SELMFACES_WITH_THREE_BORDER_VERTICES         );
	actionSelMFacesWithThreeVerticesSelected->setProperty(        "gmMeshFunctionCall", MeshParams::SELMFACES_WITH_THREE_SELECTED_VERTICES       );
	actionSelMFacesBorderBridgeTriConn->setProperty(              "gmMeshFunctionCall", MeshParams::SELMFACES_BORDER_BRIDGE_TRICONN              );
	actionSelMFacesBorderBridge->setProperty(                     "gmMeshFunctionCall", MeshParams::SELMFACES_BORDER_BRIDGE                      );
	actionSelMFacesBorderDangling->setProperty(                   "gmMeshFunctionCall", MeshParams::SELMFACES_BORDER_DANGLING                    );
	actionSelMFacesLabledVerticesVoronoiCorner->setProperty(      "gmMeshFunctionCall", MeshParams::SELMFACES_LABEL_CORNER                       );
	actionSelPolyVerticesNr->setProperty(                         "gmMeshFunctionCall", MeshParams::SELMPOLY_BY_VERTEX_COUNT                     );
	actionPlaneSetByAxisSelPrim->setProperty(                     "gmMeshFunctionCall", MeshParams::SELECT_MESH_PLANE_AXIS_SELPRIM               );
	actionPlaneSetByAxisLastSelPos->setProperty(                  "gmMeshFunctionCall", MeshParams::SELECT_MESH_PLANE_AXIS_SELPOS                );
	actionPlaneOrientTowardsAxis->setProperty(                    "gmMeshFunctionCall", MeshParams::ORIENT_MESH_PLANE_TO_AXIS                    );
	// ... Feature vectors ...................................................................................................................................
	actionFeatureVecMeanOneRingRepeat->setProperty(               "gmMeshFunctionCall", MeshParams::FEATUREVEC_MEAN_ONE_RING_REPEAT              );
	actionUnloadFeatureVectors->setProperty(                      "gmMeshFunctionCall", MeshParams::FEATUREVEC_UNLOAD_ALL                        );
	// ... Function values ...................................................................................................................................
	actionFuncValFeatureVecElementsStdDev->setProperty(           "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_STDDEV_ELEMENTS        );
	actionFuncValFeatureVecCorrelateWith->setProperty(            "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_CORRELATE_WITH         );
	actionFuncValFeatureVecPNorm->setProperty(                    "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_APPLY_PNORM            );
	actionFuncValFeatureVecMahalanobis->setProperty(              "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_APPLY_MAHALANOBIS      );
	actionFeatElement->setProperty(                               "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_ELEMENT_BY_INDEX       );
	actionDistanceToPrimSelCOG->setProperty(                      "gmMeshFunctionCall", MeshParams::FUNCVAL_DISTANCE_TO_SELPRIM                  );
	actionFuncValPlaneAngle->setProperty(                         "gmMeshFunctionCall", MeshParams::FUNCVAL_PLANE_ANGLE                          );
	actionFuncValAngleToRadial->setProperty(                      "gmMeshFunctionCall", MeshParams::FUNCVAL_ANGLE_TO_RADIAL                      );
	actionFuncValAxisAngleToRadial->setProperty(                  "gmMeshFunctionCall", MeshParams::FUNCVAL_AXIS_ANGLE_TO_RADIAL                 );
	actionFuncValOrthogonalAxisAngleToRaial->setProperty(         "gmMeshFunctionCall", MeshParams::FUNCVAL_ORTHOGONAL_AXIS_ANGLE_TO_RADIAL      );
	actionFuncValFeatureVecMin->setProperty(                      "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_MIN_ELEMENT            );
	actionFuncValFeatureVecMax->setProperty(                      "gmMeshFunctionCall", MeshParams::FUNCVAL_FEATUREVECTOR_MAX_ELEMENT            );
	actionColorRGBAvgToFuncVal->setProperty(                      "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_GRAY_RGB_AVERAGE                 );
	actionColorRGBAvgWeigthToFuncVal->setProperty(                "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_GRAY_RGB_AVERAGE_WEIGHTED        );
	actionColorRGBSaturationRemovalToFuncVal->setProperty(        "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_GRAY_SATURATION_REMOVAL          );
	actionColorHSVComponentToFuncVal->setProperty(                "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_GRAY_HSV_DECOMPOSITION           );
	actionDistanceToLineDir->setProperty(                         "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_DISTANCE_TO_LINE                 );
	actionDistanceToAxis->setProperty(                            "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_DISTANCE_TO_AXIS                 );
	actionDistanceToSphere->setProperty(                          "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_DISTANCE_TO_SPHERE               );
	actionAngleConeAxis->setProperty(                             "gmMeshFunctionCall", MeshParams::FUNCVAL_SET_ANGLE_USING_AXIS                 );
	actionFuncValScalarMultiply->setProperty(                     "gmMeshFunctionCall", MeshParams::FUNCVAL_MULTIPLY_SCALAR                      );
	actionFuncValSetToOrder->setProperty(                         "gmMeshFunctionCall", MeshParams::FUNCVAL_TO_ORDER                             );
	actionFuncValMeanOneRingRepeat->setProperty(                  "gmMeshFunctionCall", MeshParams::FUNCVAL_MEAN_ONE_RING_REPEAT                 );
	actionFuncValMedianOneRingRepeat->setProperty(                "gmMeshFunctionCall", MeshParams::FUNCVAL_MEDIAN_ONE_RING_REPEAT               );
	actionFuncValAdjacentFaceCount->setProperty(                  "gmMeshFunctionCall", MeshParams::FUNCVAL_ADJACENT_FACES                       );
	actionVisFaceMarchSphereIndex->setProperty(                   "gmMeshFunctionCall", MeshParams::FUNCVAL_DISTANCE_TO_SEED_MARCHING            );
	actionVisVert1RArea->setProperty(                             "gmMeshFunctionCall", MeshParams::FUNCVAL_VERT_ONE_RING_AREA                   );
	actionVisVert1RSumAngles->setProperty(                        "gmMeshFunctionCall", MeshParams::FUNCVAL_VERT_ONE_RING_ANGLE_SUM              );
	actionFuncVertDistancesMax->setProperty(                      "gmMeshFunctionCall", MeshParams::FUNCVAL_VERT_MAX_DISTANCE                    );
	actionVisFaceSortIndex->setProperty(                          "gmMeshFunctionCall", MeshParams::FUNCVAL_FACE_SORT_INDEX                      );
	actionSphereSurfaceLength->setProperty(                       "gmMeshFunctionCall", MeshParams::FUNCVAL_SPHERE_SURFACE_LENGTH                );
	actionSphereVolumeArea->setProperty(                          "gmMeshFunctionCall", MeshParams::FUNCVAL_SPHERE_VOLUME_AREA                   );
	actionSphereSurfaceNumberOfComponents->setProperty(           "gmMeshFunctionCall", MeshParams::FUNCVAL_SPHERE_SURFACE_NUMBER_OF_COMPONENTS  );
	actionAmbientOcclusion->setProperty(                          "gmMeshGLFunctionCall", MeshGLParams::FUNCVAL_AMBIENT_OCCLUSION                );
	// ... Mesh editing ......................................................................................................................................
	actionRemoveFacesSelected->setProperty(                       "gmMeshFunctionCall", MeshParams::EDIT_REMOVE_SELMFACES                        );
	actionRemoveFacesZeroArea->setProperty(                       "gmMeshFunctionCall", MeshParams::EDIT_REMOVE_FACESZERO                        );
	actionEditRemoveFacesBorderErosion->setProperty(              "gmMeshFunctionCall", MeshParams::EDIT_REMOVE_FACES_BORDER_EROSION             );
	actionEditMeshPolish->setProperty(                            "gmMeshFunctionCall", MeshParams::EDIT_AUTOMATIC_POLISHING                     );
	actionEditRemoveSeededSynthComp->setProperty(                 "gmMeshFunctionCall", MeshParams::EDIT_REMOVE_SEEDED_SYNTHETIC_COMPONENTS      );
	actionEditRecomputeVertexNormals->setProperty(                "gmMeshFunctionCall", MeshParams::EDIT_VERTICES_RECOMPUTE_NORMALS              );
	actionEditVerticesAdd->setProperty(                           "gmMeshFunctionCall", MeshParams::EDIT_VERTICES_ADD                            );
	actionSplitByPlane->setProperty(                              "gmMeshFunctionCall", MeshParams::EDIT_SPLIT_BY_PLANE                          );
	actionFacesInvertOrientation->setProperty(                    "gmMeshFunctionCall", MeshParams::EDIT_FACES_INVERT_ORIENTATION                );
	actionApplyMatrix4D->setProperty(                             "gmMeshFunctionCall", MeshParams::APPLY_TRANSMAT_ALL                           );
	actionApplyMatrix4DScale->setProperty(                        "gmMeshFunctionCall", MeshParams::APPLY_TRANSMAT_ALL_SCALE                     );
	actionApplyMatrix4DVertSel->setProperty(                      "gmMeshFunctionCall", MeshParams::APPLY_TRANSMAT_SELMVERT                      );
	actionTransformFunctionValuesToRGB->setProperty(              "gmMeshGLFunctionCall", MeshGLParams::TRANSFORM_FUNCTION_VALUES_TO_RGB         );
	actionMultiplyColorValuesWithFunctionValues->setProperty(     "gmMeshGLFunctionCall", MeshGLParams::MULTIPLY_COLORVALS_WITH_FUNCVALS         );
	actionNormalizeFunctionValues->setProperty(                   "gmMeshGLFunctionCall", MeshGLParams::NORMALIZE_FUNCTION_VALUES                );
	actionSelectPositionsDeselectAll->setProperty(                "gmMeshFunctionCall", MeshParams::SELMPRIMS_POS_DESELECT_ALL                   );
	actionSetMeridianPrime->setProperty(                          "gmMeshFunctionCall", MeshParams::AXIS_ENTER_PRIMEMERIDIAN_ROTATION            );
	actionSetMeridianPrimeSelPrim->setProperty(                   "gmMeshFunctionCall", MeshParams::AXIS_SET_PRIMEMERIDIAN_SELPRIM               );
	actionSetMeridianCutSelPrim->setProperty(                     "gmMeshFunctionCall", MeshParams::AXIS_SET_CUTTINGMERIDIAN_SELPRIM             );
	actionUnrollMeshAroundCone->setProperty(                      "gmMeshFunctionCall", MeshParams::UNROLL_AROUND_CONE                           );
	actionUnrollMeshAroundCylinder->setProperty(                  "gmMeshFunctionCall", MeshParams::UNROLL_AROUNG_CYLINDER                       );
	actionMakeConeCoverMesh->setProperty(                         "gmMeshFunctionCall", MeshParams::CONE_COVER_MESH                              );
	actionExtrudePolylines->setProperty(                          "gmMeshFunctionCall", MeshParams::EXTRUDE_POLYLINES                            );
	// ... Labeling ..........................................................................................................................................
	actionLabelVertices->setProperty(                             "gmMeshFunctionCall", MeshParams::LABELING_LABEL_ALL                           );
	actionLabelSelMVerts->setProperty(                            "gmMeshFunctionCall", MeshParams::LABELING_LABEL_SELMVERTS                     );
	// ... Colorramp and Isoline .............................................................................................................................
	actionSetFixedRangeNormalized->setProperty(                   "gmMeshGLFunctionCall", MeshGLParams::TEXMAP_FIXED_SET_NORMALIZED              );
	actionViewVerticesNone->setProperty(                          "gmMeshGLFunctionCall", MeshGLParams::SET_SHOW_VERTICES_NONE                   );
	actionSetIsolinesByNumber->setProperty(                       "gmMeshGLFunctionCall", MeshGLParams::ISOLINES_SET_BY_NUMBER                   );
	actionIsolinesTenZeroed->setProperty(                         "gmMeshGLFunctionCall", MeshGLParams::ISOLINES_SET_TEN_ZEROED                  );
	// ... Mesh analyze / Polyline ...........................................................................................................................
	actionApplyTpsRpmTransformation->setProperty(                 "gmMeshGLFunctionCall", MeshGLParams::RUN_TPS_RPM_TRANSFORMATION               );
	actionPositionsEuclideanDistances->setProperty(               "gmMeshFunctionCall", MeshParams::SELMPRIMS_POS_DISTANCES                      );
	actionPositionsComputeCircleCenters->setProperty(             "gmMeshFunctionCall", MeshParams::SELMPRIMS_POS_CIRCLE_CENTERS                 );
	actionGeodesicPatchSelPrim->setProperty(                      "gmMeshFunctionCall", MeshParams::GEODESIC_DISTANCE_TO_SELPRIM                 );
	actionPolylinesFromMultipleFuncVals->setProperty(             "gmMeshFunctionCall", MeshParams::POLYLINES_FROM_MULTIPLE_FUNCTION_VALUES      );
	actionPolylinesFromFuncVal->setProperty(                      "gmMeshFunctionCall", MeshParams::POLYLINES_FROM_FUNCTION_VALUE                );
	actionPolylinesFromPlaneIntersect->setProperty(               "gmMeshFunctionCall", MeshParams::POLYLINES_FROM_PLANE_INTERSECTIONS           );
	actionPolylinesFromAxisAndPostions->setProperty(              "gmMeshFunctionCall", MeshParams::POLYLINES_FROM_AXIS_AND_POSTIONS             );
	actionRemovePolylinesSelected->setProperty(                   "gmMeshFunctionCall", MeshParams::POLYLINES_REMOVE_SELECTED                    );
	actionRemovePolylinesAll->setProperty(                        "gmMeshFunctionCall", MeshParams::POLYLINES_REMOVE_ALL                         );
	actionCompAxisFromCircleCenters->setProperty(                 "gmMeshFunctionCall", MeshParams::AXIS_FROM_CIRCLE_CENTERS                     );
	// ... Show Information ..................................................................................................................................
	actionShowInfoMesh->setProperty(                              "gmMeshFunctionCall", MeshParams::SHOW_INFO_MESH                               );
	actionShowInfoSelPrim->setProperty(                           "gmMeshFunctionCall", MeshParams::SHOW_INFO_SELECTION                          );
	actionShowInfoFuncVal->setProperty(                           "gmMeshFunctionCall", MeshParams::SHOW_INFO_FUNCVAL                            );
	actionShowInfoLabelProp->setProperty(                         "gmMeshFunctionCall", MeshParams::SHOW_INFO_LABEL_PROPS                        );
	actionShowInfoAxis->setProperty(                              "gmMeshFunctionCall", MeshParams::SHOW_INFO_AXIS                               );
	// ... Other .............................................................................................................................................
	actionShowLaTeXInfo->setProperty(                             "gmMeshFunctionCall", MeshParams::LATEX_TEMPLATE                               );
        actionMetaDataEditModelId->setProperty(                       "gmMeshFunctionCall", MeshParams::METADATA_EDIT_MODEL_ID                       );
        //! actionMetaDataEditUser->setProperty(                          "gmMeshFunctionCall", MeshParams::METADATA_EDIT_USER                           );
	actionMetaDataEditMaterial->setProperty(                      "gmMeshFunctionCall", MeshParams::METADATA_EDIT_MODEL_MATERIAL                 );
	actionMetaDataEditWebReference->setProperty(                  "gmMeshFunctionCall", MeshParams::METADATA_EDIT_REFERENCE_WEB                  );
	actionEllipsenFit->setProperty(                               "gmMeshFunctionCall", MeshParams::ELLIPSENFIT_EXPERIMENTAL                     );
	actionSelFaceSelfIntersecting->setProperty(                   "gmMeshFunctionCall", MeshParams::DRAW_SELF_INTERSECTIONS                      );
	// =======================================================================================================================================================
	//! \todo MeshWidget Function calls - here is a first try:
	actionExportPlaneIntersectSVG->setProperty(                   "gmMeshWidgetFunctionCall", MeshWidgetParams::EXPORT_POLYLINES_INTERSECT_PLANE );
	actionScreenshot->setProperty(                                "gmMeshWidgetFunctionCall", MeshWidgetParams::SCREENSHOT_CURRENT_VIEW_SINGLE   );
	actionScreenshotPDF->setProperty(                             "gmMeshWidgetFunctionCall", MeshWidgetParams::SCREENSHOT_CURRENT_VIEW_SINGLE_PDF   );
	actionScreenshotViews->setProperty(                           "gmMeshWidgetFunctionCall", MeshWidgetParams::SCREENSHOT_VIEWS_IMAGES          );
	actionScreenshotViewsPDF->setProperty(                        "gmMeshWidgetFunctionCall", MeshWidgetParams::SCREENSHOT_VIEWS_PDF             );
	actionScreenshotViewsPDFDirectory->setProperty(               "gmMeshWidgetFunctionCall", MeshWidgetParams::SCREENSHOT_VIEWS_PDF_DIRECTORY   );
	actionScreenshotViewsPNGDirectory->setProperty(               "gmMeshWidgetFunctionCall", MeshWidgetParams::SCREENSHOT_VIEWS_PNG_DIRECTORY   );
	actionCurrentViewToDefault->setProperty(                      "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_CURRENT_VIEW_TO_DEFAULT      );
	actionSetConeAxisCentralPixel->setProperty(                   "gmMeshWidgetFunctionCall", MeshWidgetParams::EDIT_SET_CONEAXIS_CENTRALPIXEL       );
	actionOrthoSetDPI->setProperty(                               "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_ORTHO_DPI                    );
	actionRenderDefault->setProperty(                             "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_RENDER_DEFAULT               );
	actionRenderMatted->setProperty(                              "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_RENDER_MATTED                );
	actionRenderMetallic->setProperty(                            "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_RENDER_METALLIC              );
	actionRenderLightShading->setProperty(                        "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_RENDER_LIGHT_SHADING         );
	actionRenderFlatAndEdges->setProperty(                        "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_RENDER_FLAT_AND_EDGES        );
	actionBackGroundGridRaster->setProperty(                      "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_GRID_RASTER                  );
	actionBackGroundGridPolar->setProperty(                       "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_GRID_POLAR                   );
	actionBackGroundGridNone->setProperty(                        "gmMeshWidgetFunctionCall", MeshWidgetParams::SET_GRID_NONE                    );

        // --- Github userdata check --------------------------------------------------------------------------------------------------------------

        QObject::connect( actionMetaDataEditUser, &QAction::triggered, this, &QGMMainWindow::saveUser);
        QObject::connect( actionAuthorizeUser, &QAction::triggered, this, &QGMMainWindow::authenticate);
        QObject::connect(this, &QGMMainWindow::authentication, this, &QGMMainWindow::authenticate);
        QObject::connect(this, &QGMMainWindow::authenticated, this, &QGMMainWindow::updateUser);

	mMeshFunctionCalls = new QActionGroup( this );
	for(QAction*& currAction : allActions) {
		    bool propertyPresent = currAction->property( "gmMeshFunctionCall" ).isValid() |
		                       currAction->property( "gmMeshGLFunctionCall" ).isValid() |
		                       currAction->property( "gmMeshWidgetFunctionCall" ).isValid();
		if( !propertyPresent ) {
			continue;
		}
		currAction->setActionGroup( mMeshFunctionCalls );
	}
	mMeshFunctionCalls->setExclusive( false );

	// ALL: Connect:
	QObject::connect( mMeshFunctionCalls, &QActionGroup::triggered, this, &QGMMainWindow::callFunction );
}

//! Add and initalize the mMeshWidget
bool QGMMainWindow::setupMeshWidget( const QGLFormat& rGLFormat ) {
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] Start ..." << endl;
	// Initialize the widget for display of the Mesh using OpenGL - AFTER connecting the menu's actions!
	mMeshWidget = new MeshWidget( rGLFormat, static_cast<QWidget*>( this ) );
	//mMeshWidget->setVisible( false );
	setCentralWidget( mMeshWidget );
	//verticalLayoutMain->addWidget( mMeshWidget );
	mMeshWidget->setFocus();
	mMeshWidget->makeCurrent();
	//mMeshWidget->show();
	//mMeshWidget->qglClearColor( Qt::red );


    if(!((rGLFormat.majorVersion() >= 4) || (rGLFormat.majorVersion() == 4 && rGLFormat.minorVersion() < 3)))
    {
        cout << "[QGMMainWindow::" << __FUNCTION__ << "] OpenGL Version 4.3 not supported: disable atomic-loop and A-Buffer transparency" << endl;
    }

	connect(mMeshWidget, &MeshWidget::loadedMeshIsTextured, mDockSurface, &QGMDockSideBar::enableTextureMeshRendering);

    cout << "[QGMMainWindow::" << __FUNCTION__ << "] ... End" << endl;
	return true;
}

std::ostream& operator<<(std::ostream out, QGMMainWindow::Provider p){

    switch(p){
        case QGMMainWindow::GITHUB: out << "GITHUB"; break;
        case QGMMainWindow::GITLAB: out << "GITLAB"; break;
        case QGMMainWindow::ORCID: out << "ORCID"; break;
        case QGMMainWindow::REDDIT: out << "REDDIT"; break;
        case QGMMainWindow::MATTERMOST: out << "MATTERMOST"; break;
        default: out << int(p); break;
    }

    return out;
}

void QGMMainWindow::authenticate(){

    QSettings settings;
    bool ok;
    cout << "[QGMMainWindow::" << __FUNCTION__ << "] Last user: " << settings.value( "userName" ).toString().toStdString() << endl;

    //QString username = QInputDialog::getText(this, tr("Github Authentication"), tr("Username: "), QLineEdit::Normal, QDir::home().dirName(), &ok);

    Provider provider;

    QStringList list = InputDialog::getStrings(this, &ok);
    if(ok){
        QString username = list.at(0);
        provider = static_cast<Provider>(list.at(1).toInt());
        emit authenticating(&username, &provider);
    }

    settings.setValue( "provider", int(provider));
}

void QGMMainWindow::updateUser(QJsonObject data){
    qDebug() << "[QGMMainWindow] Authentication successful.";

    emit sViewUserInfo(MeshWidgetParams::USER_INFO_STATUS, QString( "authenticated" ));

    QSettings settings;
    if(data.contains("id") && data.contains("name")){
        settings.setValue( "userName", data.value("login").toString());
        settings.setValue( "id", data.value("id").toString());
        settings.setValue( "fullName", data.value("name").toString());
        emit sViewUserInfo(MeshWidgetParams::USER_INFO_USER_NAME, settings.value("userName").toString());
    }
}


void QGMMainWindow::saveUser(){

    // create jsonObj for user data
    QSettings settings;
    QJsonObject newUser;
    newUser.insert("provider", settings.value("provider").toString() );
    newUser.insert("userName", settings.value("userName").toString() );
    newUser.insert("id", settings.value("id").toString() );
    newUser.insert("fullName", settings.value("fullName").toString() );

    // get current time
    time_t _tm = time(NULL );
    struct tm * currTime = localtime ( &_tm );

    // save user data as key:value pair -> User { date : { userdata } }
    QJsonObject userData = QJsonDocument::fromJson(QByteArray::fromStdString(this->mMeshWidget->getMesh()->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_USER_DATA))).object();
    std::cout << "[QGMMainWindow::" << __FUNCTION__ << "] Old Meta Data: " << this->mMeshWidget->getMesh()->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_USER_DATA) << std::endl;
    userData.insert(asctime(currTime), newUser);

    //! \todo error message when no mesh loaded
    // convert to byteArray and update meta data
    QJsonDocument doc(userData);
    QByteArray bytes = doc.toJson();
    this->mMeshWidget->getMesh()->getModelMetaDataRef().setModelMetaString( ModelMetaData::META_USER_DATA, bytes.toStdString() );
    qDebug() << "[QGMMainWindow::" << __FUNCTION__ << "] Updated User Data.";
    qDebug() << "[QGMMainWindow::" << __FUNCTION__ << "] Current user: " << settings.value( "userName" ).toString();

}

MeshWidget* QGMMainWindow::getWidget(){
	return this->mMeshWidget;
}

//! Overloaded from QGMMainWindow
void QGMMainWindow::closeEvent( QCloseEvent* rEvent ) {
	emit unloadMesh();
	rEvent->accept();
}

//! Event handling:
bool QGMMainWindow::event( QEvent* rEvent ) {
	//! When using focus / becoming_inactive, the mouse-mode-extra has to be deactivated:
	if( rEvent->type() == QEvent::WindowDeactivate ) {
		emit sSelectMouseModeExtra(false,MeshWidgetParams::MOUSE_MODE_COUNT);
	}

	return QMainWindow::event(rEvent);
}

//  === SLOTS ==================================================================================================================================================
//
// The main windows slots should be kept as simple as possible.

// --- File Open, Import and Export ----------------------------------------------------------------------------------------------------------------------------

//! Open file-dialog to load 3D-model.
void QGMMainWindow::load() {
	QSettings settings;
	QString fileName = QFileDialog::getOpenFileName( this,
                                                         tr( "Open 3D-Mesh or Point Cloud" ),
	                                                 settings.value( "lastPath" ).toString(),
													 tr( "3D mesh files (*.ply *.PLY *.obj *.OBJ);;Other 3D files (*.txt *.TXT *.xyz *.XYZ)" )
	                                                );
	if( fileName.size() > 0 ) {
		emit sFileOpen( fileName );
	}

    mMeshWidget->setFocus();
}

//! Will emit a signal to load a file with the given rFileName.
void QGMMainWindow::load( const QString& rFileName ) {
	if( rFileName.size() <= 0 ) {
		cerr << "[QGMMainWindow::" << __PRETTY_FUNCTION__ << "] ERROR: No filename given!" << endl;
		return;
	}
	emit sFileOpen( rFileName );
}

//! Will emit a signal to load the last file opened (before).
//! 
//! @returns in case of an error e.g. no last file. True otherwise.
bool QGMMainWindow::loadLast() {
	// Fetch first entry from the .config
	QSettings settings;
	int size = settings.beginReadArray( "recentFiles" );
	if( size <= 0 ) {
		return( false );
	}
	QStringList recentFiles;
	settings.setArrayIndex( 0 );
	recentFiles.append( settings.value("fullPath").toString() );
	settings.endArray();

	QString lastFile = recentFiles.at( 0 );
	// cout << lastFile.toStdString().c_str() << endl;
	emit sFileOpen( lastFile );
	return( true );
}

//! Open a file using the "gmLoadFile" property to open a file.
//! Typically used to open recent files - see QGMMainWindow::updateRecentFileMenu.
bool QGMMainWindow::fileOpen( QAction* rFileAction ){
	if( rFileAction == nullptr ) {
		return false;
	}
	QString fileName = rFileAction->property( "gmLoadFile" ).toString();
	load( fileName );
	return true;
}


//! Handles the dialog for importing function values (per vertex AKA quality).
//! see also QGMMainWindow:: and MeshQt::importFunctionValues
void QGMMainWindow::menuImportFunctionValues() {
	QSettings settings;
	QString fileNames = QFileDialog::getOpenFileName( this,
													  tr( "Import Function Values (per Vertex)" ),
	                                                  settings.value( "lastPath" ).toString(),
													  tr( "ASCII Text (*.mat *.txt)" )
	                                                 );
	if( fileNames.size() > 0 ) {
		emit sFileImportFunctionValues( fileNames );
	}
}

//! Handles the dialog for importing a texture map (color per vertex).
//! see also QGMMainWindow::importTexMap and MeshQt::importTexMapFromFile
void QGMMainWindow::menuImportTexMap() {
	QSettings settings;
	QString fileNames = QFileDialog::getOpenFileName( this,
													  tr( "Import Texture Map (Color per Vertex)" ),
	                                                  settings.value( "lastPath" ).toString(),
													  tr( "Texture maps (*.tex)" )
	                                                 );
	if( fileNames.size() > 0 ) {
		emit sFileImportTexMap( fileNames );
	}
}

//! Handles the dialog for importing feature vectors (to vertices).
//! see also MeshIO::importFeatureVectors and MeshWidget::importFeatureVectorsFile
void QGMMainWindow::menuImportFeatureVectors() {
	QSettings settings;
	QString fileName = QFileDialog::getOpenFileName( this,
													 tr( "Import Feature Vectors (Vertices)" ),
	                                                 settings.value( "lastPath" ).toString(),
													 tr( "Feature vectors (*.mat *.txt)" )
	                                                );
	if( fileName.length() > 0 ) {
		emit sFileImportFeatureVectors( fileName );
	}
}

//! Handles the dialog for importing normal vectors (to vertices).
//! see also MeshIO::importFeatureVectors and MeshWidget::importFeatureVectorsFile
void QGMMainWindow::menuImportNormalVectors() {
	QSettings settings;
	QString fileName = QFileDialog::getOpenFileName( this,
													 tr( "Import Normal Vectors (Vertices)" ),
	                                                 settings.value( "lastPath" ).toString(),
													 tr( "Normal vectors (*.mat *.txt)" )
	                                                );
	if( fileName.length() > 0 ) {
		emit sFileImportNormals( fileName );
	}
}

// --- MENU - MeshWidget ---------------------------------------------------------------------------------------------------------------------------------------

//! Show/Hide stuff within the OpenGL context, handled by the mMeshWidget by emitting sShowFlagMeshWidget.
bool QGMMainWindow::setMeshWidgetFlag( QAction* rAction ) {
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int  flagID;
	bool flagState;
	if( !getFlagIDandState( rAction, "gmMeshWidgetFlag", &flagID, &flagState ) ) {
		if( getFlagIDandState( rAction, "gmMeshWidgetFlagInvert", &flagID, &flagState ) ) {
			flagState = not( flagState );
		} else {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching flag and state failed!" << endl;
			return false;
		}
	}

	if( flagID <= MeshWidget::PARAMS_FLAG_UNDEFINED ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ID out of range (low)!" << endl;
		return false;
	}
	if( flagID >= MeshWidget::PARAMS_FLAG_COUNT ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ID out of range (high)!" << endl;
		return false;
	}

	emit sShowFlagMeshWidget( static_cast<MeshWidget::eParamFlag>(flagID), flagState );
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag: " << flagID << ", " << flagState << " emitted." << endl;
	return true;
}

//! Set view parameters (integer) of the mMeshWidget.
bool QGMMainWindow::setMeshWidgetParamInt( QAction* rAction ) {
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int paramID;
	int paramState;
	if ( !getParamID( rAction, "gmMeshWidgetParamInt", &paramID ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching paramID failed!" << endl;
		return false;
	}
	if ( !getParamID( rAction, "gmMeshWidgetParamValue", &paramState ) ) {
		// If there is no value -- typicall for single, user set values -- send a signal, which has to trigger a user interaction:
		//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag: " << paramID << " (no values) emitted." << endl;
		emit sShowParamIntMeshWidget( static_cast<MeshWidgetParams::eParamInt>(paramID) );
		return true;
	}

	if( paramID <= MeshWidget::PARAMS_INT_UNDEFINED ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ViewParams ID out of range (low)!" << endl;
		return false;
	}
	if( paramID >= MeshWidget::PARAMS_INT_COUNT ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ViewParams ID out of range (high)!" << endl;
		return false;
	}

	//! \todo probably move the following sanity check to the mMeshWidget class!
	if( paramID == MeshWidget::SELECTION_MODE ) {
		if( paramState <= MeshWidgetParams::SELECTION_MODE_NONE ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Selection ID out of range (low," << paramState << ")!" << endl;
			return false;
		}
		if( paramState >= MeshWidgetParams::SELECTION_MODE_COUNT ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Selection ID out of range (high" << paramState << ")!" << endl;
			return false;
		}
	}

	emit sShowParamIntMeshWidget( static_cast<MeshWidgetParams::eParamInt>(paramID), paramState );
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag: " << paramID << ", " << paramState << " emitted." << endl;
	return true;
}

//! Set view parameters (float) of the mMeshWidget.
bool QGMMainWindow::setMeshWidgetParamFloat( QAction* rAction ) {
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int    paramID;
	double paramValueMin;
	double paramValueMax;
	bool   noMinMaxPresent = false;
	if ( !getParamID( rAction, "gmMeshWidgetParamFloat", &paramID ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching paramID failed!" << endl;
		return false;
	}
	if ( !getParamValue( rAction, "gmMeshWidgetParamValueMin", &paramValueMin ) ) {
		//cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << paramID << " no value (min) found." << endl;
		noMinMaxPresent = true;
	}
	if ( !getParamValue( rAction, "gmMeshWidgetParamValueMax", &paramValueMax ) ) {
		//cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << paramID << " no value (max) found." << endl;
		noMinMaxPresent = true;
	}

	// Sanity checks
	if( paramID <= MeshWidget::PARAMS_FLT_UNDEFINED ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Parameter ID out of range (low)!" << endl;
		return false;
	}
	if( paramID >= MeshWidget::PARAMS_FLT_COUNT ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Parameter ID out of range (high)!" << endl;
		return false;
	}

	if( noMinMaxPresent ) {
		emit sShowParamFloatMeshWidget( static_cast<MeshWidgetParams::eParamFlt>(paramID) );
	} else {
		emit sShowParamFloatMeshWidget( static_cast<MeshWidgetParams::eParamFlt>(paramID), paramValueMin, paramValueMax );
	}
	return true;
}

//! sets flags according to the needs of an inspection mode
void QGMMainWindow::activateInspectionOptions() {
	//! \todo source revision using callFunction
	emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_MONO );
	emit sShowParamIntMeshGL( MeshGLParams::TRANSPARENCY_TRANS_FUNCTION, 3 );
	emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_TRANSPARENCY );
	emit sShowParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SOLO,         true );
	emit sShowParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_BORDER,       true );
	emit sShowParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SINGULAR,     true );
	emit sShowParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_NON_MANIFOLD, true );
	emit sShowParamFlagMeshGL( MeshGLParams::SHOW_FACES_CULLED,          false );
}


// --- MENU - MeshGL -------------------------------------------------------------------------------------------------------------------------------------------

//! Show/Hide stuff within the OpenGL context by emitting showFlagMesh.
bool QGMMainWindow::setMeshGLFlag( QAction* rAction ) {
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int  flagID;
	bool flagState;
	if( !getFlagIDandState( rAction, "gmMeshGLFlag", &flagID, &flagState ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching flag and state failed!" << endl;
		return false;
	}
	if( static_cast<MeshGLParams::eParamFlag>(flagID) <= MeshGLParams::SHOW_UNDEFINED ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ID out of range (low)!" << endl;
		return false;
	}
	if( static_cast<MeshGLParams::eParamFlag>(flagID) >= MeshGLParams::PARAMS_FLAG_COUNT ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ID out of range (high)!" << endl;
		return false;
	}

	emit sShowParamFlagMeshGL( static_cast<MeshGLParams::eParamFlag>(flagID), flagState );
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag: " << flagID << ", " << flagState << " emitted." << endl;
	return true;
}

//! Set parameters of type integer for class MeshGL.
bool QGMMainWindow::setMeshGLParamInt( QAction* rAction ) {
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] called." << endl;
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int  paramMeshGLID;
	if( !getParamID( rAction, "gmMeshGLParamInt", &paramMeshGLID ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching ID failed!" << endl;
		return false;
	}
	if( paramMeshGLID <= MeshGLParams::VIEWPARAMS_INT_UNDEFINED ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ID out of range (low)!" << endl;
		return false;
	}
	if( paramMeshGLID >= MeshGLParams::PARAMS_INT_COUNT ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: ID out of range (high)!" << endl;
		return false;
	}
	int paramState;
	if( !getParamID( rAction, "gmMeshGLParamValue", &paramState ) ) {
		// If there is no value -- typicall for single, user set values -- send a signal, which has to trigger a user interaction:
		//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag: " << paramID << " (no values) emitted." << endl;
		emit sShowParamIntMeshGLDialog( static_cast<MeshGLParams::eParamInt>(paramMeshGLID) );
		return true;
	}

	emit sShowParamIntMeshGL( static_cast<MeshGLParams::eParamInt>(paramMeshGLID), paramState );
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag: sShowParamIntMeshGL( " << paramMeshGLID << "," << paramState << " ) emitted." << endl;
	return true;
}

//! Set parameters of type double for class MeshGL AND Mesh.
//! See also: MeshGLParams and MeshParams
bool QGMMainWindow::setMeshGLParamFloat( QAction* rAction ) {
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Triggered." << endl;
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	// Check for Min/Max Values
	double paramValueMin;
	double paramValueMax;
	bool   noMinMaxPresent = false;
	if ( !getParamValue( rAction, "gmParamValueMin", &paramValueMin ) ) {
		//cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << paramID << " no value (min) found." << endl;
		noMinMaxPresent = true;
	}
	if ( !getParamValue( rAction, "gmParamValueMax", &paramValueMax ) ) {
		//cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << paramID << " no value (max) found." << endl;
		noMinMaxPresent |= true;
	}

	// Check for MeshGLParams
	int paramID;
	if ( getParamID( rAction, "gmMeshGLParamFloat", &paramID ) ) {
		// Sanity checks
		if( paramID <= MeshGLParams::PARAMS_FLT_UNDEFINED ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: MeshGL Parameter ID out of range (low)!" << endl;
			return false;
		}
		if( paramID >= MeshGLParams::PARAMS_FLT_COUNT ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: MeshGL Parameter ID out of range (high)!" << endl;
			return false;
		}
		// cout << "[QGMMainWindow::" << __FUNCTION__ << "] MeshGL Parameter ID: " << paramID << endl;
		// Start user interaction
		if( noMinMaxPresent ) {
			emit sShowParamFloatMeshGLDialog( static_cast<MeshGLParams::eParamFlt>(paramID) );
		} else {
			emit sShowParamFloatMeshGLLimits( static_cast<MeshGLParams::eParamFlt>(paramID), paramValueMin, paramValueMax );
		}
		return true;
	}

	// Check for MeshParam
	if ( getParamID( rAction, "gmMeshParamFloat", &paramID ) ) {
		// Sanity checks
		if( paramID <= MeshParams::PARAMS_FLT_UNDEFINED ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Mesh Parameter ID out of range (low)!" << endl;
			return false;
		}
		if( paramID >= MeshParams::PARAMS_FLT_COUNT ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Mesh Parameter ID out of range (high)!" << endl;
			return false;
		}
		// cout << "[QGMMainWindow::" << __FUNCTION__ << "] Mesh Parameter ID: " << paramID << endl;
		// Start user interaction
		if( noMinMaxPresent ) {
			emit sShowParamFloatMeshDialog( static_cast<MeshParams::eParamFlt>(paramID) );
		} else {
			emit sShowParamFloatMeshLimits( static_cast<MeshParams::eParamFlt>(paramID), paramValueMin, paramValueMax );
		}
		return true;
	}

	cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching paramID failed!" << endl;
	return true;
}

//! Set parameters of type color for class MeshGL.
bool QGMMainWindow::setMeshGLColor( QAction* rAction ) {
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int paramID;
	if ( !getParamID( rAction, "gmMeshGLColor", &paramID ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching paramID failed!" << endl;
		return false;
	}

	// Sanity checks
	//if( paramID <= MeshGL::COLOR_UNDEFINED ) {
	//	cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Parameter ID out of range (low)!" << endl;
	//	return false;
	//}
	if( paramID >= MeshGLColors::COLOR_SETTING_COUNT ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Parameter ID out of range (high)!" << endl;
		return false;
	}

	emit sShowParamColorMeshGL( static_cast<MeshGLColors::eColorSettings>(paramID) );
	return true;
}

//! Checks for properties with eFunctionCall IDs of MeshParams and MeshGLParams.
//! Emits a signal with the ID.
//!
//! @returns false in case of an error e.g. no property present. True otherwise.
bool QGMMainWindow::callFunction( QAction* rAction ) {

	// Sanity check
	if( rAction == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	int callID;
	bool actionState = rAction->isChecked(); // Additional flag for buttons, which can be toggled
	if ( getParamID( rAction, "gmMeshFunctionCall", &callID ) ) {
		emit sCallFunctionMesh( static_cast<MeshParams::eFunctionCall>(callID), actionState );
		return true;
	}
	if ( getParamID( rAction, "gmMeshGLFunctionCall", &callID ) ) {
		emit sCallFunctionMeshGL( static_cast<MeshGLParams::eFunctionCall>(callID), actionState );
		return true;
	}
	if ( getParamID( rAction, "gmMeshWidgetFunctionCall", &callID ) ) {
		emit sCallFunctionMeshWidget( static_cast<MeshWidgetParams::eFunctionCall>(callID), actionState );
		return true;
	}

	cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: fetching callID failed!" << endl;
	return false;
}


// --- ---------------------------------------------------------------------------------------------------------------------------------------------------------


//! Strips the ID and the state by name from a property of an action.
bool QGMMainWindow::getFlagIDandState( QAction* rAction, const char* rName, int* rID, bool* rState ) {

	if( ( rAction == nullptr ) || ( rID == nullptr ) || ( rState == nullptr ) || ( rName == nullptr ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	QVariant someFlag = rAction->property( rName );
	if( !someFlag.isValid() ) {
		// No critical error - just:
		return false;
	}

	bool convertOk;
	int  someID = someFlag.toInt( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Can not convert property!" << endl;
		return false;
	}

	(*rID)    = someID;
	(*rState) = rAction->isChecked();
	return true;
}

//! Retrieve the value of an integer parameter using a given name from an QAction i.e. menu entry.
bool QGMMainWindow::getParamIDValueState( QAction*    rAction,    //!< Element -- typically an menu entry.
                                          const char* rName,      //!< Name of the property.
                                          const char* rValueName, //!< Name of the property holding the integer value.
                                          int*        rID,        //!< ID od the property -- typically from an enumerator.
                                          int*        rValue,     //!< Integer value stored for this property.
                                          bool*       rState      //!< Checked state of the element rAction.
                                         ) {
	// cout << "[QGMMainWindow::" << __FUNCTION__ << "] START: param name '" << rName << "' paramvalue name '" << rValueName << "'" << endl;

	// Sanity check:
	if( ( rAction == nullptr ) || ( rID == nullptr ) || ( rState == nullptr ) || ( rName == nullptr ) || ( rValueName == nullptr ) || ( rValue == nullptr ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	QVariant someParam = rAction->property( rName );
	if( !someParam.isValid() ) {
		// No critical error - just:
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << rAction->objectName().toStdString() << " param name '" << rName << "' not valid!" << endl;
		return false;
	}
	QVariant someParamValue = rAction->property( rValueName );
	if( !someParamValue.isValid() ) {
		// Ignore:
		if( rAction == actionVideoFrameSizeSet ) {
			return false;
		}
		// No critical error - just:
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << rAction->objectName().toStdString() << " paramvalue name '" << rValueName << "' not valid!" << endl;
		rAction->dumpObjectInfo();
		return false;
	}

	bool convertOk;
	int  someParamID = someParam.toInt( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << rAction->objectName().toStdString() << " Can not convert property (1)!" << endl;
		return false;
	}
	int  someParamValueID = someParamValue.toInt( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: " << rAction->objectName().toStdString() << " Can not convert property (2)!" << endl;
		return false;
	}

	(*rID)    = someParamID;
	(*rValue) = someParamValueID;
	(*rState) = rAction->isChecked();
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] DONE: " << rAction->objectName().toStdString() << " param name '" << rName << "' paramvalue name '" << rValueName << "'" << endl;
	return true;
}

//! Strips the ID and the state by name from a property of an action.
bool QGMMainWindow::getParamID( QAction* rAction, const char* rName, int* rID ) {
	if( ( rAction == nullptr ) || ( rID == nullptr ) || ( rName == nullptr ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	QVariant someFlag = rAction->property( rName );
	if( !someFlag.isValid() ) {
		// Do not show an error as it can be legit to have a parameter missing. e.g. this typically triggers an user interaction.
		return false;
	}
	bool convertOk;
	int  someID = someFlag.toInt( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Can not convert property!" << endl;
		return false;
	}

	(*rID) = someID;
	return true;
}

//! Strips the value from a property of an action.
bool QGMMainWindow::getParamValue( QAction* rAction, const char* rName, double* rValue ) {
	if( ( rAction == nullptr ) || ( rName == nullptr ) || ( rValue == nullptr ) ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	QVariant someFlag = rAction->property( rName );
	if( !someFlag.isValid() ) {
		// Do not show an error as it can be legit to have a parameter missing. e.g. this typically triggers an user interaction.
		return false;
	}
	bool  convertOk;
	double someValue = someFlag.toDouble( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Can not convert property!" << endl;
		return false;
	}

	(*rValue) = someValue;
	return true;
}

// --- Select ------------------------------------------------------------------------------------------

//! Set the MainWindow to a fixed size, so that the mMeshWidget has the request size.
//! Used for rendering videos/flasg.
void QGMMainWindow::setWidgetSizeFixed( bool rFixed ) {
	// Sanity check
	if( mMeshWidget == nullptr ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: mMeshWidget is NULL!" << endl;
		return;
	}
	if( rFixed ) {
		QSize sizemMeshWidget = mMeshWidget->size();
		QSize sizeMainWin    = size();
		int sizeWidth;
		int sizeHeight;
		mMeshWidget->getParamIntegerMeshWidget( MeshWidgetParams::VIDEO_FRAME_WIDTH, &sizeWidth );
		mMeshWidget->getParamIntegerMeshWidget( MeshWidgetParams::VIDEO_FRAME_HEIGHT, &sizeHeight );
		//cout << "mMeshWidget " << sizemMeshWidget.width() << " x " << sizemMeshWidget.height() << endl;
		//cout << "mainWin   " << sizeMainWin.width() << " x " << sizeMainWin.height() << endl;
		sizeMainWin.setWidth(   sizeMainWin.width()-sizemMeshWidget.width()+sizeWidth );
		sizeMainWin.setHeight( sizeMainWin.height()-sizemMeshWidget.height()+sizeHeight );
		setFixedSize( sizeMainWin );
		//sizemMeshWidget = mMeshWidget->size();
		//sizeMainWin    = size();
		//cout << "mMeshWidget " << sizemMeshWidget.width() << " x " << sizemMeshWidget.height() << endl;
		//cout << "mainWin   " << sizeMainWin.width() << " x " << sizeMainWin.height() << endl;
	} else {
		setMinimumSize( 400, 300 );
		setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
	}
}

// --- ? ----------------------------------------------------------------------------------

//! Show the keyboard shortcuts defined within the menus.
//! Excludes mouse related controls and all shortcuts defined not in the main menus!
void QGMMainWindow::infoKeyShortcuts() {
	QString infoString;
	QList<QAction*> allActions = findChildren<QAction*>();
	infoString = tr("See also keyboard layout for 3D navigation.") + "<br />";
	infoString += "<table>";
	infoString += "<tr><td><b>" + tr("Key(s)") + "</b></td><td>&nbsp;</td><td><b>" + tr("Action") + "</b></td></tr>";
	infoString += "<tr><td colspan='3'><hr/></td></tr>";
	for(QAction*& currAction : allActions) {
		    QKeySequence shortCutSeq = currAction->shortcut();
		if( shortCutSeq.count() == 0 ) {
			continue;
		}
		QString actionText = currAction->text();
		QString actionShortCutText = (currAction->shortcut()).toString();
		infoString.append( "<tr><td align=right>" + actionShortCutText.replace( " ", "&nbsp;" )  + "</td><td>&nbsp;</td>" \
		                   "<td align=left>" + actionText.replace( "&", "" ).replace( " ", "&nbsp;" ) + "</td></tr>" );
	}
	infoString += "</table>";
	SHOW_MSGBOX_INFO( tr("Keyboard Shortcuts (Menu only)"), QString( "%1" ).arg( infoString ) );
}

//! Open the GigaMesh Video Tutorials within the browser.
void QGMMainWindow::visitVideoTutorials() {
	QDesktopServices::openUrl( QUrl("https://gigamesh.eu/youtube") );
}

//! Open the GigaMesh web site within the browser.
void QGMMainWindow::visitWebSite() {
	QDesktopServices::openUrl( QUrl("https://gigamesh.eu") );
}

//! Show ''about'' dialog.
void QGMMainWindow::aboutBox() {
	QGMDialogAbout dlgAbout;
	dlgAbout.exec();
}

// --- DYNAMIC MENU -----------------------------------------------------------------------

//! Sets menu items according to the flags of MeshGL::showFlagMeshsArr
//! \todo Transition of hard-coded version (switch) to new, automated checkbox handling (for).
void QGMMainWindow::updateMeshShowFlag( MeshGLParams::eParamFlag rShowFlagNr, bool rSetState ) {
	QList<QAction*> meshActions = mMeshGLFlag->actions();
	for( int i=0; i<meshActions.size(); ++i ) {
		QAction* currAction = meshActions.at( i );
		int  flagMeshID;
		bool flagMeshState;
		if( !getFlagIDandState( currAction, "gmMeshGLFlag", &flagMeshID, &flagMeshState ) ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: getFlagIDandState failed for action No. " << i << "!" << endl;
			continue;
		}
		if( rShowFlagNr == static_cast<MeshGLParams::eParamFlag>(flagMeshID) ) {
			currAction->setChecked( rSetState );
			//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag set for action No. " << i << "!" << endl;
		}
	}
	if( ( rShowFlagNr <  MeshGLParams::SHOW_UNDEFINED    ) || \
	    ( rShowFlagNr >= MeshGLParams::PARAMS_FLAG_COUNT ) ) {
		//! Ignore FlagIDs outside the MeshGLParams::SHOW_UNDEFINED to MeshGLParams::PARAMS_FLAG_COUNT range.
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: unknown or undefined show flag " << rShowFlagNr << "!" << endl;
	}
}

//! Sets menu items according to the flags of MeshGLParams::mParamInt
void QGMMainWindow::updateMeshParamInt( MeshGLParams::eParamInt rParam, int rValue ) {
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Integer parameter no: " << rParam << " Value: " << rValue << endl;
	QList<QAction*> allActions = findChildren<QAction*>();
	for(QAction*& currAction : allActions) {
		    QVariant someFlag = currAction->property( "gmMeshGLParamInt" );
		if( !someFlag.isValid() ) {
			continue;
		}
		// Check for presence of a value
		// If there is none, than we have a simple value, typically entered by the user ...
		someFlag = currAction->property( "gmMeshGLParamValue" );
		if( !someFlag.isValid() ) {
			continue;
		}
		// ... otherwise the action is part of an exclusive group.
		int  paramID;
		int  paramValue;
		bool paramState;
		if( !getParamIDValueState( currAction, "gmMeshGLParamInt", "gmMeshGLParamValue", &paramID, &paramValue, &paramState ) ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: getParamIDValueState failed for " << currAction->objectName().toStdString() << "'." << endl;
			continue;
		}
		if( ( paramID == rParam ) && ( paramValue == rValue ) ) {
			if( currAction->isCheckable() ) {
				currAction->setChecked( true );
				//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Check mark set for '" << currAction->objectName().toStdString() << "'." << endl;
			}
		}
	}
}

//! Sets menu items according to the flags of MeshWidgetParams::eParamFlag
void QGMMainWindow::updateWidgetShowFlag(MeshWidgetParams::eParamFlag rFlag, bool rState ) {
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag no: " << showFlagWidgetNr << " State: " << setState << endl;

	bool flagAssigned = false;
	QList<QAction*> meshActions = mMeshWidgetFlag->actions();
	for( int i=0; i<meshActions.size(); ++i ) {
		QAction* currAction = meshActions.at( i );
		int  flagMeshID;
		bool flagMeshState;
		if( !getFlagIDandState( currAction, "gmMeshWidgetFlag", &flagMeshID, &flagMeshState ) ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: getFlagIDandState failed for action No. " << i << "!" << endl;
			continue;
		}
		if( flagMeshID == rFlag ) {
			currAction->setChecked( rState );
			//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Flag set for action No. " << i << "!" << endl;
			flagAssigned = true;
		}
	}

	switch(rFlag)
	{
	case MeshWidgetParams::PARAMS_FLAG_UNDEFINED:
		break;
	case MeshWidgetParams::ORTHO_MODE:
		// Attention this has to be treated extra, because of the "option style",
		// which removes the actionProjectOrthographic from the mMeshWidgetFlag group!
		actionProjectOrthographic->setChecked( rState );
		actionProjectPerspective->setChecked( !rState );
		break;
	case MeshWidgetParams::SHOW_GRID_RECTANGULAR:
		break;
	case MeshWidgetParams::SHOW_GRID_HIGHLIGHTCENTER:
		break;
	case MeshWidgetParams::SHOW_GRID_POLAR_LINES:
		break;
	case MeshWidgetParams::SHOW_GRID_POLAR_CIRCLES:
		break;
	case MeshWidgetParams::SHOW_GRID_HIGHLIGHTCENTER_FRONT:
		break;
	case MeshWidgetParams::SHOW_HISTOGRAM:
		break;
	case MeshWidgetParams::SHOW_HISTOGRAM_LOG:
		break;
	case MeshWidgetParams::SHOW_HISTOGRAM_SCENE:
		break;
	case MeshWidgetParams::SHOW_HISTOGRAM_SCENE_LOG:
		break;
	case MeshWidgetParams::SHOW_GIGAMESH_LOGO_FORCED:
		break;
	case MeshWidgetParams::SHOW_GIGAMESH_LOGO_CANVAS:
		break;
	case MeshWidgetParams::SHOW_KEYBOARD_CAMERA:
		break;
	case MeshWidgetParams::SHOW_FOG:
		break;
	case MeshWidgetParams::SPHERICAL_VERTICAL:
		// Attention this has to be treated extra, because of the "option style",
		// which removes the actionProjectOrthographic from the mMeshWidgetFlag group!
		actionSphericalImagesVertical->setChecked( rState );
		actionSphericalImagesHorizontal->setChecked( !rState );
		break;
	case MeshWidgetParams::LIGHT_ENABLED:
		break;
	case MeshWidgetParams::LIGHT_FIXED_WORLD:
		break;
	case MeshWidgetParams::LIGHT_FIXED_CAM:
		break;
	case MeshWidgetParams::LIGHT_AMBIENT:
		break;
	case MeshWidgetParams::CROP_SCREENSHOTS:
		break;
	case MeshWidgetParams::VIDEO_FRAME_FIXED:
		setWidgetSizeFixed( rState );
		break;
	case MeshWidgetParams::EXPORT_SVG_AXIS_DASHED:
		break;
	case MeshWidgetParams::EXPORT_SIDE_VIEWS_SIX:
		break;
	case MeshWidgetParams::SCREENSHOT_FILENAME_WITH_DPI:
		break;
	case MeshWidgetParams::SHOW_MESH_REDUCED:
		break;
	case MeshWidgetParams::ENABLE_SHOW_MESH_REDUCED:
		break;
	case MeshWidgetParams::PARAMS_FLAG_COUNT:
		break;
	default:
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: unsupported/unimplemented flag no: " << rFlag << "!" << endl;
		break;
	}
}

//! Sets menu items according to the flags of MeshWidgetParams::eParamInt
void QGMMainWindow::updateWidgetShowInteger( MeshWidgetParams::eParamInt rParam, int rValue ) {
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Integer parameter no: " << rParam << " Value: " << rValue << endl;
	QList<QAction*> allActions = findChildren<QAction*>();
	for(QAction*& currAction : allActions) {
		    QVariant someParamInteger = currAction->property( "gmMeshWidgetParamInt" );
		if( !someParamInteger.isValid() ) {
			continue;
		}
		// Check for presence of a value
		// If there is none, than we have a simple value, typically entered by the user ...
		QVariant someValueInteger = currAction->property( "gmMeshWidgetParamValue" );
		if( !someValueInteger.isValid() ) {
			continue;
		}
		// ... otherwise the action is part of an exclusive group.
		int  paramID;
		int  paramValue;
		bool paramState;
		if( !getParamIDValueState( currAction, "gmMeshWidgetParamInt", "gmMeshWidgetParamValue", &paramID, &paramValue, &paramState ) ) {
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: getParamIDValueState failed for " << currAction->objectName().toStdString() << "'." << endl;
			continue;
		}
		if( ( paramID == rParam ) && ( paramValue == rValue ) ) {
			if( currAction->isCheckable() ) {
				currAction->setChecked( true );
				//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Check mark set for '" << currAction->objectName().toStdString() << "'." << endl;
			}
		}
	}
}

//! Sets menu items according to the flags of Primitive::primitiveTypes
void QGMMainWindow::updateMeshElementFlag( int elementFlagNr, bool setState ) {
	switch( elementFlagNr ) {
		case Primitive::IS_POLYLINE:
			actionExportPolylines->setEnabled( setState );
			actionExportPolylinesProjected->setEnabled( setState );
			actionRemovePolylinesSelected->setEnabled( setState );
			actionRemovePolylinesAll->setEnabled( setState );
			actionNormalsPolyline->setEnabled( setState );
			actionNormalsPolylineMain->setEnabled( setState );
			actionViewPolylines->setEnabled( setState );
			actionViewPolylinesCurv->setEnabled( setState );
			actionViewPolylinesCurvAbs->setEnabled( setState );
			actionViewPolylinesCurvScale->setEnabled( setState );
			actionAdvancePolyThres->setEnabled( setState );
			actionPolylineExtrema->setEnabled( setState );
			break;
		default:
			cerr << "[QGMMainWindow::" << __FUNCTION__ << "] unsupported/unimplemented flag no: " << elementFlagNr << "!" << endl;
	}
}

// --- VARIOUS ----------------------------------------------------------------------------

//! Updates the main windows title bar and
//! the recently used file menu
void QGMMainWindow::fileChanged(
                const QString& rFileNameFull,
                const QString& rFileNameBase
) {
	setWindowTitle( QString( "GigaMesh [" ) + rFileNameBase + QString( "]" ) );
	// Qt uses "/" as a universal directory separator in the same way that "/" is used as a path separator in URLs.
	// If you always use "/" as a directory separator, Qt will translate your paths to conform to the underlying operating system.
	// Source: http://doc.qt.nokia.com/latest/qdir.html
	QSettings settings;
	int size = settings.beginReadArray( "recentFiles" );
	QStringList recentFiles;
	for( int i=0; i<size; ++i ) {
		settings.setArrayIndex( i );
		recentFiles.append( settings.value("fullPath").toString() );
	}
	settings.endArray();
	QString fullName = rFileNameFull;
	fullName.replace( "//", "/" );
	QRegExp fullNameExact( fullName, Qt::CaseSensitive, QRegExp::FixedString );
	int isInListAt;
	do {
		isInListAt = recentFiles.indexOf( fullNameExact );
		if( isInListAt >= 0 ) {
			recentFiles.removeAt( isInListAt );
		}
	} while( isInListAt >= 0 );
	recentFiles.prepend( fullName );
	if( recentFiles.size() > 10 ) {
		recentFiles.removeLast();
	}
	settings.beginWriteArray( "recentFiles" );
	for( int i=0; i<recentFiles.size(); ++i ) {
		settings.setArrayIndex( i );
		settings.setValue( "fullPath", recentFiles.at(i) );
	}
	settings.endArray();
	updateRecentFileMenu();
}

//! Update the recent file menu entries.
void QGMMainWindow::updateRecentFileMenu() {
	//! 1. Remove entries from the menu and the action group
	QList<QAction*> menuFileRecentActions = menuFileRecent->actions();
	for(QAction*& menuFileRecentAction : menuFileRecentActions) {
		menuFileRecent->removeAction( menuFileRecentAction );
	}
	if( mRecentFiles != nullptr ) {
		mRecentFiles->disconnect();
		delete mRecentFiles;
		mRecentFiles = nullptr;
	}
	//! 2. Fetch entries from the .config
	QSettings settings;
	int size = settings.beginReadArray( "recentFiles" );
	QStringList recentFiles;
	for( int i=0; i<size; ++i ) {
		settings.setArrayIndex( i );
		recentFiles.append( settings.value("fullPath").toString() );
	}
	settings.endArray();
	//! 3. Add entries to the menu and the action group
	mRecentFiles = new QActionGroup( this );
	for( int i=0; i<recentFiles.size(); i++ ) {
		QAction* someAction = new QAction( this );
		QString someFileBase = recentFiles.at(i).section( "/", -1 );
		if( i==0 ) {
			someFileBase = someFileBase + " (Alt+&F)";
		}
		someAction->setText( someFileBase );
		someAction->setProperty( "gmLoadFile", recentFiles.at(i) );
		mRecentFiles->addAction( someAction );
		menuFileRecent->addAction( someAction );
	}
	QObject::connect( mRecentFiles, SIGNAL(triggered(QAction*)), this, SLOT(fileOpen(QAction*)) );
}

void QGMMainWindow::setMenuContextToSelection( Primitive* primitive ) {
	//! Turns on/off menus related to feature vectors and selections handled by menuContextToSelection.
	if( primitive != nullptr ) {
		menuContextToSelection->setEnabled( true );
	} else {
		menuContextToSelection->setEnabled( false );
	}
}

//! Slot taking care about successfull http-request to fetch the
//! latest version number of GigaMesh from the WebSite.
void QGMMainWindow::slotHttpCheckVersion( QNetworkReply* rReply ) {
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] Current version is   " << QString( "%1" ).arg( VERSION_PACKAGE ).toStdString() << endl;
	if( rReply->error() != QNetworkReply::NoError ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Code " << rReply->error() << ": " << rReply->errorString().toStdString() << endl;
		return;
	}
	QByteArray responseBytes = rReply->readAll();
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] Available Version is " << responseBytes.constData() << endl;

	bool convOk = false;
	unsigned int versionOnline = responseBytes.toUInt( &convOk );
	if( !convOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Version (online) is not an unsigned integer!" << endl;
		return;
	}
	
	convOk = false;
	
	unsigned int versionCurrent = QString( "%1" ).arg( VERSION_PACKAGE ).toUInt( &convOk );
	if( !convOk ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR: Version (current) is not an unsigned integer!" << endl;
		return;
	}

	// versionCurrent = 170101; // for testing (2/2)

	if( versionOnline == versionCurrent ) {
		cout << "[QGMMainWindow::" << __FUNCTION__ << "] You are using the latest offical version." << endl;
	} else if ( versionOnline < versionCurrent ) {
		cout << "[QGMMainWindow::" << __FUNCTION__ << "] You are using a NEWER version than the offical version." << endl;
	} else {
		cout << "[QGMMainWindow::" << __FUNCTION__ << "] There is a newer version of GigaMesh available for" << endl;
		cout << "[QGMMainWindow::" << __FUNCTION__ << "] download at: https://gigamesh.eu/download" << endl;
		QString msgStr = tr( "There is a newer version (%1) of GigaMesh available for download at: <br /><br />"
		                          "<a href='https://gigamesh.eu/download'>https://gigamesh.eu/download</a> <br /><br />"
		                          "The version you are using is&nbsp;%2.<br /><br />"
		                          "See the CHANGELOG file within the new package for updates. "
		                          "Additional info is typically provided "
		                          "in our <a href='https://gigamesh.eu/news'>WebSite's news section</a> and "
		                          "in the <a href='https://gigamesh.eu/researchgate'>ResearchGate project log</a>."
		                        ).arg( versionOnline ).arg( versionCurrent );
		SHOW_MSGBOX_WARN( tr("NEW Version available"), msgStr.toStdString().c_str() );
	}

	// Store the current timestamp for the last successful attempt
	time_t timeNow;
	time( &timeNow );
	QSettings settings;
	settings.setValue( "lastVersionCheck", qlonglong( timeNow ) );
}

void QGMMainWindow::slotChangeLanguage(QAction* action)
{
	if(action != nullptr)
	{
		loadLanguage(action->data().toString());
		QSettings settings;
		settings.setValue("language", action->data().toString());
	}
}

void switchTranlator(QTranslator& translator, const QString& fileName, const QString& directory)
{
	qApp->removeTranslator(&translator);

	if(translator.load(fileName, directory))
	{
		qApp->installTranslator(&translator);
	}
}

void QGMMainWindow::loadLanguage(const QString& language)
{
	if(mCurrentLanguage != language)
	{
		mCurrentLanguage = language;

		switchTranlator(mTranslator, QString("GigaMesh_%1").arg(language), ":/languages");

	}
}

void QGMMainWindow::createLanguageMenu()
{
	auto langGroup = new QActionGroup(menuLanguages);
	langGroup->setExclusive(true);

	connect(langGroup, &QActionGroup::triggered, this, &QGMMainWindow::slotChangeLanguage);

	QSettings settings;

	QString defaultLocale = settings.value("language", QString("")).toString();

	if(defaultLocale.isEmpty())
	{
		defaultLocale = QLocale::system().name();
		defaultLocale.truncate(defaultLocale.lastIndexOf('_'));
	}

	QDir dir(QString(":/languages"));
	QStringList fileNames = dir.entryList(QStringList("GigaMesh_*.qm"));

	bool localeSet = false;

	for(auto locale : fileNames)
	{
		locale.truncate(locale.lastIndexOf('.'));
		locale.remove(0, locale.indexOf('_') + 1);

		QString lang = QLocale::languageToString(QLocale(locale).language());

		auto action = new QAction(lang, this);
		action->setCheckable(true);
		action->setData(locale);

		menuLanguages->addAction(action);
		langGroup->addAction(action);

		if(defaultLocale == locale)
		{
			localeSet = true;
			action->setChecked(true);
			//call slot manually, as the menu is created in QGMMainWindow's constructor. remove if it is done elsewhere in the future,
			//because then it is handled via signal/slots by setChecked
			slotChangeLanguage(action);
		}
	}
}

void QGMMainWindow::openExternalProgramsDialog()
{
	if(mMeshWidget == nullptr)
		return;

	ExternalProgramsDialog dialog;
	std::string temp;
	mMeshWidget->getParamStringMeshWidget(MeshWidgetParams::INKSCAPE_COMMAND, &temp);
	dialog.setInkscapePath(QString::fromStdString(temp));

	mMeshWidget->getParamStringMeshWidget(MeshWidgetParams::PDF_LATEX_COMMAND, &temp);
	dialog.setPdfLatexPath(QString::fromStdString(temp));

	mMeshWidget->getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND, &temp);
	dialog.setPdfViewerPath(QString::fromStdString(temp));

	mMeshWidget->getParamStringMeshWidget(MeshWidgetParams::PYTHON3_COMMAND, &temp);
	dialog.setPythonPath(QString::fromStdString(temp));

	if(dialog.exec() == QDialog::Accepted)
	{
		mMeshWidget->setParamStringMeshWidget(MeshWidgetParams::INKSCAPE_COMMAND  , dialog.inkscapePath().toStdString());
		mMeshWidget->setParamStringMeshWidget(MeshWidgetParams::PDF_LATEX_COMMAND , dialog.pdfLatexPath().toStdString());
		mMeshWidget->setParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND, dialog.pdfViewerPath().toStdString());
		mMeshWidget->setParamStringMeshWidget(MeshWidgetParams::PYTHON3_COMMAND   , dialog.pythonPath().toStdString());

		QSettings settings;
		settings.setValue("Inkscape_Path" , QString(dialog.inkscapePath()));
		settings.setValue("PdfLatex_Path" , QString(dialog.pdfLatexPath()));
		settings.setValue("PdfViewer_Path", QString(dialog.pdfViewerPath()));
		settings.setValue("Python3_Path"  , QString(dialog.pythonPath()));
	}
}

void QGMMainWindow::openGridPositionDialog()
{
	if(mMeshWidget == nullptr)
		return;

	dialogGridCenterSelect dialog;

	int centerSelect = 0;
	mMeshWidget->getParamIntegerMeshWidget(MeshWidgetParams::GRID_CENTER_POSITION, &centerSelect);
	dialog.setCenterPos(static_cast<dialogGridCenterSelect::CenterPos>(centerSelect));
	if(dialog.exec() == QDialog::Accepted)
	{
		mMeshWidget->setParamIntegerMeshWidget(MeshWidgetParams::GRID_CENTER_POSITION, static_cast<int>(dialog.centerPos()));
	}
}

//! Add extra key shortcuts for fullscreen and mouse mode:
void QGMMainWindow::keyPressEvent( QKeyEvent *rEvent ) {
	// The "return" statements ensure "...do not call the base class implementation if you act upon the key."
	// See: qt-project.org/doc/qwidget.html#keyPressEvent
	// At the end of this method <parent>::keyPressEvent has to be called, otherwise the key-handling becomes f**ked up.
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Key: " << rEvent->key() << endl;

	//! Key 0 to turn on/off light.
	if( rEvent->key() == Qt::Key_0 ) {
		emit sShowFlagMeshWidget( MeshWidget::LIGHT_ENABLED, !actionLightning->isChecked() );
		return;
	}
	if( ( rEvent->key() == Qt::Key_Escape ) && // Qt::Key_F11 not used as it is mapped to a menu entry.
	    ( isFullScreen() ) ) {                  //Abort Fullscreen mode
		toggleFullscreen();
		return;
	}
	//! Switch to the default mouse mode(s) using the space bar.
	if( rEvent->key() == Qt::Key_Space ) {
		emit sSelectMouseModeDefault();
	}
	if( rEvent->key() == Qt::Key_Control ) {
		emit sSelectMouseModeExtra(true,MeshWidgetParams::MOUSE_MODE_SELECT);
	}
	if( rEvent->key() == Qt::Key_Shift ) {
		emit sSelectMouseModeExtra(true,MeshWidgetParams::MOUSE_MODE_MOVE_PLANE);
	}
	if( rEvent->key() == Qt::Key_Alt ) {
		emit sSelectMouseModeExtra(true,MeshWidgetParams::MOUSE_MODE_MOVE_LIGHT_FIXED_CAM);
	}
	if( rEvent->key() == Qt::Key_AltGr ) {
		emit sSelectMouseModeExtra(true,MeshWidgetParams::MOUSE_MODE_MOVE_LIGHT_FIXED_WORLD);
	}
	//cout << "[QGMMainWindow::" << __FUNCTION__ << "] Key: " << rEvent->key() << " ignored." << endl;
	QMainWindow::keyPressEvent( rEvent );
}

//! Add extra key shortcuts for fullscreen and mouse mode:
void QGMMainWindow::keyReleaseEvent( QKeyEvent *rEvent ) {
	if( rEvent->key() == Qt::Key_Control ) {
		emit sSelectMouseModeExtra(false,MeshWidgetParams::MOUSE_MODE_COUNT);
	}
	if( rEvent->key() == Qt::Key_Shift ) {
		emit sSelectMouseModeExtra(false,MeshWidgetParams::MOUSE_MODE_COUNT);
	}
	//if( rEvent->key() == Qt::Key_Alt ) {
	//	emit sSelectMouseModeExtra(false,MeshWidgetParams::MOUSE_MODE_COUNT);
	//}
	if( rEvent->key() == Qt::Key_AltGr ) {
		emit sSelectMouseModeExtra(false,MeshWidgetParams::MOUSE_MODE_COUNT);
	}
	QMainWindow::keyReleaseEvent( rEvent );
}

//! Toogle the fullscreen mode.
void QGMMainWindow::toggleFullscreen() {
	if( isFullScreen() ) {
		menuWidget()->setVisible( actionMenu->isChecked() );
		statusBar()->setVisible( actionStatusbar->isChecked() );
		uiMainToolBar->setVisible( actionToolbar->isChecked() );
		mDockInfo->setVisible( true );
		mDockSurface->setVisible( true );
		mDockView->setVisible( true );
		this->showNormal();
	} else {
		menuWidget()->hide();
		statusBar()->hide();
		uiMainToolBar->hide();
		mDockInfo->hide();
		mDockSurface->hide();
		mDockView->hide();
		this->showFullScreen();
	}
}

//! Sets the statusbar of the main window.
//! Only plain text is shown, because HTML tags are stripped.
void QGMMainWindow::setStatusBarMessage( const QString& messageStr ) {
	//cout << "[QGMMainWindow::setStatusBarMessage] " << messageStr.toStdString() << endl;
	QString messagePlainStr = QTextDocumentFragment::fromHtml( messageStr ).toPlainText();
	statusBar()->showMessage( messagePlainStr );
}

//! set drag and drop of files into gigamesh
void QGMMainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}


//! drop event, accept the following 3D-Files (*.obj *.OBJ *.ply *.PLY *.wrl *.WRL *.txt *.TXT *.xyz *.XYZ)
void QGMMainWindow::dropEvent(QDropEvent *e)
{
    QString fileName = "";
    foreach (const QUrl &url, e->mimeData()->urls()) {
       if(url.toLocalFile().endsWith("obj", Qt::CaseInsensitive) ||
               url.toLocalFile().endsWith("ply", Qt::CaseInsensitive) ||
               url.toLocalFile().endsWith("wrl", Qt::CaseInsensitive)||
               url.toLocalFile().endsWith("txt", Qt::CaseInsensitive) ||
               url.toLocalFile().endsWith("xyz", Qt::CaseInsensitive))
       fileName = url.toLocalFile();
    }

    if(fileName != "")
        load(fileName);
}

//-----------------------------------------------------------------------------------------------------------------

//! Overwritten for the multi-language interface.
void QGMMainWindow::changeEvent(QEvent* event)
{
	if(event != nullptr)
	{
		switch(event->type()) {
			case QEvent::LanguageChange:
				retranslateUi(this);
				break;
			default: // Do nothing - required. Otherwise many warnings will occur at compile time.
				break;
		}
	}
	QMainWindow::changeEvent(event);
}
