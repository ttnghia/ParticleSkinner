#!smake

#-----------------------------------------
#Basic Stuff -----------------------------
CC          = g++
cc          = gcc

#-----------------------------------------
#Misc Flags ------------------------------
MISC_FLAGS = -Wall

#-----------------------------------------
#Optimization ----------------------------
OPT   = -O3 -flto -std=c++11  #-fast

#-----------------------------------------
# Debug mode -----------------------------
DEBUG = -g -pg

LIB_MOD =

TARGETS = particleskinner

#-----------------------------------------
#-----------------------------------------

OBJECTS = main.o smoothingGrid.o marchingTet.o kdTree.o slMatrix.o slVector.o slUtil.o

#-----------------------------------------
#INCLUDE = -I../Libs/tbb_osx/include
#LIBS = -ltbb -L../Libs/tbb_osx/lib


INCLUDE = -I../Libs/tbb_linux/include
LIBS = -ltbb -L../Libs/tbb_linux/lib/intel64/gcc4.7 -lpthread
#INCS =
CCOPTS = $(OPT) $(MISC_FLAGS) $(DEBUG) $(INCLUDE)
LDOPTS = $(OPT) $(MISC_FLAGS) $(DEBUG) $(LIBS)

#-----------------------------------------
#-----------------------------------------

default: $(TARGETS)



clean:
	/bin/rm -rf *.o particleskinner

#-----------------------------------------

$(TARGETS): $(OBJECTS)
	$(CC) $(LDOPTS) $(OBJECTS) $(LIBS) -flto -o particleskinner

#-----------------------------------------
.C.o:
	$(CC) $(CCOPTS) -c $<

.cpp.o:
	$(CC) $(CCOPTS) -c $<

.c.o:
	$(cc) $(CCOPTS) -c $<

.o: $(OBJECTS)
	$(CC)  $(LDOPTS) $(OBJS) $(OBJECTS) $< $(LIBS) -o $@

.C: $(OBJECTS)
	$(CC) $(LDOPTS)  $(OBJS) $(OBJECTS) $< $(LIBS) $(FOR_LIB) -o $@

#-----------------------------------------
#-----------------------------------------














