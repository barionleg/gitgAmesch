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

set(GUI_SOURCES src/main.cpp
				src/QGMMainWindow.cpp                 src/QGMMainWindow.h
				src/meshQt.cpp                        src/meshQt.h
				src/image2d.cpp                       src/image2d.h
				src/meshwidget.cpp                    src/meshwidget.h
				src/qgmdockinfo.cpp                   src/qgmdockinfo.h
				src/QGMDialogMSII.cpp                 src/QGMDialogMSII.h
				src/QGMDialogAbout.cpp                src/QGMDialogAbout.h
				src/QGMDialogRuler.cpp                src/QGMDialogRuler.h
				src/qgmdocksidebar.cpp                src/qgmdocksidebar.h
				src/qgmdockview.cpp                   src/qgmdockinfo.h
				src/meshwidget_params.cpp             src/meshwidget_params.h
				src/QGMDialogComboBox.cpp             src/QGMDialogComboBox.h
				src/QGMDialogSliderHD.cpp             src/QGMDialogSliderHD.h
				src/QGMDialogConeParam.cpp            src/QGMDialogConeParam.h
				src/QGMDialogEnterText.cpp            src/QGMDialogEnterText.h
				src/QGMDialogPlaneParam.cpp           src/QGMDialogPlaneParam.h
				src/QGMDialogNprSettings.cpp          src/QGMDialogNprSettings.h
				src/MeshQtCSVImportExport.cpp         src/MeshQtCSVImportExport.h
				src/QGMDialogCutOffValues.cpp         src/QGMDialogCutOffValues.h
				src/dialogGridCenterSelect.cpp        src/dialogGridCenterSelect.h
				src/ExternalProgramsDialog.cpp        src/ExternalProgramsDialog.h
				src/qruntpsrpmscriptdialog.cpp        src/qruntpsrpmscriptdialog.h
				src/QGMDialogTransparencySettings.cpp src/QGMDialogTransparencySettings.h
				src/meshGL/meshGL.cpp                 src/meshGL/meshGL.h
				src/meshGL/meshglcolors.cpp           src/meshGL/meshglcolors.h
				src/meshGL/meshGL_params.cpp          src/meshGL/meshGL_params.h
				src/meshGL/meshGLShader.cpp           src/meshGL/meshGLShader.h
				src/meshGL/PinRenderer.cpp            src/meshGL/PinRenderer.h
				src/meshGL/TexturedMesh.cpp           src/meshGL/TexturedMesh.h
				src/meshGL/TexturedMeshRenderer.cpp   src/meshGL/TexturedMeshRenderer.h
				src/svg/SvgWriter.cpp                 src/svg/SvgWriter.h
				src/svg/SvgCircle.cpp                 src/svg/SvgCircle.h
				src/svg/SvgImage.cpp                  src/svg/SvgImage.h
				src/svg/SvgPath.cpp                   src/svg/SvgPath.h
				src/svg/SvgRect.cpp                   src/svg/SvgRect.h
				src/svg/SvgText.cpp                   src/svg/SvgText.h
				src/normalSphereSelection/ArcBall.cpp src/normalSphereSelection/ArcBall.h
				src/normalSphereSelection/NormalSphereSelectionDialog.cpp
				src/normalSphereSelection/NormalSphereSelectionDialog.h
				src/normalSphereSelection/NormalSphereSelectionRenderWidget.cpp
				src/normalSphereSelection/NormalSphereSelectionRenderWidget.h
)

#add ui files, so they appear it qtcreator projects view
set(GUI_UI_FILES
	src/normalSphereSelection/NormalSphereSelectionDialog.ui
	forms/QGMDialogTransparencySettings.ui
	forms/dialogComboBox.ui
	forms/qgmdialognprsettings.ui
	forms/qgmdockview.ui
	forms/qruntpsrpmscriptdialog.ui
	forms/dialogMSII.ui
	forms/QGMDialogSliderHD.ui
	forms/dialogAbout.ui
	forms/dialogGridCenterSelect.ui
	forms/dialogRuler.ui
	forms/dialogEnterText.ui
	forms/qgmdockinfo.ui
	forms/dialogConeParam.ui
	forms/ExternalProgramsDialog.ui
	forms/mainWin.ui
	forms/dialogPlaneParam.ui
	forms/qgmdocksidebar.ui
	forms/dialogCutOffValues.ui
)

#NOTE: instead of adding translations via resources, the ts-files can also be build by
#      qt5_create_translation and then be added
qt5_add_resources(GUI_RESOURCES "forms/gigamesh.qrc"
								"src/shaders/shaders.qrc"
								"languages/translations.qrc"
								"latex_report_template/latex_templates.qrc"
								"python-scripts/pythonscripts.qrc"
)

#set here the paths of the ui-files, so autouic can find them
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms
							   ${CMAKE_CURRENT_SOURCE_DIR}/normalSphereSelection)

add_executable(gigamesh ${GUI_SOURCES} ${GUI_RESOURCES} ${GUI_UI_FILES})

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
target_include_directories(gigamesh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(gigamesh PRIVATE ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Network_LIBRARIES} psalm spherical_intersection OpenGL::GL gigameshCore)

#extras for static linking using the MXE mingw static build (set this via "export MINGW_CI_BUILD=True" on the command line)
if(MINGW_CI_BUILD)
	qt5_import_plugins(gigamesh INCLUDE Qt5::QSvgIconPlugin Qt5::QSvgPlugin)
	target_link_libraries(gigamesh PRIVATE ssl crypto crypt32 iphlpapi)
	target_sources(gigamesh PRIVATE GigaMesh_Logo.rc)
endif()

install(TARGETS gigamesh DESTINATION bin)
