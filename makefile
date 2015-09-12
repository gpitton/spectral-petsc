
CFLAGS	         =-std=c99
FFLAGS	         =
CPPFLAGS         =
FPPFLAGS         =
CLINKER		 = mpicxx
#INC              = -I/${PETSC_DIR}include
#LOCDIR           = src/ksp/ksp/examples/tutorials/
CLEANFILES       = cheb
NP               = 1

#include ${PETSC_DIR}/conf/variables
#include ${PETSC_DIR}/conf/rules
include ${PETSC_DIR}lib/petsc/conf/variables
include ${PETSC_DIR}lib/petsc/conf/rules

chebyshev: chebyshev.o  chkopts
	-${CLINKER} -o chebyshev chebyshev.o -lm -lfftw3 -lfftw3_mpi ${PETSC_LIB}
cheb:   chebyshev.o cheb.o  chkopts
	-${CLINKER} -o cheb cheb.o chebyshev.o -lm -lfftw3 -lfftw3_mpi ${PETSC_LIB}
	${RM} cheb.o chebyshev.o
elliptic: chebyshev.o elliptic.o  chkopts
	-${CLINKER} -o elliptic elliptic.o chebyshev.o -lm -lfftw3 -lfftw3_mpi ${PETSC_LIB}
elliptic2: chebyshev.o elliptic2.o  chkopts
	-${CLINKER} -o elliptic2 elliptic2.o chebyshev.o -lm -lfftw3 -lfftw3_mpi ${PETSC_LIB}
	${RM} elliptic.o chebyshev.o
tref3.6: tref3.6.o  chkopts
	-${CLINKER} -o tref3.6 tref3.6.o -lm -lfftw3 -lfftw3_mpi ${PETSC_LIB}
	${RM} tref3.6.o

#include ${PETSC_DIR}/conf/test
include ${PETSC_DIR}lib/petsc/conf/test
