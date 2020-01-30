cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(GUI_SOURCES main.cpp
				QGMMainWindow.cpp
				meshQt.cpp
				image2d.cpp
				meshwidget.cpp
				qgmdockinfo.cpp
				QGMDialogMSII.cpp
				QGMDialogAbout.cpp
				QGMDialogRuler.cpp
				qgmdocksidebar.cpp
				qgmdockview.cpp
				meshwidget_params.cpp
				QGMDialogComboBox.cpp
				QGMDialogSliderHD.cpp
				QGMDialogConeParam.cpp
				QGMDialogEnterText.cpp
				QGMDialogPlaneParam.cpp
				QGMDialogNprSettings.cpp
				MeshQtCSVImportExport.cpp
				QGMDialogCutOffValues.cpp
				dialogGridCenterSelect.cpp
				ExternalProgramsDialog.cpp
				qruntpsrpmscriptdialog.cpp
				QGMDialogTransparencySettings.cpp
				meshGL/meshGL.cpp
				meshGL/meshglcolors.cpp
				meshGL/meshGL_params.cpp
				meshGL/meshGLShader.cpp
				meshGL/PinRenderer.cpp
				meshGL/TexturedMesh.cpp
				meshGL/TexturedMeshRenderer.cpp
				svg/SvgWriter.cpp
				svg/SvgCircle.cpp
				svg/SvgImage.cpp
				svg/SvgPath.cpp
				svg/SvgRect.cpp
				svg/SvgText.cpp
				normalSphereSelection/ArcBall.cpp
				normalSphereSelection/NormalSphereSelectionDialog.cpp
				normalSphereSelection/NormalSphereSelectionRenderWidget.cpp
)

set(GUI_HEADERS normalSphereSelection/NormalSphereSelectionRenderWidget.h)

find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL Network REQUIRED)

#NOTE: instead of adding translations via resources, the ts-files can also be build by
#      qt5_create_translation and then be added
qt5_add_resources(GUI_RESOURCES "forms/gigamesh.qrc"
								"shaders/shaders.qrc"
								"languages/translations.qrc"
								"latex_report_template/latex_templates.qrc"
								"python-scripts/pythonscripts.qrc"
)

qt5_wrap_ui(GUI_UI_HEADERS   "forms/mainWin.ui"
						 "forms/qruntpsrpmscriptdialog.ui"
						 "forms/ExternalProgramsDialog.ui"
						 "forms/dialogGridCenterSelect.ui"
						 "forms/qgmdialognprsettings.ui"
						 "forms/QGMDialogTransparencySettings.ui"
						 "forms/QGMDialogSliderHD.ui"
						 "forms/qgmdocksidebar.ui"
						 "forms/qgmdockinfo.ui"
						 "forms/qgmdockview.ui"
						 "forms/dialogAbout.ui"
						 "forms/dialogConeParam.ui"
						 "forms/dialogMSII.ui"
						 "forms/dialogCutOffValues.ui"
						 "forms/dialogEnterText.ui"
						 "forms/dialogPlaneParam.ui"
						 "forms/dialogComboBox.ui"
						 "forms/dialogRuler.ui"
						 "normalSphereSelection/NormalSphereSelectionDialog.ui"
)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)

add_executable(gigamesh ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_RESOURCES} ${GUI_UI_HEADERS})


add_definitions(${Qt5Widgets_DEFINITIONS})
target_include_directories(gigamesh PRIVATE ${Qt5Widgets_INCLUDES})

add_definitions(${Qt5OpenGL_DEFINITIONS})
target_include_directories(gigamesh PRIVATE ${Qt5OpenGL_INCLUDES})

add_definitions(${Qt5Network_DEFINITIONS})
target_include_directories(gigamesh PRIVATE ${Qt5Network_INCLUDES})

#needed to find generated ui-header files
target_include_directories(gigamesh PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#needed, if ui-header files need references to custom classes (e.g. NormalSphereSelectionRenderWidget.h)
target_include_directories(gigamesh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gigamesh PRIVATE Qt5::Widgets Qt5::OpenGL Qt5::Gui Qt5::Network psalm spherical_intersection GL gigameshCore)
target_compile_options(gigamesh PRIVATE -DTHREADS -DNUM_THREADS=11 -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")
