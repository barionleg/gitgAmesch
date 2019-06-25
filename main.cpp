//! \mainpage GigaMesh
//!
//! \author Hubert Mara

//! \todo check back later for debuging using http://www.cplusplus.com/reference/std/typeinfo/type_info/ AND find a nice way to demangle the output of .name()

// generic C++ includes:
#include <iostream>

// generic Qt includes:
#include <QApplication>
#include <QGLFormat>
#include <QMessageBox>
#include <QTranslator>

// Qt Interface includes:
#include "QGMMacros.h"
#include "QGMMainWindow.h"

// Qt data structure:
// none.

// C++ includes:
#include "printbuildinfo.h"

#ifdef LIBTEST
	#include "libtest.h"
#endif

using namespace std;

int main( int argc, char *argv[] ) {
	QApplication app( argc, argv );

	/*
	//Initialize translation:
	QTranslator translator;

	if(translator.load("GigaMesh_" + QLocale::system().name(), ":/languages"))
		app.installTranslator(&translator);
	*/

	// Application settings:
	app.setOrganizationName( "GigaMesh" );
	app.setApplicationName( "GigaMesh" );
	QCoreApplication::setApplicationVersion( VERSION_PACKAGE );

	// Command line parsing
	QCommandLineParser parser;
	parser.setApplicationDescription( "GigaMesh Software Framework GUI Interface" );
	parser.addHelpOption();
	parser.addVersionOption();
	// from example: parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
	// from example: parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

	// A boolean option with a single name
	QCommandLineOption showLoadLastOption( "load-last", 
	                                       QCoreApplication::translate( "main", "Load the last file used." ) );
	parser.addOption( showLoadLastOption );

	QCommandLineOption showHiDPI20Option( "hidpi20", 
	                                       QCoreApplication::translate( "main", "Assume HiDPI Display scale by a factor of 2." ) );
	parser.addOption( showHiDPI20Option );

	// File name(s)
	parser.addPositionalArgument(  "filename", 
	                               QCoreApplication::translate( "main", "File with 3D-data.") );

	// Process the actual command line arguments given by the user
	parser.process( app );

	const QStringList args = parser.positionalArguments();

	QString targetFile;
	if( args.size() > 1 ) {
		std::cout << "[Main] WARINING: More than one filename specified. Additional files will be ignored!" << std::endl;
	}
	if( args.size() >= 1 ) {
		targetFile = args.at( 0 );
		std::cout << "[Main] Loading: " << targetFile.toStdString() << std::endl;
	}

	bool loadLast      = parser.isSet( showLoadLastOption );
	bool hidpi20       = parser.isSet( showHiDPI20Option );

	if( ( targetFile.size() > 0 ) && loadLast ) {
		cerr << "ERROR: Exclusive options: --file and --load-last were given." << endl;
		exit( EXIT_FAILURE );
	}

	// Progress with starting GigaMesh
	cout << "[GigaMesh] using Qt Version: " << qVersion() << endl; // see: http://doc.trolltech.com/4.6/qtglobal.html
	printBuildInfo();

#ifdef LIBTEST
	cout << "[GigaMesh] ------------------------------------------------------------------------" << endl;
	cout << "[GigaMesh] libtest has the answer: " << foobar() << endl;
#endif
	// System checks:
	if( !QGLFormat::hasOpenGL() ) {
		cerr << "[Main] ERROR: This system has no OpenGL support!" << endl;
		SHOW_MSGBOX_WARN( "No OpenGL", "ERROR: This system has no OpenGL support!" );
		exit( EXIT_FAILURE );
	}
	// Specify an OpenGL 3.2 format using the Core profile.
	// That is, no old-school fixed pipeline functionality
    QGLFormat glFormat; //QGLFormat is deprecated...
	//! \todo QSurfaceFormat is the replacment for the deprecated QGLFormat
	if(glFormat.openGLVersionFlags() & QGLFormat::OpenGL_Version_4_3)
		glFormat.setVersion( 4, 3 );        //Higher Version needed to handle transparency in shader. Revert to 3.3 if it breaks core functionalities...
	else
		glFormat.setVersion( 3, 3 );                   // The version has to be larger than 3.3 due to geometry shaders and other usefull stuff.

	glFormat.setProfile( QGLFormat::CoreProfile ); // Do not even think to change the Profile to CompatibilityProfile !!!
	glFormat.setSampleBuffers( true );

	// The main window:
	QGMMainWindow mainWindow;
	mainWindow.show();
	mainWindow.setupMeshWidget( glFormat );
	if( hidpi20 ) {
		QSize winSize = mainWindow.size();
		winSize *= 2.0;
		mainWindow.resize( winSize );
	}

	// Pass arguments to the main window
	if( targetFile.size() > 0 ) {
		mainWindow.load( targetFile );
	} else if( loadLast ) {
		if( !mainWindow.loadLast() ) {
			exit( EXIT_FAILURE );
		}
	} else {
		mainWindow.load();
	}

	return app.exec();
}
