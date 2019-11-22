#include "printbuildinfo.h"

#include <iostream>

//! Show build information.
void printBuildInfo() {
	std::cout << "[GigaMesh] ----------------------------------------------------------------------------------" << std::endl;
#ifdef VERSION_PACKAGE
	std::cout << "[GigaMesh] Package version: " << VERSION_PACKAGE << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Package version missing!" << std::endl;
#endif
	std::cout << "[GigaMesh] .................................................................................." << std::endl;
#ifdef COMP_USER
	std::cout << "[GigaMesh] Built by: " << COMP_USER << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Build user information missing!" << std::endl;
#endif
#ifdef COMP_DATE
	std::cout << "[GigaMesh] ..... on: " << COMP_DATE << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Build date information missing!" << std::endl;
#endif
#ifdef COMP_EDIT
	std::cout << "[GigaMesh] .... for: " << COMP_EDIT << std::endl;
#else
	std::cerr << "[GigaMesh] ERROR: Build edition information missing!" << std::endl;
#endif
#ifdef COMP_GITHEAD
	std::cout << "[GigaMesh] Git SHA1: " << COMP_GITHEAD << std::endl;
#else
	std::cerr << "[GigaMesh] Build  not based on git commit!" << std::endl;
#endif
	std::cout << "[GigaMesh] ----------------------------------------------------------------------------------" << std::endl;
}
