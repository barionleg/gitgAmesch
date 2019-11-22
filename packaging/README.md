# Building the GigaMesh packages:

1. build_newsentry.sh: Should always be called first to set the version number within the changelog.

2. build_tarball.sh Should be executed on an older distribution e.g. Ubuntu 16.04, to provide binaries compatible with older systems.
   Vice versa you should not execute this script on a Manjaro system as the resulting binaries will run only with newest versions of the Qt libraries.

3. build_debian.sh: Same as for the tarball.

4. build_manjaro.sh: Executed on Manjaro. Best to be called last.

