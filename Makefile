# Minimal Makefile for a C program based on header
# and source directories
# Author: Philip Machanick 2 February 2012

# set up the executable file name here
EXE = cachesim
INSTALLDIR = .

${EXE}: Source Headers
	cd Source; make ${EXE}; cp ${EXE} ..

clean:
	cd Source; make clean

install: ${EXE}
	cp Source/${EXE} ${INSTALLDIR}
