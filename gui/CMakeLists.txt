cmake_minimum_required(VERSION 3.10)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(GUI_SOURCES main.cpp
				QGMMainWindow.cpp                 QGMMainWindow.h
				meshQt.cpp                        meshQt.h
				image2d.cpp                       image2d.h
				meshwidget.cpp                    meshwidget.h
				qgmdockinfo.cpp                   qgmdockinfo.h
				QGMDialogMSII.cpp                 QGMDialogMSII.h
				QGMDialogAbout.cpp                QGMDialogAbout.h
				QGMDialogRuler.cpp                QGMDialogRuler.h
				qgmdocksidebar.cpp                qgmdocksidebar.h
				qgmdockview.cpp                   qgmdockinfo.h
				meshwidget_params.cpp             meshwidget_params.h
				QGMDialogComboBox.cpp             QGMDialogComboBox.h
				QGMDialogSliderHD.cpp             QGMDialogSliderHD.h
				QGMDialogConeParam.cpp            QGMDialogConeParam.h
				QGMDialogEnterText.cpp            QGMDialogEnterText.h
				QGMDialogPlaneParam.cpp           QGMDialogPlaneParam.h
				QGMDialogNprSettings.cpp          QGMDialogNprSettings.h
				MeshQtCSVImportExport.cpp         MeshQtCSVImportExport.h
				QGMDialogCutOffValues.cpp         QGMDialogCutOffValues.h
				dialogGridCenterSelect.cpp        dialogGridCenterSelect.h
				ExternalProgramsDialog.cpp        ExternalProgramsDialog.h
				qruntpsrpmscriptdialog.cpp        qruntpsrpmscriptdialog.h
				QGMDialogTransparencySettings.cpp QGMDialogTransparencySettings.h
				meshGL/meshGL.cpp                 meshGL/meshGL.h
				meshGL/meshglcolors.cpp           meshGL/meshglcolors.h
				meshGL/meshGL_params.cpp          meshGL/meshGL_params.h
				meshGL/meshGLShader.cpp           meshGL/meshGLShader.h
				meshGL/PinRenderer.cpp            meshGL/PinRenderer.h
				meshGL/TexturedMesh.cpp           meshGL/TexturedMesh.h
				meshGL/TexturedMeshRenderer.cpp   meshGL/TexturedMeshRenderer.h
				svg/SvgWriter.cpp                 svg/SvgWriter.h
				svg/SvgCircle.cpp                 svg/SvgCircle.h
				svg/SvgImage.cpp                  svg/SvgImage.h
				svg/SvgPath.cpp                   svg/SvgPath.h
				svg/SvgRect.cpp                   svg/SvgRect.h
				svg/SvgText.cpp                   svg/SvgText.h
				normalSphereSelection/ArcBall.cpp normalSphereSelection/ArcBall.h
				normalSphereSelection/NormalSphereSelectionDialog.cpp
				normalSphereSelection/NormalSphereSelectionDialog.h
				normalSphereSelection/NormalSphereSelectionRenderWidget.cpp
				normalSphereSelection/NormalSphereSelectionRenderWidget.h
)

#NOTE: instead of adding translations via resources, the ts-files can also be build by
#      qt5_create_translation and then be added
qt5_add_resources(GUI_RESOURCES "forms/gigamesh.qrc"
								"shaders/shaders.qrc"
								"languages/translations.qrc"
								"latex_report_template/latex_templates.qrc"
								"python-scripts/pythonscripts.qrc"
)

#set here the paths of the ui-files, so autouic can find them
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms
							   ${CMAKE_CURRENT_SOURCE_DIR}/normalSphereSelection)

add_executable(gigamesh ${GUI_SOURCES} ${GUI_RESOURCES})

add_definitions(${Qt5Widgets_DEFINITIONS})
target_include_directories(gigamesh PRIVATE ${Qt5Widgets_INCLUDES})

add_definitions(${Qt5OpenGL_DEFINITIONS})
target_include_directories(gigamesh PRIVATE ${Qt5OpenGL_INCLUDES})

add_definitions(${Qt5Network_DEFINITIONS})
target_include_directories(gigamesh PRIVATE ${Qt5Network_INCLUDES})

add_dependencies(gigamesh OpenGL::GL)

#needed to find generated ui-header files
target_include_directories(gigamesh PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#needed, if ui-header files need references to custom classes (e.g. NormalSphereSelectionRenderWidget.h)
target_include_directories(gigamesh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gigamesh PRIVATE ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Network_LIBRARIES} psalm spherical_intersection OpenGL::GL gigameshCore)

#extras for static linking using the MXE mingw static build (set this via "export MINGW_CI_BUILD=True" on the command line)
if(MINGW_CI_BUILD)
	qt5_import_plugins(gigamesh INCLUDE Qt5::QSvgIconPlugin Qt5::QSvgPlugin)
	target_link_libraries(gigamesh PRIVATE ssl crypto crypt32 iphlpapi)
endif()

target_compile_options(gigamesh PRIVATE -DTHREADS -DNUM_THREADS=11 -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")
