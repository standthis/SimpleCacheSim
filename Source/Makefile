# Minimal Makefile for a C program
# Author: Philip Machanick 2 February 2012

# set up the executable file name here
EXE = cachesim

INCLUDES = ../Headers

#put in all the compiled file names here (all .c files with .o replacing .c)
OBJS = cachesim.o get_args.o stringutils.o readfile.o IOutils.o multilevelAssoc.o \
       workload.o error.o simulateMultilevelAssoc.o stats.o readtrace.o \
       cachesetup.o rawcache.o
# list all the header files here (not the system headers)
HEADERS = ${INCLUDES}

#get_args.h stringutils.h readfile.h IOutils.h multilevelAssoc.h  \
#       workload.h error.h simulateMultilevelAssoc.h stats.h readtrace.h \
#       generaltypes.h rawcache.h cachesetup.h
# name of the C compiler
CC = gcc
# delete -g if you don't plan on using the debugger
CFLAGS = -g -I${INCLUDES}

# In most cases you won't need to change anything below here except
# the action for make test

# Makefile dependencies are generally expressed as
# thing_to_create : things_it_depends_on
# followed by a line with a rule to do the work
# Here, we use a general rule that applies for any
# C file and so only have to explicitly name the
# executables and compiled files (make recognizes
# .c as a C file ending).

# this line says for a C program to generate a .o file 
# do the next line (the next line must be indented with TAB;
# "C" means C and source files can end in .c);
# this automatically picks up CFLAGS (which we define) and
# CPPFLAGS (which we don't), and compiles with the command
# $(CC) $(CPPFLAGS) $(CFLAGS) -c:
.C.o:

# this line says if any of the files named on the OBJS line
# change, relink
$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS)

# this line says if Makefile or any headers change, rebuild everything
# where we have specific compilable files depending on specific
# headers you can write separate rules for each to reduce recompiles
$(EXE) $(OBJS): $(HEADERS) Makefile

# Putting the Makefile in a rule is rare for simple programs
# because doing so cause a rebuild every time you make a
# trivial change (you can always remove Makefile from the
# above rule when making cosmetic changes like to comments
# like this).

# These are fake dependencies on a file that doesn't exist so
# you can create a target for make that describes an action
# rather than an output file. A common example: make clean
# gets rid of compiler and linker outputs. You can also add
# in install as a target if you want to copy the executable
# to a standard location

# remove compiled outputs
clean:
	rm -f $(OBJS) $(EXE)

# remove all unnecessary files include backups created by an editor
realclean:
	rm -f $(OBJS) $(EXE) *~

# unit tests -- need work, used in early version and no longer current FIXME
#unittesterror: error.o error.h
#	$(CC) -o errortest -DUNITTESTERROR error.c; ./errortest; rm ./errortest

#unittestcachetypes: error.o cachetypes.o stringutils.o stats.o
#	$(CC) -g -o cachetypestest -DUNITTESTCACHETYPES cachetypes.c error.o stringutils.o stats.o; ./cachetypestest; rm ./cachetypestest

# FIXME: test not implemented
# run a test: in a real program you will want to include inputs
# and comparison with a known output (you could replace the second
# line by a script); trick here: shell script takes "-" as meaning
# read from stdin when you give a file name
#test: $(EXE)
#	./$(EXE) 1024 32 < Data/test-small.workload | Scripts/compare.sh - Data/test-small.out

