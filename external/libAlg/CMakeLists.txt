cmake_minimum_required(VERSION 3.10)

project(libalg LANGUAGES CXX)

set(ALG_SOURCES src/ablas.cpp
	src/ablasf.cpp
	src/airyf.cpp
	src/ap.cpp
	src/apserv.cpp
	src/autogk.cpp
	src/bdss.cpp
	src/bdsvd.cpp
	src/bessel.cpp
	src/betaf.cpp
	src/binomialdistr.cpp
	src/blas.cpp
	src/chebyshev.cpp
	src/chisquaredistr.cpp
	src/conv.cpp
	src/corr.cpp
	src/correlation.cpp
	src/correlationtests.cpp
	src/creflections.cpp
	src/dawson.cpp
	src/densesolver.cpp
	src/descriptivestatistics.cpp
	src/dforest.cpp
	src/elliptic.cpp
	src/estnorm.cpp
	src/evd.cpp
	src/expintegrals.cpp
	src/fdistr.cpp
	src/fft.cpp
	src/fht.cpp
	src/fresnel.cpp
	src/ftbase.cpp
	src/gammafunc.cpp
	src/gkq.cpp
	src/gq.cpp
	src/hblas.cpp
	src/hermite.cpp
	src/hqrnd.cpp
	src/hsschur.cpp
	src/ialglib.cpp
	src/ibetaf.cpp
	src/idwint.cpp
	src/igammaf.cpp
	src/inverseupdate.cpp
	src/jacobianelliptic.cpp
	src/jarquebera.cpp
	src/kmeans.cpp
	src/laguerre.cpp
	src/lda.cpp
	src/ldlt.cpp
	src/legendre.cpp
	src/linmin.cpp
	src/linreg.cpp
	src/logit.cpp
	src/lsfit.cpp
	src/mannwhitneyu.cpp
	src/matdet.cpp
	src/matgen.cpp
	src/matinv.cpp
	src/minasa.cpp
	src/mincg.cpp
	src/minlbfgs.cpp
	src/minlm.cpp
	src/mlpbase.cpp
	src/mlpe.cpp
	src/mlptrain.cpp
	src/nearestneighbor.cpp
	src/nearunityunit.cpp
	src/normaldistr.cpp
	src/odesolver.cpp
	src/ortfac.cpp
	src/pca.cpp
	src/poissondistr.cpp
	src/polint.cpp
	src/psif.cpp
	src/pspline.cpp
	src/ratint.cpp
	src/ratinterpolation.cpp
	src/rcond.cpp
	src/reflections.cpp
	src/rotations.cpp
	src/safesolve.cpp
	src/sblas.cpp
	src/schur.cpp
	src/sdet.cpp
	src/sinverse.cpp
	src/spdgevd.cpp
	src/spline1d.cpp
	src/spline2d.cpp
	src/spline3.cpp
	src/srcond.cpp
	src/ssolve.cpp
	src/stest.cpp
	src/studenttdistr.cpp
	src/studentttests.cpp
	src/svd.cpp
	src/trfac.cpp
	src/trigintegrals.cpp
	src/trlinsolve.cpp
	src/tsort.cpp
	src/variancetests.cpp
	src/wsr.cpp
	src/xblas.cpp)


add_library(alg STATIC ${ALG_SOURCES} ${ALG_HEADERS})

target_include_directories(alg PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
							$<INSTALL_INTERFACE:include>
							PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(alg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/alg)
target_compile_features(alg PRIVATE cxx_std_17)
#set_target_properties(alg PROPERTIES PUBLIC_HEADER )

install(TARGETS alg
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		PUBLIC_HEADER DESTINATION include/alg)
